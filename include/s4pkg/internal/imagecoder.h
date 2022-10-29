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

#pragma once

#include <s4pkg/internal/image.h>

#include <memory>

namespace s4pkg::internal::imagecoder {

/**
 * @brief Formats of images we can convert from/to. Not all might be implemented
 * just yet.
 */
enum ImageFormat { LRLE, RLE2, RLES, DST, JFIF_WITH_ALPHA };

/**
 * @brief Decodes an image into RGBA pixels
 * @param data: raw data
 * @param format: the format of the image to decode
 * @return an Image class if successful, nullptr on failure
 */
std::shared_ptr<Image> decode(std::vector<uint8_t> data, ImageFormat format);

/**
 * @brief Encodes an RGBA image into raw bytes
 * @param image: the image to encode
 * @param format: the format to encode to
 * @return the encoded bytes (empty on failure)
 */
std::vector<uint8_t> encode(Image& image, ImageFormat format);

};  // namespace s4pkg::internal::imagecoder
