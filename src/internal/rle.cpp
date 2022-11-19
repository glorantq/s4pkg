#include <s4pkg/internal/rle.h>

#include <s4pkg/internal/streams.h>
#include <s4pkg/packageexception.h>

#include <fmt/core.h>

s4pkg::internal::rle::rle_header_t s4pkg::internal::rle::readHeader(
    std::istream& stream) {
    // Get the size of the whole stream, we'll need this later
    stream.seekg(0, std::istream::end);
    int32_t streamSize = stream.tellg();
    stream.seekg(0, std::istream::beg);

    // Read in basic information about the file, such as the RLE version and the
    // underlying format (DXT5 or L8 as far as I can tell)
    rle_header_t header{};

    streams::readUint32(stream, header.m_fourCC);
    streams::readUint32(stream, header.m_rleVersion);

    streams::readUint16(stream, header.m_width);
    streams::readUint16(stream, header.m_height);
    streams::readUint16(stream, header.m_mipCount);

    streams::readUint16(stream, header.m_unknown0);

    header.m_mipHeaders = std::vector<mip_header_t>(header.m_mipCount + 1);

    // Read in the headers for all mipmaps
    for (int i = 0; i < header.m_mipCount; i++) {
        mip_header_t mipHeader{};

        if (header.m_fourCC == fourcc_t::L8) {
            streams::readInt32(stream, mipHeader.m_commandOffset);
            streams::readInt32(stream, mipHeader.m_offset0);
        } else {
            streams::readInt32(stream, mipHeader.m_commandOffset);
            streams::readInt32(stream, mipHeader.m_offset2);
            streams::readInt32(stream, mipHeader.m_offset3);
            streams::readInt32(stream, mipHeader.m_offset0);
            streams::readInt32(stream, mipHeader.m_offset1);

            if (header.m_rleVersion == rle_version_t::RLES) {
                streams::readInt32(stream, mipHeader.m_offset4);
            }
        }

        header.m_mipHeaders[i] = mipHeader;
    }

    // This is needed in the decode function for sanity-checking the
    // decompression
    if (header.m_rleVersion == rle_version_t::RLES) {
        header.m_mipHeaders[header.m_mipCount] = {
            header.m_mipHeaders[0].m_offset2, header.m_mipHeaders[0].m_offset3,
            header.m_mipHeaders[0].m_offset0, header.m_mipHeaders[0].m_offset1,
            header.m_mipHeaders[0].m_offset4, streamSize,
        };
    } else {
        header.m_mipHeaders[header.m_mipCount] = {
            header.m_mipHeaders[0].m_offset2,
            header.m_mipHeaders[0].m_offset3,
            header.m_mipHeaders[0].m_offset0,
            header.m_mipHeaders[0].m_offset1,
            streamSize,
            0,
        };
    }

    header.m_streamSize = streamSize;
    return header;
}

// Constant blocks for decompression
static constexpr uint8_t g_fullDark[] = {0x00, 0x00, 0x00, 0x00};
static constexpr uint8_t g_fullBright[] = {0xFF, 0xFF, 0xFF, 0xFF};
static constexpr uint8_t g_fullTransparentAlpha[] = {0x00, 0x05, 0x00, 0x00,
                                                     0x00, 0x00, 0x00, 0x00};
static constexpr uint8_t g_fullTransparentWhite[] = {0xFF, 0xFF, 0x00, 0x00,
                                                     0x00, 0x00, 0x00, 0x00};
static constexpr uint8_t g_fullTransparentBlack[] = {0x00, 0x00, 0x00, 0x00,
                                                     0x00, 0x00, 0x00, 0x00};
static constexpr uint8_t g_fullOpaqueAlpha[] = {0x00, 0x05, 0xFF, 0xFF,
                                                0xFF, 0xFF, 0xFF, 0xFF};

s4pkg::internal::rle::rle_file_t s4pkg::internal::rle::readFile(
    std::istream& stream) {
    rle_file_t rleFile{};

    // Try to read in the file header
    try {
        rleFile.m_rleHeader = readHeader(stream);
    } catch (PackageException e) {
        throw PackageException(
            fmt::format("Failed to read RLE header: {}", e.what()));
    }

    // Seek to the beginning of the stream and read in everything
    stream.seekg(std::istream::beg);
    rleFile.m_rleData = std::vector<uint8_t>(rleFile.m_rleHeader.m_streamSize);
    streams::readBytes(stream, rleFile.m_rleData.data(),
                       rleFile.m_rleData.size());

    // RLE images contain a DDS image, so we set that up here based on
    // information from the RLE header, this is all stuff that has been
    // documented either in the DDS implementation or the imagecoder dealing
    // with DDS files
    uint8_t blockSize = 16;
    if (rleFile.m_rleHeader.m_fourCC == MAKE_FOURCC('D', 'X', 'T', '1') ||
        rleFile.m_rleHeader.m_fourCC == MAKE_FOURCC('D', 'S', 'T', '1')) {
        blockSize = 8;
    }

    dds::dds_pixelformat_t ddsPixelFormat{
        32,
    };

    if (rleFile.m_rleHeader.m_fourCC == fourcc_t::L8) {
        ddsPixelFormat.m_flags = dds::DDPF_LUMINANCE;
        ddsPixelFormat.m_rgbBitCount = 8;
        ddsPixelFormat.m_rBitMask = 0x000000FF;
        ddsPixelFormat.m_gBitMask = 0x000000FF;
        ddsPixelFormat.m_bBitMask = 0x000000FF;
        ddsPixelFormat.m_aBitMask = 0x0000000;
    } else {
        ddsPixelFormat.m_flags = dds::DDPF_FOURCC;
        ddsPixelFormat.m_rgbBitCount = 32;
        ddsPixelFormat.m_rBitMask = 0x00FF0000;
        ddsPixelFormat.m_gBitMask = 0x0000FF00;
        ddsPixelFormat.m_bBitMask = 0x000000FF;
        ddsPixelFormat.m_aBitMask = 0xFF000000;
        ddsPixelFormat.m_fourCC = fourcc_t::DXT5;
    }

    uint32_t pitchOrLinearSize = DDS_IMAGE_SIZE(
        rleFile.m_rleHeader.m_width, rleFile.m_rleHeader.m_height, blockSize);

    dds::dds_header_t ddsHeader{
        124,
        dds::DDSD_CAPS | dds::DDSD_HEIGHT | dds::DDSD_WIDTH |
            dds::DDSD_PIXELFORMAT | dds::DDSD_PITCH,
        rleFile.m_rleHeader.m_height,
        rleFile.m_rleHeader.m_width,
        pitchOrLinearSize,
        0,
        rleFile.m_rleHeader.m_mipCount,
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        ddsPixelFormat,
        dds::DDSCAPS_TEXTURE,
        0,
        0,
        0,
        0,
    };

    if (rleFile.m_rleHeader.m_mipCount > 1) {
        ddsHeader.m_flags |= dds::DDSD_MIPMAPCOUNT;
    }

    dds::dds_file_t ddsFile{
        ddsHeader,
    };

    uint32_t ddsImageDataSize = 0;
    {
        uint32_t width = rleFile.m_rleHeader.m_width;
        uint32_t height = rleFile.m_rleHeader.m_height;

        for (int i = 0; i < rleFile.m_rleHeader.m_mipCount; i++) {
            ddsImageDataSize += DDS_IMAGE_SIZE(width, height, blockSize);

            width /= 2;
            height /= 2;
        }
    }

    // Make room for the RLE-decoded blocks of the image, and do the actual
    // decoding
    std::vector<uint8_t> ddsImageData(ddsImageDataSize);

    uint32_t c = 0;

    if (rleFile.m_rleHeader.m_fourCC == fourcc_t::L8) {
        // TODO: Implement L8
    } else if (rleFile.m_rleHeader.m_rleVersion == rle_version_t::RLE2) {
        for (int i = 0; i < rleFile.m_rleHeader.m_mipCount; i++) {
            mip_header_t mipHeader = rleFile.m_rleHeader.m_mipHeaders[i];
            mip_header_t nextMipHeader =
                rleFile.m_rleHeader.m_mipHeaders[i + 1];

            int blockOffset2 = mipHeader.m_offset2;
            int blockOffset3 = mipHeader.m_offset3;
            int blockOffset0 = mipHeader.m_offset0;
            int blockOffset1 = mipHeader.m_offset1;

            for (int commandOffset = mipHeader.m_commandOffset;
                 commandOffset < nextMipHeader.m_commandOffset;
                 commandOffset += 2) {
                uint16_t command =
                    ((uint16_t)rleFile.m_rleData[commandOffset + 1] << 8 |
                     (uint16_t)rleFile.m_rleData[commandOffset + 0]);

                uint16_t operation = command & 3;
                uint16_t count = command >> 2;

                if (operation == 0) {
                    for (int j = 0; j < count; j++) {
                        COPY_BYTES(g_fullTransparentAlpha, ddsImageData, 0, 8,
                                   c);
                        COPY_BYTES(g_fullTransparentWhite, ddsImageData, 0, 8,
                                   c);
                    }
                } else if (operation == 1) {
                    for (int j = 0; j < count; j++) {
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset0, 2, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset1, 6, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset2, 4, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset3, 4, c);

                        blockOffset2 += 4;
                        blockOffset3 += 4;
                        blockOffset0 += 2;
                        blockOffset1 += 6;
                    }
                } else if (operation == 2) {
                    for (int j = 0; j < count; j++) {
                        COPY_BYTES(g_fullOpaqueAlpha, ddsImageData, 0, 8, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset2, 4, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset3, 4, c);

                        blockOffset2 += 4;
                        blockOffset3 += 4;
                    }
                } else {
                    throw PackageException(fmt::format(
                        "Unknown command {} while decoding RLE2 image!",
                        operation));
                }
            }

            if (blockOffset0 != nextMipHeader.m_offset0 ||
                blockOffset1 != nextMipHeader.m_offset1 ||
                blockOffset2 != nextMipHeader.m_offset2 ||
                blockOffset3 != nextMipHeader.m_offset3) {
                throw PackageException(
                    "Got out of sync while decoding RLE2 image!");
            }
        }
    } else {
        for (int i = 0; i < rleFile.m_rleHeader.m_mipCount; i++) {
            mip_header_t mipHeader = rleFile.m_rleHeader.m_mipHeaders[i];
            mip_header_t nextMipHeader =
                rleFile.m_rleHeader.m_mipHeaders[i + 1];

            int blockOffset2 = mipHeader.m_offset2;
            int blockOffset3 = mipHeader.m_offset3;
            int blockOffset0 = mipHeader.m_offset0;
            int blockOffset1 = mipHeader.m_offset1;
            int blockOffset4 = mipHeader.m_offset4;

            for (int commandOffset = mipHeader.m_commandOffset;
                 commandOffset < nextMipHeader.m_commandOffset;
                 commandOffset += 2) {
                uint16_t command =
                    ((uint16_t)rleFile.m_rleData[commandOffset + 1] << 8 |
                     (uint16_t)rleFile.m_rleData[commandOffset + 0]);

                uint16_t operation = command & 3;
                uint16_t count = command >> 2;

                if (operation == 0) {
                    for (int j = 0; j < count; j++) {
                        COPY_BYTES(g_fullTransparentAlpha, ddsImageData, 0, 8,
                                   c);
                        COPY_BYTES(g_fullTransparentBlack, ddsImageData, 0, 8,
                                   c);
                    }
                } else if (operation == 1) {
                    for (int j = 0; j < count; j++) {
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset0, 2, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset1, 6, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset2, 4, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset3, 4, c);

                        blockOffset2 += 4;
                        blockOffset3 += 4;
                        blockOffset0 += 2;
                        blockOffset1 += 6;

                        blockOffset4 += 16;
                    }
                } else if (operation == 2) {
                    for (int j = 0; j < count; j++) {
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset0, 2, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset1, 6, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset2, 4, c);
                        COPY_BYTES(rleFile.m_rleData, ddsImageData,
                                   blockOffset3, 4, c);

                        blockOffset2 += 4;
                        blockOffset3 += 4;
                        blockOffset0 += 2;
                        blockOffset1 += 6;
                    }
                } else {
                    throw PackageException(fmt::format(
                        "Unknown command {} while decoding RLES image!",
                        operation));
                }
            }

            if (blockOffset0 != nextMipHeader.m_offset0 ||
                blockOffset1 != nextMipHeader.m_offset1 ||
                blockOffset2 != nextMipHeader.m_offset2 ||
                blockOffset3 != nextMipHeader.m_offset3 ||
                blockOffset4 != nextMipHeader.m_offset4) {
                throw PackageException(
                    "Got out of sync while decoding RLES image!");
            }
        }
    }

    // This is yet again taken from the imagecoder, we take the continous stream
    // of blocks we decoded above, and split it up into a main surface and its
    // mipmaps

    uint32_t mipmapCopyIdx = 0;

    uint32_t mipmapPosition = DDS_IMAGE_SIZE(
        rleFile.m_rleHeader.m_width, rleFile.m_rleHeader.m_height, blockSize);

    std::vector<uint8_t> reconstructedMainImage(mipmapPosition);
    COPY_BYTES(ddsImageData, reconstructedMainImage, 0,
               reconstructedMainImage.size(), mipmapCopyIdx);
    mipmapCopyIdx = 0;

    std::vector<std::vector<uint8_t>> reconstructedMipmaps(
        std::max<int32_t>(0, ddsFile.m_header.m_mipMapCount - 1));

    {
        uint32_t width = rleFile.m_rleHeader.m_width;
        uint32_t height = rleFile.m_rleHeader.m_height;

        for (int i = 0; i < reconstructedMipmaps.size(); i++) {
            width /= 2;
            height /= 2;

            width = std::max<uint32_t>(1, width);
            height = std::max<uint32_t>(1, height);

            std::vector<uint8_t> mipmap(
                DDS_IMAGE_SIZE(width, height, blockSize));
            COPY_BYTES(ddsImageData, mipmap, mipmapPosition, mipmap.size(),
                       mipmapCopyIdx);

            mipmapCopyIdx = 0;
            mipmapPosition += (uint32_t)mipmap.size();

            reconstructedMipmaps[i] = mipmap;
        }
    }

    ddsFile.m_mainImage = reconstructedMainImage;
    ddsFile.m_mipmaps = reconstructedMipmaps;

    // Finally we store the DDS file we decompressed
    rleFile.m_ddsFile = ddsFile;
    return rleFile;
}
