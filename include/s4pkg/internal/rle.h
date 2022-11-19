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
