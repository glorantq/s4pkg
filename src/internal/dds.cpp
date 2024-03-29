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

#include <ostream>
#include <vector>

#include <fmt/core.h>
#include <fmt/printf.h>
#include <fmt/ranges.h>

#include <s4pkg/internal/membuf.h>
#include <s4pkg/internal/streams.h>
#include <s4pkg/packageexception.h>

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

void readCompressedImageData(std::istream& stream, dds_file_t& file) {
    uint32_t width = file.m_header.m_width;
    uint32_t height = file.m_header.m_height;

    // Calculate the block size, see:
    // https://learn.microsoft.com/en-gb/windows/win32/direct3ddds/dds-file-layout-for-textures
    uint8_t blockSize = 16;
    if (file.m_header.m_pixelFormat.m_fourCC ==
            MAKE_FOURCC('D', 'X', 'T', '1') ||
        file.m_header.m_pixelFormat.m_fourCC ==
            MAKE_FOURCC('D', 'S', 'T', '1')) {
        blockSize = 8;
    }

    // Read in the main image
    try {
        file.m_mainImage =
            lib::ByteBuffer(DDS_IMAGE_SIZE(width, height, blockSize));
        streams::readBytes(stream, file.m_mainImage.data(),
                           (int)file.m_mainImage.size());
    } catch (PackageException e) {
        throw PackageException(
            fmt::format("Reading DDS main image (compressed) (requested size: "
                        "{}, position: "
                        "{}, block size: {}, header: {}): {}",
                        file.m_mainImage.size(), stream.tellg(), blockSize,
                        headerToString(file.m_header), e.what()));
    }

    // Then make room for the mipmaps
    file.m_mipmaps = std::vector<lib::ByteBuffer>(
        std::max<int32_t>(0, file.m_header.m_mipMapCount - 1));

    // And read them in (if there are any)
    for (int i = 0; i < file.m_mipmaps.size(); i++) {
        width /= 2;
        height /= 2;

        width = std::max<uint32_t>(1, width);
        height = std::max<uint32_t>(1, height);

        lib::ByteBuffer mipmap(DDS_IMAGE_SIZE(width, height, blockSize));
        streams::readBytes(stream, mipmap.data(), (int)mipmap.size());

        file.m_mipmaps[i] = mipmap;
    }
}

void readUncompressedImageData(std::istream& stream, dds_file_t& file) {
    uint32_t width = file.m_header.m_width;
    uint32_t height = file.m_header.m_height;

    uint8_t bytesPerPixel = file.m_header.m_pixelFormat.m_rgbBitCount / 8;

    // Read in the main image
    try {
        file.m_mainImage = lib::ByteBuffer(width * height * bytesPerPixel);
        streams::readBytes(stream, file.m_mainImage.data(),
                           (int)file.m_mainImage.size());
    } catch (PackageException e) {
        throw PackageException(fmt::format(
            "Reading DDS main image (uncompressed) (requested size: "
            "{}, position: "
            "{}, bpp: {}, header: {}): {}",
            file.m_mainImage.size(), stream.tellg(), bytesPerPixel,
            headerToString(file.m_header), e.what()));
    }

    // Then make room for the mipmaps
    file.m_mipmaps = std::vector<lib::ByteBuffer>(
        std::max<int32_t>(0, file.m_header.m_mipMapCount - 1));

    // And read them in (if there are any)
    for (int i = 0; i < file.m_mipmaps.size(); i++) {
        width /= 2;
        height /= 2;

        width = std::max<uint32_t>(1, width);
        height = std::max<uint32_t>(1, height);

        lib::ByteBuffer mipmap(width * height * bytesPerPixel);
        streams::readBytes(stream, mipmap.data(), (int)mipmap.size());

        file.m_mipmaps[i] = mipmap;
    }
}

bool readFile(std::istream& stream, dds_file_t& file) {
    uint8_t magicBytes[4];
    uint8_t expectedMagic[] = {'D', 'D', 'S', ' '};

    streams::readBytes(stream, magicBytes, 4);

    // Verify that this actually is a DDS file
    for (int i = 0; i < 4; i++) {
        if (magicBytes[i] != expectedMagic[i]) {
            return false;
        }
    }

    try {
        file.m_header = readHeader(stream);
    } catch (PackageException e) {
        throw PackageException(fmt::format("Reading DDS header: {}", e.what()));
    }

    // Verify the two required tags (there are other tags that are required, but
    // Microsoft recommends against checking for them), see:
    // https://learn.microsoft.com/fi-fi/windows/win32/direct3ddds/dds-header
    if ((file.m_header.m_flags & DDSD_HEIGHT) == 0 ||
        (file.m_header.m_flags & DDSD_WIDTH) == 0) {
        return false;
    }

    // Read in the image data

    if ((file.m_header.m_pixelFormat.m_flags & DDPF_FOURCC) !=
        0) {  // Compressed image
        readCompressedImageData(stream, file);
    } else if ((file.m_header.m_pixelFormat.m_flags & DDPF_RGB) !=
               0) {  // Uncompressed image
        readUncompressedImageData(stream, file);
    } else {
        throw PackageException(
            fmt::format("Unrecognised DDS file! {}", fileToString(file)));
    }

    return true;
}

// We're going for functionality over looks here
lib::ByteBuffer writeFile(const dds_file_t& file) {
    uint8_t magicBytes[] = {'D', 'D', 'S', ' '};

    uint32_t imageDataSize = (uint32_t)file.m_mainImage.size();
    for (const auto& mipmap : file.m_mipmaps) {
        imageDataSize += (uint32_t)mipmap.size();
    }

    uint32_t totalFileSize =
        sizeof(magicBytes) + sizeof(dds_header_t) + imageDataSize;

    lib::ByteBuffer output(totalFileSize);

    uint32_t c = 0;
    COPY_BYTES(magicBytes, output, 0, 4, c);
    COPY_UINT32(file.m_header.m_size, output, c);
    COPY_UINT32(file.m_header.m_flags, output, c);
    COPY_UINT32(file.m_header.m_height, output, c);
    COPY_UINT32(file.m_header.m_width, output, c);
    COPY_UINT32(file.m_header.m_pitchOrLinearSize, output, c);
    COPY_UINT32(file.m_header.m_depth, output, c);
    COPY_UINT32(file.m_header.m_mipMapCount, output, c);

    for (int i = 0; i < 11; i++) {
        COPY_UINT32(file.m_header.m_reserved[i], output, c);
    }

    {
        COPY_UINT32(file.m_header.m_pixelFormat.m_size, output, c);
        COPY_UINT32(file.m_header.m_pixelFormat.m_flags, output, c);
        COPY_UINT32(file.m_header.m_pixelFormat.m_fourCC, output, c);
        COPY_UINT32(file.m_header.m_pixelFormat.m_rgbBitCount, output, c);
        COPY_UINT32(file.m_header.m_pixelFormat.m_rBitMask, output, c);
        COPY_UINT32(file.m_header.m_pixelFormat.m_gBitMask, output, c);
        COPY_UINT32(file.m_header.m_pixelFormat.m_bBitMask, output, c);
        COPY_UINT32(file.m_header.m_pixelFormat.m_aBitMask, output, c);
    }

    COPY_UINT32(file.m_header.m_caps, output, c);
    COPY_UINT32(file.m_header.m_caps2, output, c);
    COPY_UINT32(file.m_header.m_caps3, output, c);
    COPY_UINT32(file.m_header.m_caps4, output, c);
    COPY_UINT32(file.m_header.m_reserved2, output, c);

    COPY_BYTES(file.m_mainImage, output, 0, file.m_mainImage.size(), c);

    for (const auto& mipmap : file.m_mipmaps) {
        COPY_BYTES(mipmap, output, 0, mipmap.size(), c);
    }

    return output;
}

std::string fileToString(const dds_file_t& file) {
    return fmt::format("[ header={}, mainImageSize={}, readMipMaps={} ]",
                       headerToString(file.m_header), file.m_mainImage.size(),
                       file.m_mipmaps.size());
}

};  // namespace s4pkg::internal::dds
