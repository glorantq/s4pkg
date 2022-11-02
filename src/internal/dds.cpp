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

#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <s4pkg/internal/streams.h>

namespace s4pkg::internal::dds {

std::string pixelFormatToString(const dds_pixelformat_t& pixelFormat) {
    std::vector<std::string> flagsStrings;
    if ((pixelFormat.m_flags & DDPF_ALPHAPIXELS) > 0) {
        flagsStrings.push_back("DDPF_ALPHAPIXELS");
    }

    if ((pixelFormat.m_flags & DDPF_ALPHA) > 0) {
        flagsStrings.push_back("DDPF_ALPA");
    }

    if ((pixelFormat.m_flags & DDPF_FOURCC) > 0) {
        flagsStrings.push_back("DDPF_FOURCC");
    }

    if ((pixelFormat.m_flags & DDPF_RGB) > 0) {
        flagsStrings.push_back("DDPF_RGB");
    }

    if ((pixelFormat.m_flags & DDPF_YUV) > 0) {
        flagsStrings.push_back("DDPF_YUV");
    }

    if ((pixelFormat.m_flags & DDPF_LUMINANCE) > 0) {
        flagsStrings.push_back("DDPF_LUMINANCE");
    }

    char fourCCString[5] = {0, 0, 0, 0, 0};
    fourCCString[3] = pixelFormat.m_fourCC >> 24 & 0xFF;
    fourCCString[2] = pixelFormat.m_fourCC >> 16 & 0xFF;
    fourCCString[1] = pixelFormat.m_fourCC >> 8 & 0xFF;
    fourCCString[0] = pixelFormat.m_fourCC & 0xFF;

    return fmt::format(
        "[ flags=({}), fourCC={}, rgbBitCount={}, rBitMask={}, gBitMask={}, "
        "bBitMask={}, aBitMask={} ]",
        fmt::join(flagsStrings, " | "), fourCCString, pixelFormat.m_rgbBitCount,
        pixelFormat.m_rBitMask, pixelFormat.m_gBitMask, pixelFormat.m_bBitMask,
        pixelFormat.m_aBitMask);
}

std::string headerToString(const dds_header_t& header) {
    std::vector<std::string> flagsStrings;
    if ((header.m_flags & DDSD_CAPS) > 0) {
        flagsStrings.push_back("DDSD_CAPS");
    }

    if ((header.m_flags & DDSD_HEIGHT) > 0) {
        flagsStrings.push_back("DDSD_HEIGHT");
    }

    if ((header.m_flags & DDSD_WIDTH) > 0) {
        flagsStrings.push_back("DDSD_WIDTH");
    }

    if ((header.m_flags & DDSD_PITCH) > 0) {
        flagsStrings.push_back("DDSD_PITCH");
    }

    if ((header.m_flags & DDSD_PIXELFORMAT) > 0) {
        flagsStrings.push_back("DDSD_PIXELFORMAT");
    }

    if ((header.m_flags & DDSD_MIPMAPCOUNT) > 0) {
        flagsStrings.push_back("DDSD_MIPMAPCOUNT");
    }

    if ((header.m_flags & DDSD_LINEARSIZE) > 0) {
        flagsStrings.push_back("DDSD_LINEARSIZE");
    }

    if ((header.m_flags & DDSD_DEPTH) > 0) {
        flagsStrings.push_back("DDSD_DEPTH");
    }

    std::vector<std::string> capsStrings;
    if ((header.m_caps & DDSCAPS_COMPLEX) > 0) {
        capsStrings.push_back("DDSCAPS_COMPLEX");
    }

    if ((header.m_caps & DDSCAPS_MIPMAP) > 0) {
        capsStrings.push_back("DDSCAPS_MIPMAP");
    }

    if ((header.m_caps & DDSCAPS_TEXTURE) > 0) {
        capsStrings.push_back("DDSCAPS_TEXTURE");
    }

    return fmt::format(
        "[ size={}, flags=({}), height={}, width={}, pitchOrLinearSize={}, "
        "depth={}, "
        "mipMapCount={}, pixelFormat={}, caps=({}), caps2={}, "
        "caps3={}, caps4={} "
        "]",
        header.m_size, fmt::join(flagsStrings, " | "), header.m_height,
        header.m_width, header.m_pitchOrLinearSize, header.m_depth,
        header.m_mipMapCount, pixelFormatToString(header.m_pixelFormat),
        fmt::join(capsStrings, " | "), header.m_caps2, header.m_caps3,
        header.m_caps4);
}

dds_pixelformat_t readPixelFormat(std::istream& stream) {
    dds_pixelformat_t pixelFormat{};

    streams::readUint32(stream, pixelFormat.m_size);
    streams::readUint32(stream, pixelFormat.m_flags);
    streams::readUint32(stream, pixelFormat.m_fourCC);
    streams::readUint32(stream, pixelFormat.m_rgbBitCount);
    streams::readUint32(stream, pixelFormat.m_rBitMask);
    streams::readUint32(stream, pixelFormat.m_gBitMask);
    streams::readUint32(stream, pixelFormat.m_bBitMask);
    streams::readUint32(stream, pixelFormat.m_aBitMask);

    return pixelFormat;
}

dds_header_t readHeader(std::istream& stream) {
    uint8_t magic[4];
    streams::readBytes(stream, magic, 4);

    dds_header_t header{};

    streams::readUint32(stream, header.m_size);
    streams::readUint32(stream, header.m_flags);
    streams::readUint32(stream, header.m_height);
    streams::readUint32(stream, header.m_width);
    streams::readUint32(stream, header.m_pitchOrLinearSize);
    streams::readUint32(stream, header.m_depth);
    streams::readUint32(stream, header.m_mipMapCount);
    streams::readUint32Array(stream, header.m_reserved, 11);

    header.m_pixelFormat = readPixelFormat(stream);

    streams::readUint32(stream, header.m_caps);
    streams::readUint32(stream, header.m_caps2);
    streams::readUint32(stream, header.m_caps3);
    streams::readUint32(stream, header.m_caps4);
    streams::readUint32(stream, header.m_reserved2);

    return header;
}

};  // namespace s4pkg::internal::dds
