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

#include <s4pkg/internal/dds.h>
#include <s4pkg/internal/imagecoder.h>
#include <s4pkg/internal/membuf.h>
#include <s4pkg/internal/rle.h>
#include <s4pkg/internal/streams.h>

#include <cmath>
#include <optional>
#include <tuple>
#include <unordered_map>

#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

#include <jpeglib.h>

#include <fmt/printf.h>

#include <squish.h>

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

// For some reason the stbi_write_*_to_mem functions aren't available from
// stb_image_write.h
STBIWDEF unsigned char* stbi_write_png_to_mem(const unsigned char* pixels,
                                              int stride_bytes,
                                              int x,
                                              int y,
                                              int n,
                                              int* out_len);

// Decoders

std::shared_ptr<Image> decodeJfifWithAlpha(const lib::ByteBuffer& data) {
    // "JFIF with Alpha" is the name I've given to the format being used to
    // store thumbnails in packages. The is a standard JFIF file,
    // able to be read by regular programs. It contains the RBB data compressed
    // as a jpeg, and an alpha mask embedded in an APP0 section named "ALFA"
    // (with no null terminator) that is a greyscale PNG image encoding the
    // alpha value at each pixel

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
    jpeg_mem_src(&cinfo, data.data(), (int)data.size());

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
    std::optional<lib::ByteBuffer> alphaData;

    // Look through all the read markers to find the alpha image
    jpeg_saved_marker_ptr currentMarker = cinfo.marker_list;
    while (currentMarker != nullptr) {
        if (currentMarker->data[0] == 'A' && currentMarker->data[1] == 'L' &&
            currentMarker->data[2] == 'F' && currentMarker->data[3] == 'A') {
            // The size of the embedded PNG image. This should be the same as
            // currentMarker->data_length - 8 (identifier + 32-bit integer for
            // length)
            uint32_t alphaImageLength = (currentMarker->data[4] << 24) |
                                        (currentMarker->data[5] << 16) |
                                        (currentMarker->data[6] << 8) |
                                        (currentMarker->data[7] & 0xFF);

            if (alphaImageLength !=
                currentMarker->data_length - 8) {  // Some sanity-checking
                currentMarker = currentMarker->next;
                continue;
            }

            alphaData = lib::ByteBuffer(alphaImageLength);

            for (uint32_t i = 0; i < alphaImageLength; i++) {
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

    lib::ByteBuffer rgbImage(rowStride * cinfo.output_height);

    while (cinfo.output_scanline < cinfo.output_height) {
        if (jpeg_read_scanlines(&cinfo, scanlineBuffer, 1) > 0) {
            for (uint32_t i = 0; i < rowStride; i++) {
                rgbImage[(cinfo.output_scanline - 1) * rowStride + i] =
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

    // Decode the alpha (final A channel)
    int alphaWidth = 0;
    int alphaHeight = 0;
    int alphaChannelsInFile = 0;
    unsigned char* alphaImage = stbi_load_from_memory(
        alphaData->data(), (int)alphaData->size(), &alphaWidth, &alphaHeight,
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

    lib::ByteBuffer finalImage(width * height * 4);

    // Combine the two images into a single RGBA image
    for (int i = 0; i < width * height; i++) {
        uint8_t r = rgbImage[i * 3];
        uint8_t g = rgbImage[i * 3 + 1];
        uint8_t b = rgbImage[i * 3 + 2];

        uint8_t a = alphaImage[i];

        finalImage[i * 4] = r;
        finalImage[i * 4 + 1] = g;
        finalImage[i * 4 + 2] = b;
        finalImage[i * 4 + 3] = a;
    }

    stbi_image_free(alphaImage);

    return std::make_shared<Image>(width, height, finalImage);
}

// This is split into a separate method because more than one resource uses DXT5
// compressed textures; they are just stored differently in the package. We
// first convert them back to DXT5, then delegate to this method for the actual
// decoding.
std::shared_ptr<Image> decodeDxt5Internal(const dds::dds_file_t& ddsFile) {
    // Verify that it is indeed compressed as DXT5
    if (ddsFile.m_header.m_pixelFormat.m_fourCC !=
        MAKE_FOURCC('D', 'X', 'T', '5')) {
        return nullptr;
    }

    lib::ByteBuffer decompressedData(ddsFile.m_header.m_width *
                                     ddsFile.m_header.m_height * 4);

    squish::DecompressImage(decompressedData.data(), ddsFile.m_header.m_width,
                            ddsFile.m_header.m_height,
                            ddsFile.m_mainImage.data(), squish::kDxt5);

    return std::make_shared<Image>(ddsFile.m_header.m_width,
                                   ddsFile.m_header.m_height, decompressedData);
}

std::shared_ptr<Image> decodeDxt1Internal(const dds::dds_file_t& ddsFile) {
    // Verify that it is indeed compressed as DXT1
    if (ddsFile.m_header.m_pixelFormat.m_fourCC !=
        MAKE_FOURCC('D', 'X', 'T', '1')) {
        return nullptr;
    }

    lib::ByteBuffer decompressedData(ddsFile.m_header.m_width *
                                     ddsFile.m_header.m_height * 4);

    squish::DecompressImage(decompressedData.data(), ddsFile.m_header.m_width,
                            ddsFile.m_header.m_height,
                            ddsFile.m_mainImage.data(), squish::kDxt1);

    return std::make_shared<Image>(ddsFile.m_header.m_width,
                                   ddsFile.m_header.m_height, decompressedData);
}

/**
 * @brief Method to reconstruct the image data in a dds_file_t struct from raw
 * block data. For more info on how this works, see the implementation of the
 * DDS parser.
 * @param ddsFile: the file to modify
 * @param imageData: the raw block data
 */
void reconstructDdsImageData(dds::dds_file_t& ddsFile,
                             const lib::ByteBuffer& imageData) {
    std::vector<lib::ByteBuffer> reconstructedMipmaps(
        std::max<int32_t>(0, ddsFile.m_header.m_mipMapCount - 1));

    uint32_t mipmapCopyIdx = 0;
    uint32_t width = ddsFile.m_header.m_width;
    uint32_t height = ddsFile.m_header.m_height;

    uint8_t blockSize = 16;
    if (ddsFile.m_header.m_pixelFormat.m_fourCC ==
            MAKE_FOURCC('D', 'X', 'T', '1') ||
        ddsFile.m_header.m_pixelFormat.m_fourCC ==
            MAKE_FOURCC('D', 'S', 'T', '1')) {
        blockSize = 8;
    }

    uint32_t mipmapPosition = DDS_IMAGE_SIZE(width, height, blockSize);

    lib::ByteBuffer reconstructedMainImage(mipmapPosition);
    COPY_BYTES(imageData, reconstructedMainImage, 0,
               reconstructedMainImage.size(), mipmapCopyIdx);
    mipmapCopyIdx = 0;

    for (int i = 0; i < reconstructedMipmaps.size(); i++) {
        width /= 2;
        height /= 2;

        width = std::max<uint32_t>(1, width);
        height = std::max<uint32_t>(1, height);

        lib::ByteBuffer mipmap(DDS_IMAGE_SIZE(width, height, blockSize));
        COPY_BYTES(imageData, mipmap, mipmapPosition, mipmap.size(),
                   mipmapCopyIdx);

        mipmapCopyIdx = 0;
        mipmapPosition += (uint32_t)mipmap.size();

        reconstructedMipmaps[i] = mipmap;
    }

    ddsFile.m_mainImage = reconstructedMainImage;
    ddsFile.m_mipmaps = reconstructedMipmaps;
}

/**
 * @brief Method to concatenate all block data into a single continous vector
 * (like in the original file). For more info on how this works, see the
 * implementation of the DDS parser.
 * @param ddsFile: the file whose image data should be processed
 * @return the raw block data
 */
lib::ByteBuffer concatDdsImageData(const dds::dds_file_t& ddsFile) {
    uint32_t dataSize = (uint32_t)ddsFile.m_mainImage.size();
    for (const auto& mipmap : ddsFile.m_mipmaps) {
        dataSize += (uint32_t)mipmap.size();
    }

    uint32_t copyIdx = 0;
    lib::ByteBuffer imageData(dataSize);
    COPY_BYTES(ddsFile.m_mainImage, imageData, 0, ddsFile.m_mainImage.size(),
               copyIdx);

    for (const auto& mipmap : ddsFile.m_mipmaps) {
        COPY_BYTES(mipmap, imageData, 0, mipmap.size(), copyIdx);
    }

    return imageData;
}

std::shared_ptr<Image> decodeDst5(const lib::ByteBuffer& data) {
    // Read in the DDS file from memory
    membuf memoryBuffer(data.data(), data.size());
    std::istream stream(&memoryBuffer);

    dds::dds_file_t ddsFile{};

    // Verify that we could read the file correctly
    if (!dds::readFile(stream, ddsFile)) {
        return nullptr;
    }

    // Verify that this is in fact a DST5 compressed file
    if (ddsFile.m_header.m_pixelFormat.m_fourCC !=
        MAKE_FOURCC('D', 'S', 'T', '5')) {
        return nullptr;
    }

    // The raw blocks data as it would appear in the file
    lib::ByteBuffer imageData = concatDdsImageData(ddsFile);

    // Vector to hold the unshuffled block data
    lib::ByteBuffer outputImage(imageData.size());

    // Block offsets for unshuffling
    uint32_t blockOffset0 = 0;
    uint32_t blockOffset2 = blockOffset0 + ((uint32_t)imageData.size() >> 3);
    uint32_t blockOffset1 = blockOffset2 + ((uint32_t)imageData.size() >> 2);
    uint32_t blockOffset3 =
        blockOffset1 + (6 * (uint32_t)imageData.size() >> 4);

    // Count of all blocks in the image
    uint32_t blockCount = (blockOffset2 - blockOffset0) / 2;

    // Unshuffle the blocks
    uint32_t writeIdx = 0;
    for (uint32_t i = 0; i < blockCount; i++) {
        COPY_BYTES(imageData, outputImage, blockOffset0, 2, writeIdx);
        COPY_BYTES(imageData, outputImage, blockOffset1, 6, writeIdx);
        COPY_BYTES(imageData, outputImage, blockOffset2, 4, writeIdx);
        COPY_BYTES(imageData, outputImage, blockOffset3, 4, writeIdx);

        blockOffset0 += 2;
        blockOffset1 += 6;
        blockOffset2 += 4;
        blockOffset3 += 4;
    }

    // Reconstruct the image data in the dds_file_t structure
    reconstructDdsImageData(ddsFile, outputImage);

    // Set the file header to DXT5, the file is now decoded and can be read
    ddsFile.m_header.m_pixelFormat.m_fourCC = MAKE_FOURCC('D', 'X', 'T', '5');

    return decodeDxt5Internal(ddsFile);
}

std::shared_ptr<Image> decodeDxt5(const lib::ByteBuffer& data) {
    // Read in the DDS file from memory
    membuf memoryBuffer(data.data(), data.size());
    std::istream stream(&memoryBuffer);

    dds::dds_file_t ddsFile{};

    // Verify that we could read the file correctly
    if (!dds::readFile(stream, ddsFile)) {
        return nullptr;
    }

    // Verify that this is in fact a DXT5 compressed file
    if (ddsFile.m_header.m_pixelFormat.m_fourCC !=
        MAKE_FOURCC('D', 'X', 'T', '5')) {
        return nullptr;
    }

    return decodeDxt5Internal(ddsFile);
}

std::shared_ptr<Image> decodeDst1(const lib::ByteBuffer& data) {
    // Read in the DDS file from memory
    membuf memoryBuffer(data.data(), data.size());
    std::istream stream(&memoryBuffer);

    dds::dds_file_t ddsFile{};

    // Verify that we could read the file correctly
    if (!dds::readFile(stream, ddsFile)) {
        return nullptr;
    }

    // Verify that this is in fact a DST1 compressed file
    if (ddsFile.m_header.m_pixelFormat.m_fourCC !=
        MAKE_FOURCC('D', 'S', 'T', '1')) {
        return nullptr;
    }

    // The raw blocks data as it would appear in the file
    lib::ByteBuffer imageData = concatDdsImageData(ddsFile);

    // Vector to hold the unshuffled block data
    lib::ByteBuffer outputImage(imageData.size());

    // Block offsets for unshuffling
    uint32_t blockOffset2 = 0;
    uint32_t blockOffset3 = blockOffset2 + ((uint32_t)imageData.size() >> 1);

    // Count of all blocks in the image
    uint32_t blockCount = (blockOffset3 - blockOffset2) / 4;

    // Unshuffle the blocks
    uint32_t writeIdx = 0;
    for (uint32_t i = 0; i < blockCount; i++) {
        COPY_BYTES(imageData, outputImage, blockOffset2, 4, writeIdx);
        COPY_BYTES(imageData, outputImage, blockOffset3, 4, writeIdx);

        blockOffset2 += 4;
        blockOffset3 += 4;
    }

    // Reconstruct the image data in the dds_file_t structure
    reconstructDdsImageData(ddsFile, outputImage);

    // Set the file header to DXT1, the file is now decoded and can be read
    ddsFile.m_header.m_pixelFormat.m_fourCC = MAKE_FOURCC('D', 'X', 'T', '1');

    return decodeDxt1Internal(ddsFile);
}

std::shared_ptr<Image> decodeDxt1(const lib::ByteBuffer& data) {
    // Read in the DDS file from memory
    membuf memoryBuffer(data.data(), data.size());
    std::istream stream(&memoryBuffer);

    dds::dds_file_t ddsFile{};

    // Verify that we could read the file correctly
    if (!dds::readFile(stream, ddsFile)) {
        return nullptr;
    }

    // Verify that this is in fact a DXT1 compressed file
    if (ddsFile.m_header.m_pixelFormat.m_fourCC !=
        MAKE_FOURCC('D', 'X', 'T', '1')) {
        return nullptr;
    }

    return decodeDxt1Internal(ddsFile);
}

std::shared_ptr<Image> decodeDxt3(const lib::ByteBuffer& data) {
    // Read in the DDS file from memory
    membuf memoryBuffer(data.data(), data.size());
    std::istream stream(&memoryBuffer);

    dds::dds_file_t ddsFile{};

    // Verify that we could read the file correctly
    if (!dds::readFile(stream, ddsFile)) {
        return nullptr;
    }

    // Verify that it is indeed compressed as DXT3
    if (ddsFile.m_header.m_pixelFormat.m_fourCC !=
        MAKE_FOURCC('D', 'X', 'T', '3')) {
        return nullptr;
    }

    lib::ByteBuffer decompressedData(ddsFile.m_header.m_width *
                                     ddsFile.m_header.m_height * 4);

    squish::DecompressImage(decompressedData.data(), ddsFile.m_header.m_width,
                            ddsFile.m_header.m_height,
                            ddsFile.m_mainImage.data(), squish::kDxt3);

    return std::make_shared<Image>(ddsFile.m_header.m_width,
                                   ddsFile.m_header.m_height, decompressedData);
}

std::shared_ptr<Image> decodeUncompressedDds(const lib::ByteBuffer& data) {
    // Read in the DDS file from memory
    membuf memoryBuffer(data.data(), data.size());
    std::istream stream(&memoryBuffer);

    dds::dds_file_t ddsFile{};

    // Verify that we could read the file correctly
    if (!dds::readFile(stream, ddsFile)) {
        return nullptr;
    }

    // Verify that it is indeed uncompressed data
    if ((ddsFile.m_header.m_pixelFormat.m_flags &
         (dds::DDPF_RGB | dds::DDPF_LUMINANCE)) == 0) {
        return nullptr;
    }

    // Utility to extract a byte from val using the provided mask
    auto extractMasked = [](uint64_t val, uint64_t mask) {
        uint64_t masked = val & mask;

        uint8_t maskShiftedBytes = 0;
        while ((mask & 0xFF) == 0) {
            maskShiftedBytes++;
            mask = mask >> 8;
        }

        return (uint8_t)((masked >> (maskShiftedBytes * 8)) & 0xFF);
    };

    // We want to be as correct as possible, so we reorganise the pixel data
    // to respect the RGBA flags set in the pixel format of the file (we
    // don't know if the channel order is RGBA, ARGB, ABGR or BGRA, etc.,
    // but we should be able to handle all these cases)
    lib::ByteBuffer reorganisedImageData(ddsFile.m_header.m_width *
                                         ddsFile.m_header.m_height * 4);

    // Get how many bytes are there per pixel
    uint8_t bytesPerPixel = ddsFile.m_header.m_pixelFormat.m_rgbBitCount / 8;
    for (int i = 0; i < ddsFile.m_mainImage.size(); i += bytesPerPixel) {
        // Set up an integer to store the whole pixel, as large as we can
        // support
        uint64_t pixelData = 0;

        // Combine all the bytes of this pixel into our single large integer
        for (int j = 0; j < bytesPerPixel; j++) {
            pixelData |= (uint64_t)ddsFile.m_mainImage[i + j] << (j * 8);
        }

        // Extract the components of this pixel according to the flags set in
        // the file
        reorganisedImageData[i] =
            extractMasked(pixelData, ddsFile.m_header.m_pixelFormat.m_rBitMask);

        reorganisedImageData[i + 1] =
            extractMasked(pixelData, ddsFile.m_header.m_pixelFormat.m_gBitMask);

        reorganisedImageData[i + 2] =
            extractMasked(pixelData, ddsFile.m_header.m_pixelFormat.m_bBitMask);

        uint8_t alphaValue = 0xFF;

        // If the file has stored alpha, use that insteaed of fully opaque
        if ((ddsFile.m_header.m_pixelFormat.m_flags & dds::DDPF_ALPHAPIXELS) !=
            0) {
            alphaValue = extractMasked(
                pixelData, ddsFile.m_header.m_pixelFormat.m_aBitMask);
        }

        reorganisedImageData[i + 3] = alphaValue;
    }

    return std::make_shared<Image>(ddsFile.m_header.m_width,
                                   ddsFile.m_header.m_height,
                                   reorganisedImageData);
}

// Handles decoding for RLE2 and RLES
std::shared_ptr<Image> decodeRle(const lib::ByteBuffer& data) {
    membuf memoryBuffer(data.data(), data.size());
    std::istream stream(&memoryBuffer);

    rle::rle_file_t rleFile = rle::readFile(stream);

    if (rleFile.m_rleHeader.m_fourCC == rle::fourcc_t::DXT5) {
        return decodeDxt5Internal(rleFile.m_ddsFile);
    }

    // TODO: Handle L8 when ready

    return nullptr;
}

// Encoders

lib::ByteBuffer encodeJfifWithAlpha(const Image& image) {
    // Set up vectors for the RGB and A channels of the image
    lib::ByteBuffer rgbData(image.getWidth() * image.getHeight() * 3);
    lib::ByteBuffer alphaData(image.getWidth() * image.getHeight());

    // Separate the original image into an RGB array that will be JPEG
    // compressed, and an alpha array that will be saved as a greyscale PNG
    for (uint32_t i = 0; i < image.getWidth() * image.getHeight(); i++) {
        rgbData[i * 3] = image.getPixelData()[i * 4];
        rgbData[i * 3 + 1] = image.getPixelData()[i * 4 + 1];
        rgbData[i * 3 + 2] = image.getPixelData()[i * 4 + 2];

        alphaData[i] = image.getPixelData()[i * 4 + 3];
    }

    // Write the alpha to a PNG
    int32_t alphaImageSize = 0;
    unsigned char* alphaImage = stbi_write_png_to_mem(
        (const unsigned char*)alphaData.data(), image.getWidth(),
        image.getWidth(), image.getHeight(), 1, &alphaImageSize);

    if (alphaImage == nullptr) {
        return {};
    }

    // Set up the JPEG encoder
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // This will be set whenever a fatal error is encountered
    bool hasError = false;
    cinfo.client_data = &hasError;

    cinfo.err = jpeg_std_error(&jerr);

    jerr.error_exit = &libjpegErrorExit;
    jerr.emit_message = &libjpegEmitMessage;

    jpeg_create_compress(&cinfo);

    // Let libjpeg automatically manage the buffer
    unsigned char* jpegBuffer = nullptr;
    unsigned long jpegBufferSize = 0;
    jpeg_mem_dest(&cinfo, &jpegBuffer, &jpegBufferSize);

    // Feed it some initial information about the image
    cinfo.image_height = image.getHeight();
    cinfo.image_width = image.getWidth();
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, true);

    if (hasError) {
        jpeg_destroy_compress(&cinfo);

        free(alphaImage);
        free(jpegBuffer);

        return {};
    }

    jpeg_start_compress(&cinfo, true);

    // Set up the ALFA APP0 data
    lib::ByteBuffer alphaApp0Segment(4 + 4 + alphaImageSize);
    alphaApp0Segment[0] = 'A';
    alphaApp0Segment[1] = 'L';
    alphaApp0Segment[2] = 'F';
    alphaApp0Segment[3] = 'A';

    alphaApp0Segment[4] = (alphaImageSize >> 24 & 0xFF);
    alphaApp0Segment[5] = (alphaImageSize >> 16 & 0xFF);
    alphaApp0Segment[6] = (alphaImageSize >> 8 & 0xFF);
    alphaApp0Segment[7] = (alphaImageSize & 0xFF);

    // Copy the PNG image from before into the APP0 data
    for (int i = 0; i < alphaImageSize; i++) {
        alphaApp0Segment[8 + i] = alphaImage[i];
    }

    free(alphaImage);

    // Write the alpha segment into the JPEG header
    jpeg_write_marker(&cinfo, JPEG_APP0, alphaApp0Segment.data(),
                      (unsigned int)alphaApp0Segment.size());

    uint32_t rowStride = image.getWidth() * 3;

    JSAMPROW rowPointer[1];

    // Write the RGB values line-by-line encoded as JPEG
    while (cinfo.next_scanline < cinfo.image_height) {
        rowPointer[0] = rgbData.data() + (cinfo.next_scanline * rowStride);

        if (jpeg_write_scanlines(&cinfo, rowPointer, 1) != 1) {
            jpeg_destroy_compress(&cinfo);
            free(jpegBuffer);

            return {};
        }
    }

    // We're done with the file
    jpeg_finish_compress(&cinfo);

    if (hasError) {
        jpeg_destroy_compress(&cinfo);
        free(jpegBuffer);

        return {};
    }

    // Copy the final image bytes into a vector for returning
    lib::ByteBuffer finalData(jpegBufferSize);
    for (uint32_t i = 0; i < jpegBufferSize; i++) {
        finalData[i] = jpegBuffer[i];
    }

    // Clean up the JPEG encoder, and we're done
    jpeg_destroy_compress(&cinfo);
    free(jpegBuffer);

    return finalData;
}

// Pretty self-explanatory code, we generate mip-maps down to 1x1 pixels,
// every level is half of the previous
std::vector<Image> generateMipMaps(const Image& image) {
    std::vector<Image> mipmaps;

    uint32_t width = image.getWidth();
    uint32_t height = image.getHeight();

    do {
        width /= 2;
        height /= 2;

        width = std::max<uint32_t>(1, width);
        height = std::max<uint32_t>(1, height);

        lib::ByteBuffer mipmapData(width * height * 4);
        stbir_resize_uint8(image.getPixelData().data(), image.getWidth(),
                           image.getHeight(), image.getWidth() * 4,
                           mipmapData.data(), width, height, width * 4, 4);

        Image mipmap{width, height, mipmapData};

        mipmaps.push_back(mipmap);
    } while (width != 1 || height != 1);

    return mipmaps;
}

// Encode an image as DXT5, this is again, used by multiple resources in a
// package that are compressed or encoded differently. Mip-maps are
// generated automatically.
dds::dds_file_t encodeDxt5Internal(const Image& image) {
    // Generate mip-maps and compress them
    std::vector<Image> rawMipmaps = generateMipMaps(image);
    std::vector<lib::ByteBuffer> mipmaps(rawMipmaps.size());

    for (int i = 0; i < rawMipmaps.size(); i++) {
        const Image& mipmapData = rawMipmaps[i];
        lib::ByteBuffer encoded(
            DDS_IMAGE_SIZE(mipmapData.getWidth(), mipmapData.getHeight(), 16));

        squish::CompressImage(
            mipmapData.getPixelData().data(), mipmapData.getWidth(),
            mipmapData.getHeight(), encoded.data(),
            squish::kDxt5 | squish::kColourIterativeClusterFit);

        mipmaps[i] = encoded;
    }

    // Set up the DDS header with correct values

    dds::dds_pixelformat_t pixelFormat{
        32,                               // m_size
        dds::DDPF_FOURCC,                 // m_flags
        MAKE_FOURCC('D', 'X', 'T', '5'),  // m_fourCC
        0,                                // m_rgbBitCount
        0,                                // m_rBitMask
        0,                                // m_gBitMask
        0,                                // m_bBitMask
        0                                 // m_aBitMask
    };

    dds::dds_header_t header{
        124,  // m_size
        dds::DDSD_HEIGHT | dds::DDSD_WIDTH | dds::DDSD_CAPS |
            dds::DDSD_PIXELFORMAT | dds::DDSD_MIPMAPCOUNT,  // m_flags
        image.getHeight(),
        image.getWidth(),
        0,                                  // m_pitchOrLinearSize
        1,                                  // m_depth
        1 + (uint32_t)mipmaps.size(),       // m_mipMapCount
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // m_reserved
        pixelFormat,
        dds::DDSCAPS_COMPLEX | dds::DDSCAPS_MIPMAP,  // m_caps,
        0,                                           // m_caps2
        0,                                           // m_caps3
        0                                            // m_reserved2
    };

    // Compress the main image as well

    lib::ByteBuffer mainImage(
        DDS_IMAGE_SIZE(image.getWidth(), image.getHeight(), 16));

    squish::CompressImage(image.getPixelData().data(), image.getWidth(),
                          image.getHeight(), mainImage.data(),
                          squish::kDxt5 | squish::kColourIterativeClusterFit);

    // Create the final DDS file
    dds::dds_file_t ddsFile{header, mainImage, mipmaps};

    return ddsFile;
}

dds::dds_file_t encodeDxt1Internal(const Image& image) {
    // Generate mip-maps and compress them
    std::vector<Image> rawMipmaps = generateMipMaps(image);
    std::vector<lib::ByteBuffer> mipmaps(rawMipmaps.size());

    for (int i = 0; i < rawMipmaps.size(); i++) {
        const Image& mipmapData = rawMipmaps[i];
        lib::ByteBuffer encoded(
            DDS_IMAGE_SIZE(mipmapData.getWidth(), mipmapData.getHeight(), 8));

        squish::CompressImage(
            mipmapData.getPixelData().data(), mipmapData.getWidth(),
            mipmapData.getHeight(), encoded.data(),
            squish::kDxt1 | squish::kColourIterativeClusterFit);

        mipmaps[i] = encoded;
    }

    // Set up the DDS header with correct values

    dds::dds_pixelformat_t pixelFormat{
        32,                               // m_size
        dds::DDPF_FOURCC,                 // m_flags
        MAKE_FOURCC('D', 'X', 'T', '1'),  // m_fourCC
        0,                                // m_rgbBitCount
        0,                                // m_rBitMask
        0,                                // m_gBitMask
        0,                                // m_bBitMask
        0                                 // m_aBitMask
    };

    dds::dds_header_t header{
        124,  // m_size
        dds::DDSD_HEIGHT | dds::DDSD_WIDTH | dds::DDSD_CAPS |
            dds::DDSD_PIXELFORMAT | dds::DDSD_MIPMAPCOUNT,  // m_flags
        image.getHeight(),
        image.getWidth(),
        0,                                  // m_pitchOrLinearSize
        1,                                  // m_depth
        1 + (uint32_t)mipmaps.size(),       // m_mipMapCount
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // m_reserved
        pixelFormat,
        dds::DDSCAPS_COMPLEX | dds::DDSCAPS_MIPMAP,  // m_caps,
        0,                                           // m_caps2
        0,                                           // m_caps3
        0                                            // m_reserved2
    };

    // Compress the main image as well

    lib::ByteBuffer mainImage(
        DDS_IMAGE_SIZE(image.getWidth(), image.getHeight(), 8));

    squish::CompressImage(image.getPixelData().data(), image.getWidth(),
                          image.getHeight(), mainImage.data(),
                          squish::kDxt1 | squish::kColourIterativeClusterFit);

    // Create the final DDS file
    dds::dds_file_t ddsFile{header, mainImage, mipmaps};

    return ddsFile;
}

lib::ByteBuffer encodeDst5(const Image& image) {
    dds::dds_file_t dxtFile = encodeDxt5Internal(image);

    lib::ByteBuffer imageData = concatDdsImageData(dxtFile);

    // Shuffle the blocks around
    uint32_t blockCount = (uint32_t)imageData.size() / 16;

    lib::ByteBuffer block0(blockCount * 2);
    lib::ByteBuffer block1(blockCount * 6);
    lib::ByteBuffer block2(blockCount * 4);
    lib::ByteBuffer block3(blockCount * 4);

    uint32_t c0 = 0;
    uint32_t c1 = 0;
    uint32_t c2 = 0;
    uint32_t c3 = 0;

    uint32_t sourceIdx = 0;

    for (uint32_t i = 0; i < blockCount; i++) {
        COPY_BYTES(imageData, block0, sourceIdx, 2, c0);
        sourceIdx += 2;

        COPY_BYTES(imageData, block1, sourceIdx, 6, c1);
        sourceIdx += 6;

        COPY_BYTES(imageData, block2, sourceIdx, 4, c2);
        sourceIdx += 4;

        COPY_BYTES(imageData, block3, sourceIdx, 4, c3);
        sourceIdx += 4;
    }

    lib::ByteBuffer shuffledData(imageData.size());

    uint32_t c = 0;
    COPY_BYTES(block0, shuffledData, 0, c0, c);
    COPY_BYTES(block2, shuffledData, 0, c2, c);
    COPY_BYTES(block1, shuffledData, 0, c1, c);
    COPY_BYTES(block3, shuffledData, 0, c3, c);

    reconstructDdsImageData(dxtFile, shuffledData);

    dxtFile.m_header.m_pixelFormat.m_fourCC = MAKE_FOURCC('D', 'S', 'T', '5');

    return dds::writeFile(dxtFile);
}

lib::ByteBuffer encodeDst1(const Image& image) {
    dds::dds_file_t dxtFile = encodeDxt1Internal(image);

    lib::ByteBuffer imageData = concatDdsImageData(dxtFile);

    // Shuffle the blocks around
    uint32_t blockCount = (uint32_t)imageData.size() / 8;

    lib::ByteBuffer block0(blockCount * 4);
    lib::ByteBuffer block1(blockCount * 4);

    uint32_t c0 = 0;
    uint32_t c1 = 0;

    uint32_t sourceIdx = 0;

    for (uint32_t i = 0; i < blockCount; i++) {
        COPY_BYTES(imageData, block0, sourceIdx, 4, c0);
        sourceIdx += 4;

        COPY_BYTES(imageData, block1, sourceIdx, 4, c1);
        sourceIdx += 4;
    }

    lib::ByteBuffer shuffledData(imageData.size());

    uint32_t c = 0;
    COPY_BYTES(block0, shuffledData, 0, c0, c);
    COPY_BYTES(block1, shuffledData, 0, c1, c);

    reconstructDdsImageData(dxtFile, shuffledData);

    dxtFile.m_header.m_pixelFormat.m_fourCC = MAKE_FOURCC('D', 'S', 'T', '1');

    return dds::writeFile(dxtFile);
}

lib::ByteBuffer encodeDxt5(const Image& image) {
    dds::dds_file_t dxtFile = encodeDxt5Internal(image);

    return dds::writeFile(dxtFile);
}

lib::ByteBuffer encodeDxt1(const Image& image) {
    dds::dds_file_t dxtFile = encodeDxt1Internal(image);

    return dds::writeFile(dxtFile);
}

lib::ByteBuffer encodeDxt3(const Image& image) {
    // Generate mip-maps and compress them
    std::vector<Image> rawMipmaps = generateMipMaps(image);
    std::vector<lib::ByteBuffer> mipmaps(rawMipmaps.size());

    for (int i = 0; i < rawMipmaps.size(); i++) {
        const Image& mipmapData = rawMipmaps[i];
        lib::ByteBuffer encoded(
            DDS_IMAGE_SIZE(mipmapData.getWidth(), mipmapData.getHeight(), 16));

        squish::CompressImage(
            mipmapData.getPixelData().data(), mipmapData.getWidth(),
            mipmapData.getHeight(), encoded.data(),
            squish::kDxt3 | squish::kColourIterativeClusterFit);

        mipmaps[i] = encoded;
    }

    // Set up the DDS header with correct values

    dds::dds_pixelformat_t pixelFormat{
        32,                               // m_size
        dds::DDPF_FOURCC,                 // m_flags
        MAKE_FOURCC('D', 'X', 'T', '3'),  // m_fourCC
        0,                                // m_rgbBitCount
        0,                                // m_rBitMask
        0,                                // m_gBitMask
        0,                                // m_bBitMask
        0                                 // m_aBitMask
    };

    dds::dds_header_t header{
        124,  // m_size
        dds::DDSD_HEIGHT | dds::DDSD_WIDTH | dds::DDSD_CAPS |
            dds::DDSD_PIXELFORMAT | dds::DDSD_MIPMAPCOUNT,  // m_flags
        image.getHeight(),
        image.getWidth(),
        0,                                  // m_pitchOrLinearSize
        1,                                  // m_depth
        1 + (uint32_t)mipmaps.size(),       // m_mipMapCount
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  // m_reserved
        pixelFormat,
        dds::DDSCAPS_COMPLEX | dds::DDSCAPS_MIPMAP,  // m_caps,
        0,                                           // m_caps2
        0,                                           // m_caps3
        0                                            // m_reserved2
    };

    // Compress the main image as well

    lib::ByteBuffer mainImage(
        DDS_IMAGE_SIZE(image.getWidth(), image.getHeight(), 16));

    squish::CompressImage(image.getPixelData().data(), image.getWidth(),
                          image.getHeight(), mainImage.data(),
                          squish::kDxt3 | squish::kColourIterativeClusterFit);

    // Create the final DDS file
    dds::dds_file_t ddsFile{header, mainImage, mipmaps};

    return dds::writeFile(ddsFile);
}

// Implementation of the s4pkg::internal::imagecoder::(decode / encode)
// functions, which just delegate to the actual implementations defined
// above

typedef std::shared_ptr<Image> (*decoderFunction_t)(const lib::ByteBuffer&);

typedef lib::ByteBuffer (*encoderFunction_t)(const Image&);

const std::unordered_map<ImageFormat, decoderFunction_t> g_decoderMapping = {
    {JFIF_WITH_ALPHA, &decodeJfifWithAlpha},
    {DST5, &decodeDst5},
    {DXT5, &decodeDxt5},
    {DST1, &decodeDst1},
    {DXT1, &decodeDxt1},
    {DXT3, &decodeDxt3},
    {DDS_UNCOMPRESSED, &decodeUncompressedDds},
    {RLE2, &decodeRle},
    {RLES, &decodeRle},
};

const std::unordered_map<ImageFormat, encoderFunction_t> g_encoderMapping = {
    {JFIF_WITH_ALPHA, &encodeJfifWithAlpha},
    {DST5, &encodeDst5},
    {DXT5, &encodeDxt5},
    {DST1, &encodeDst1},
    {DXT1, &encodeDxt1},
    {DXT3, &encodeDxt3},
    // TODO: Encoder for uncompressed DDS
    // TODO: Encoder for RLE2
    // TODO: Encoder for RLES
};

template <typename T>
T tryGetFunction(const std::unordered_map<ImageFormat, T>& mapping,
                 ImageFormat key) {
    if (mapping.find(key) != mapping.end()) {
        return mapping.at(key);
    } else {
        return nullptr;
    }
}

std::shared_ptr<Image> decode(const lib::ByteBuffer& data, ImageFormat format) {
    decoderFunction_t chosenDecoder = tryGetFunction(g_decoderMapping, format);
    if (chosenDecoder != nullptr && data.size() > 0) {
        return chosenDecoder(data);
    }

    return nullptr;
}

lib::ByteBuffer encode(const Image& image, ImageFormat format) {
    encoderFunction_t chosenEncoder = tryGetFunction(g_encoderMapping, format);
    if (chosenEncoder != nullptr) {
        return chosenEncoder(image);
    }

    return {};
}
};  // namespace s4pkg::internal::imagecoder
