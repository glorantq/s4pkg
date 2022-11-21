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
#include <s4pkg/internal/membuf.h>
#include <s4pkg/internal/rle.h>
#include <s4pkg/resources/imageresource.h>

namespace s4pkg::resources::ts4 {

class S4PKG_EXPORT RLEResource : public IImageResource {
   public:
    RLEResource(s4pkg::ResourceType originalType,
                uint32_t instanceEx,
                uint32_t instance,
                uint32_t group,
                const lib::ByteBuffer& data)
        : IImageResource(instanceEx, instance, group, originalType) {
        // Provide a way to make an empty image
        if (data.size() < sizeof(internal::rle::rle_header_t)) {
            setDataWithFormat(internal::imagecoder::RLE2, data);
            return;
        }

        // Read in the RLE file from memory
        internal::membuf memoryBuffer(data.data(), data.size());
        std::istream stream(&memoryBuffer);

        internal::rle::rle_header_t rleHeader =
            internal::rle::readHeader(stream);

        auto imageFormat = internal::imagecoder::UNKNOWN;

        if (rleHeader.m_rleVersion == internal::rle::rle_version_t::RLE2) {
            imageFormat = internal::imagecoder::RLE2;
        } else if (rleHeader.m_rleVersion ==
                   internal::rle::rle_version_t::RLES) {
            imageFormat = internal::imagecoder::RLES;
        }

        setDataWithFormat(imageFormat, data);
    }

    // IResource interface
   public:
    lib::String getFriendlyName() const override { return "RLE Image"; }

    // Object interface
   public:
    const lib::String toString() const override;
};

}  // namespace s4pkg::resources::ts4
