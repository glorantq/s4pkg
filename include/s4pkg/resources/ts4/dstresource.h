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

#include <s4pkg/internal/dds.h>
#include <s4pkg/internal/export.h>
#include <s4pkg/internal/membuf.h>
#include <s4pkg/resources/imageresource.h>

#include <istream>

#include <fmt/printf.h>

namespace s4pkg::resources::ts4 {

class S4PKG_EXPORT DSTResource : public IImageResource {
   public:
    DSTResource(s4pkg::ResourceType originalType,
                uint32_t instanceEx,
                uint32_t instance,
                uint32_t group,
                const std::vector<uint8_t>& data)
        : IImageResource(instanceEx, instance, group, originalType) {
        // Provide a way to make an empty image
        if (data.size() < sizeof(internal::dds::dds_header_t)) {
            setDataWithFormat(internal::imagecoder::DST5, data);
            return;
        }

        // Read in the DDS file from memory
        internal::membuf memoryBuffer(data.data(), data.size());
        std::istream stream(&memoryBuffer);

        // Skip past the magic bytes, we just want to guess here, if the format
        // we guess here is incorrect the proper error-checking in the image
        // coder will catch it
        stream.seekg(4);

        internal::dds::dds_header_t ddsHeader =
            internal::dds::readHeader(stream);

        auto imageFormat = internal::imagecoder::UNKNOWN;

        if (ddsHeader.m_pixelFormat.m_fourCC ==
            MAKE_FOURCC('D', 'S', 'T', '5')) {
            imageFormat = internal::imagecoder::DST5;
        } else if (ddsHeader.m_pixelFormat.m_fourCC ==
                   MAKE_FOURCC('D', 'X', 'T', '5')) {
            imageFormat = internal::imagecoder::DXT5;
        } else if (ddsHeader.m_pixelFormat.m_fourCC ==
                   MAKE_FOURCC('D', 'X', 'T', '1')) {
            imageFormat = internal::imagecoder::DXT1;
        } else if (ddsHeader.m_pixelFormat.m_fourCC ==
                   MAKE_FOURCC('D', 'S', 'T', '1')) {
            imageFormat = internal::imagecoder::DST1;
        } else if (ddsHeader.m_pixelFormat.m_fourCC ==
                   MAKE_FOURCC('D', 'X', 'T', '3')) {
            imageFormat = internal::imagecoder::DXT3;
        } else if ((ddsHeader.m_pixelFormat.m_flags &
                    internal::dds::DDPF_RGB) != 0) {
            imageFormat = internal::imagecoder::DDS_UNCOMPRESSED;
        } else {
            fmt::printf(
                "Unknown pixel format: %s\n",
                internal::dds::pixelFormatToString(ddsHeader.m_pixelFormat));
        }

        setDataWithFormat(imageFormat, data);
    }

    // IResource interface
   public:
    std::string getFriendlyName() const override { return "DST/DXT Image"; }

    // Object interface
   public:
    const std::string toString() const override;
};

}  // namespace s4pkg::resources::ts4
