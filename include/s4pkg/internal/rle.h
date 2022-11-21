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

#include <cinttypes>
#include <istream>
#include <string>
#include <vector>

#include <s4pkg/internal/dds.h>
#include <s4pkg/internal/export.h>

// TODO: Should only be exported when actively developing, users should not need
// to rely on these functions

namespace s4pkg::internal::rle {

typedef enum fourcc_t {
    L8 = 0x2020384C,
    DXT5 = 0x35545844,
} fourcc_t;

typedef enum rle_version_t {
    RLE2 = 0x32454C52,
    RLES = 0x53454C52,
} rle_version_t;

typedef struct mip_header_t {
    int32_t m_commandOffset;
    int32_t m_offset2;
    int32_t m_offset3;
    int32_t m_offset0;
    int32_t m_offset1;
    int32_t m_offset4;
} mip_header_t;

typedef struct rle_header_t {
    uint32_t m_fourCC;
    uint32_t m_rleVersion;

    uint16_t m_width;
    uint16_t m_height;
    uint16_t m_mipCount;

    uint16_t m_unknown0;

    std::vector<mip_header_t> m_mipHeaders;

    uint32_t m_streamSize;
} rle_header_t;

typedef struct rle_file_t {
    rle_header_t m_rleHeader;
    std::vector<uint8_t> m_rleData;
    dds::dds_file_t m_ddsFile;
} rle_file_t;

rle_header_t readHeader(std::istream&);
S4PKG_EXPORT rle_file_t readFile(std::istream&);

};  // namespace s4pkg::internal::rle
