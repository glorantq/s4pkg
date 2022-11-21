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

#include <s4pkg/internal/export.h>
#include <s4pkg/lib/bytebuffer.h>
#include <s4pkg/object.h>

#include <inttypes.h>
#include <vector>
namespace s4pkg::internal {

class Image : public Object {
   private:
    lib::ByteBuffer m_pixelData;
    uint32_t m_width;
    uint32_t m_height;

   public:
    Image(uint32_t width, uint32_t height, const lib::ByteBuffer& pixelData)
        : m_pixelData(pixelData), m_width(width), m_height(height) {}

    const uint32_t getWidth() const { return this->m_width; }
    const uint32_t getHeight() const { return this->m_height; }
    const lib::ByteBuffer getPixelData() const { return this->m_pixelData; }

    // Object interface
   public:
    const lib::String toString() const override;
};

}  // namespace s4pkg::internal
