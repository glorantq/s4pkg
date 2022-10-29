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

namespace s4pkg::internal::imagecoder {

// Decoders

// Encoders

// Implementation of the s4pkg::internal::imagecoder::(decode / encode)
// functions, which just delegate to the actual implementations defined above

typedef std::shared_ptr<Image> (*t_decoderFunction)(std::vector<uint8_t>,
                                                    ImageFormat);

typedef std::vector<uint8_t> (*t_encoderFunction)(Image&, ImageFormat);

const std::unordered_map<ImageFormat, t_decoderFunction> g_decoderMapping = {

};

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

std::shared_ptr<Image> decode(std::vector<uint8_t> data, ImageFormat format) {
    t_decoderFunction chosenDecoder = tryGetFunction(g_decoderMapping, format);
    if (chosenDecoder != nullptr) {
        return chosenDecoder(data, format);
    }

    return nullptr;
}

std::vector<uint8_t> encode(Image& image, ImageFormat format) {
    t_encoderFunction chosenEncoder = tryGetFunction(g_encoderMapping, format);
    if (chosenEncoder != nullptr) {
        return chosenEncoder(image, format);
    }

    return {};
}

};  // namespace s4pkg::internal::imagecoder
