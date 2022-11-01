/*
 * Copyright (c) 2022- Gerber Lóránt Viktor
 * Author: Gerber Lóránt Viktor <glorantv@student.elte.hu>
 *
 * This file is part of s4pkg.
 *
 * s4pkg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <s4pkg/internal/imagecoder.h>

#include <optional>
#include <unordered_map>

#include <stb_image.h>
#include <stb_image_write.h>

#include <jpeglib.h>

#include <fmt/printf.h>

namespace s4pkg::internal::imagecoder {

// Error handling for libjpeg

// If we encounter a fatal error, set the boolean in the struct to true, so we
// can return nullptr from our methods
void libjpegErrorExit(jpeg_common_struct* ptr) {
    if (ptr->client_data != nullptr) {
        *((bool*)ptr->client_data) = true;
    }
}

// We don't want error messages
void libjpegEmitMessage(jpeg_common_struct* ptr, int32_t level) {}

// Decoders

std::shared_ptr<Image> decodeJfifWithAlpha(const std::vector<uint8_t>& data) {
    // "JFIF with Alpha" is the name I've given to the format being used to
    // store thumbnails in packages. The file is an almost standard JFIF file,
    // able to be read by regular programs. It contains the RBB data compressed
    // as a jpeg, and an alpha mask embedded in a section named "ALFA" (with no
    // null terminator, hence "almost" standard), that is a greyscale PNG image
    // encoding the alpha value at each pixel

    // Set up the decompression of the whole JPEG file
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // This will be set whenever a fatal error is encountered
    bool hasError = false;
    cinfo.client_data = &hasError;

    cinfo.err = jpeg_std_error(&jerr);

    jerr.error_exit = &libjpegErrorExit;
    jerr.emit_message = &libjpegEmitMessage;

    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data.data(), data.size());

    if (hasError) {
        jpeg_destroy_decompress(&cinfo);
        return nullptr;
    }

    // We save all APP0 markers, since one of them starting with ALFA contains
    // the alpha values in the form of a greyscale PNG image
    jpeg_save_markers(&cinfo, JPEG_APP0, 0xFFFF);
    jpeg_read_header(&cinfo, true);

    if (hasError) {
        jpeg_destroy_decompress(&cinfo);
        return nullptr;
    }

    // If we find the alpha, save it here
    std::optional<std::vector<uint8_t>> alphaData;

    // Look through all the read markers to find the alpha image
    jpeg_saved_marker_ptr currentMarker = cinfo.marker_list;
    while (currentMarker != nullptr) {
        if (currentMarker->data[0] == 'A' && currentMarker->data[1] == 'L' &&
            currentMarker->data[2] == 'F' && currentMarker->data[3] == 'A') {
            // The size of the embedded PNG image. This should be the same as
            // currentMarker->data_length - 8 (identifier + 32-bit integer for
            // length)
            uint32_t alphaImageLength =
                ((currentMarker->data[4] << 24) & 0xFF) |
                ((currentMarker->data[5] << 16) & 0xFF) |
                ((currentMarker->data[6] << 8) & 0xFF) |
                (currentMarker->data[7] & 0xFF);

            if (alphaImageLength !=
                currentMarker->data_length - 8) {  // Some sanity-checking
                currentMarker = currentMarker->next;
                continue;
            }

            alphaData = std::vector<uint8_t>(alphaImageLength);

            for (int i = 0; i < alphaImageLength; i++) {
                alphaData.value()[i] =
                    currentMarker->data[8 + i];  // The image is stored after
                                                 // the identifier (4 bytes) and
                                                 // the image length (4 bytes)
            }
        }

        currentMarker = currentMarker->next;
    }

    // No alpha, not a valid file
    if (!alphaData) {
        jpeg_destroy_decompress(&cinfo);
        return nullptr;
    }

    // Now we decompress the JPEG image (final R, G, B channels)
    if (!jpeg_start_decompress(&cinfo)) {
        jpeg_destroy_decompress(&cinfo);
        return nullptr;
    }

    int width = cinfo.output_width;
    int height = cinfo.output_height;

    uint32_t rowStride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY scanlineBuffer = (cinfo.mem->alloc_sarray)(
        (j_common_ptr)&cinfo, JPOOL_IMAGE, rowStride, 1);

    std::vector<uint8_t> decodedFull(rowStride * cinfo.output_height);

    while (cinfo.output_scanline < cinfo.output_height) {
        if (jpeg_read_scanlines(&cinfo, scanlineBuffer, 1) > 0) {
            for (int i = 0; i < rowStride; i++) {
                decodedFull[(cinfo.output_scanline - 1) * rowStride + i] =
                    scanlineBuffer[0][i];
            }
        }

        if (hasError) {
            jpeg_destroy_decompress(&cinfo);
            return nullptr;
        }
    }

    // We're done with the JPEG
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // Decode the alpha
    int alphaWidth = 0;
    int alphaHeight = 0;
    int alphaChannelsInFile = 0;
    unsigned char* alphaImage = stbi_load_from_memory(
        alphaData->data(), alphaData->size(), &alphaWidth, &alphaHeight,
        &alphaChannelsInFile,
        1);  // We only care about one component in this image, because R = G =
             // B, and this contains the alpha value (white fully opaque, black
             // fully transparent)

    if (alphaImage == nullptr) {
        return nullptr;
    }

    // Verify that the two images have the same dimensions
    if (width != alphaWidth || height != alphaHeight) {
        return nullptr;
    }

    std::vector<uint8_t> finalImageData(width * height * 4);

    // Combine the two images into a single RGBA image
    for (int i = 0; i < width * height; i++) {
        uint8_t r = decodedFull[i * 3];
        uint8_t g = decodedFull[i * 3 + 1];
        uint8_t b = decodedFull[i * 3 + 2];

        uint8_t a = alphaImage[i];

        finalImageData[i * 4] = r;
        finalImageData[i * 4 + 1] = g;
        finalImageData[i * 4 + 2] = b;
        finalImageData[i * 4 + 3] = a;
    }

    stbi_image_free(alphaImage);

    return std::make_shared<Image>(width, height, finalImageData);
}

// Encoders

std::vector<uint8_t> encodeJfifWithAlpha(const Image& image) {
    return {};
}

// Implementation of the s4pkg::internal::imagecoder::(decode / encode)
// functions, which just delegate to the actual implementations defined
// above

typedef std::shared_ptr<Image> (*t_decoderFunction)(
    const std::vector<uint8_t>&);

typedef std::vector<uint8_t> (*t_encoderFunction)(const Image&);

const std::unordered_map<ImageFormat, t_decoderFunction> g_decoderMapping = {
    {JFIF_WITH_ALPHA, &decodeJfifWithAlpha}};

const std::unordered_map<ImageFormat, t_encoderFunction> g_encoderMapping = {
    {JFIF_WITH_ALPHA, &encodeJfifWithAlpha}};

template <typename T>
T tryGetFunction(const std::unordered_map<ImageFormat, T>& mapping,
                 ImageFormat key) {
    if (mapping.find(key) != mapping.end()) {
        return mapping.at(key);
    } else {
        return nullptr;
    }
}

std::shared_ptr<Image> decode(const std::vector<uint8_t>& data,
                              ImageFormat format) {
    t_decoderFunction chosenDecoder = tryGetFunction(g_decoderMapping, format);
    if (chosenDecoder != nullptr) {
        return chosenDecoder(data);
    }

    return nullptr;
}

std::vector<uint8_t> encode(const Image& image, ImageFormat format) {
    t_encoderFunction chosenEncoder = tryGetFunction(g_encoderMapping, format);
    if (chosenEncoder != nullptr) {
        return chosenEncoder(image);
    }

    return {};
}
};  // namespace s4pkg::internal::imagecoder
