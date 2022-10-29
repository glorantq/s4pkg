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

#include <unordered_map>

#include <stb_image.h>

namespace s4pkg::internal::imagecoder {

// Decoders

std::shared_ptr<Image> decodeJfifWithAlpha(const std::vector<uint8_t>& data) {
    // Validate that this is in fact a JFIF file

    if (data[0] != 0xFF || data[1] != 0xD8) {  // Start of Image
        return nullptr;
    }

    // Decode the whole file as a JPEG, to get the RGB values.

    int width = 0;
    int height = 0;
    int channelsInFile = 0;

    unsigned char* decodedFull = stbi_load_from_memory(
        data.data(), data.size(), &width, &height, &channelsInFile, 3);

    if (decodedFull == nullptr) {
        return nullptr;
    }

    // We extract the alpha, this is stored in an APP0 tag right after JFIF-APP0

    // First two bytes in the file are the SOI tag, then comes the first APP0
    // marker for JFIF-APP0, then the size of that field. The alpha is stored
    // right after this, in a non-standard way.
    uint32_t jfifApp0Start = 2;
    uint16_t jfifApp0Length = ((data[jfifApp0Start + 2] << 8) & 0xFF) |
                              (data[jfifApp0Start + 3] & 0xFF);

    uint32_t alphaSegmentStart = jfifApp0Start + 2 + jfifApp0Length;

    // We should have a marker here

    if (data[alphaSegmentStart] != 0xFF ||
        data[alphaSegmentStart + 1] != 0xE0) {
        return nullptr;
    }

    // Read the size of the alpha field

    uint16_t alphaApp0Length = ((data[alphaSegmentStart + 2] << 8) & 0xFF) |
                               (data[alphaSegmentStart + 3] & 0xFF);

    // Verify this actually is the alpha of the image
    if (data[alphaSegmentStart + 4] != 'A' ||
        data[alphaSegmentStart + 5] != 'L' ||
        data[alphaSegmentStart + 6] != 'F' ||
        data[alphaSegmentStart + 7] != 'A') {
        return nullptr;
    }

    // Read the size of the image
    uint32_t alphaImageLength = ((data[alphaSegmentStart + 8] << 24) & 0xFF) |
                                ((data[alphaSegmentStart + 9] << 16) & 0xFF) |
                                ((data[alphaSegmentStart + 10] << 8) & 0xFF) |
                                (data[alphaSegmentStart + 11] & 0xFF);

    // Read the alpha image
    std::vector<uint8_t> alphaData(alphaImageLength);

    for (int i = 0; i < alphaImageLength; i++) {
        alphaData[i] = data[alphaSegmentStart + 12 + i];
    }

    // Decode the alpha
    int alphaWidth = 0;
    int alphaHeight = 0;
    int alphaChannelsInFile = 0;
    unsigned char* alphaImage =
        stbi_load_from_memory(alphaData.data(), alphaData.size(), &alphaWidth,
                              &alphaHeight, &alphaChannelsInFile, 1);

    if (alphaImage == nullptr) {
        return nullptr;
    }

    // Verify that the two images have the same dimensions
    if (width != alphaWidth || height != alphaHeight) {
        return nullptr;
    }

    std::vector<uint8_t> finalImageData(width * height * 4);

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

    stbi_image_free(decodedFull);
    stbi_image_free(alphaImage);

    return std::make_shared<Image>(width, height, finalImageData);
}

// Encoders

// Implementation of the s4pkg::internal::imagecoder::(decode / encode)
// functions, which just delegate to the actual implementations defined
// above

typedef std::shared_ptr<Image> (*t_decoderFunction)(
    const std::vector<uint8_t>&);

typedef std::vector<uint8_t> (*t_encoderFunction)(const Image&, ImageFormat);

const std::unordered_map<ImageFormat, t_decoderFunction> g_decoderMapping = {
    {JFIF_WITH_ALPHA, &decodeJfifWithAlpha}};

const std::unordered_map<ImageFormat, t_encoderFunction> g_encoderMapping = {

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
        return chosenEncoder(image, format);
    }

    return {};
}

};  // namespace s4pkg::internal::imagecoder
