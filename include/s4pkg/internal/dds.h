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

#ifndef MAKE_FOURCC
#define MAKE_FOURCC(ch0, ch1, ch2, ch3)                           \
    ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
     ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif

// Taken from
// https://learn.microsoft.com/en-gb/windows/win32/direct3ddds/dds-file-layout-for-textures
#define DDS_IMAGE_SIZE(w, h, bs)           \
    std::max<uint32_t>(1, ((w + 3) / 4)) * \
        std::max<uint32_t>(1, ((h + 3) / 4)) * bs

namespace s4pkg::internal::dds {

// For a detailed description of these structs and their members, see:
// https://learn.microsoft.com/fi-fi/windows/win32/direct3ddds/dx-graphics-dds-pguide#dds-file-layout

typedef enum dds_pixelformat_flags_t {
    DDPF_ALPHAPIXELS = 0x1,
    DDPF_ALPHA = 0x2,
    DDPF_FOURCC = 0x4,
    DDPF_RGB = 0x40,
    DDPF_YUV = 0x200,
    DDPF_LUMINANCE = 0x20000
} dds_pixelformat_flags_t;

typedef struct dds_pixelformat_t {
    uint32_t m_size;
    uint32_t m_flags;
    uint32_t m_fourCC;
    uint32_t m_rgbBitCount;
    uint32_t m_rBitMask;
    uint32_t m_gBitMask;
    uint32_t m_bBitMask;
    uint32_t m_aBitMask;
} dds_pixelformat_t;

typedef enum dds_header_flags_t {
    DDSD_CAPS = 0x1,
    DDSD_HEIGHT = 0x2,
    DDSD_WIDTH = 0x4,
    DDSD_PITCH = 0x8,
    DDSD_PIXELFORMAT = 0x1000,
    DDSD_MIPMAPCOUNT = 0x20000,
    DDSD_LINEARSIZE = 0x80000,
    DDSD_DEPTH = 0x800000
} dds_header_flags_t;

typedef enum dds_header_caps_t {
    DDSCAPS_COMPLEX = 0x8,
    DDSCAPS_MIPMAP = 0x400000,
    DDSCAPS_TEXTURE = 0x1000
} dds_header_caps_t;

typedef struct dds_header_t {
    uint32_t m_size;
    uint32_t m_flags;
    uint32_t m_height;
    uint32_t m_width;
    uint32_t m_pitchOrLinearSize;
    uint32_t m_depth;
    uint32_t m_mipMapCount;
    uint32_t m_reserved[11];
    dds_pixelformat_t m_pixelFormat;
    uint32_t m_caps;
    uint32_t m_caps2;
    uint32_t m_caps3;
    uint32_t m_caps4;
    uint32_t m_reserved2;
} dds_header_t;

std::string pixelFormatToString(const dds_pixelformat_t&);
std::string headerToString(const dds_header_t&);

dds_pixelformat_t readPixelFormat(std::istream&);
dds_header_t readHeader(std::istream&);

typedef struct dds_file_t {
    dds_header_t m_header;
    std::vector<uint8_t> m_mainImage;
    std::vector<std::vector<uint8_t>> m_mipmaps;
} dds_file_t;

/**
 * @brief Read a DDS file. Compressed files only, that's what the game uses
 * anyways
 * @return whether the reading was successful or not
 */
bool readFile(std::istream&, dds_file_t&);

std::string fileToString(const dds_file_t&);

};  // namespace s4pkg::internal::dds
