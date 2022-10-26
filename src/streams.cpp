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

#include <s4pkg/internal/streams.h>

#include <s4pkg/packageexception.h>

#include <fmt/core.h>
#include <fmt/printf.h>

#include <miniz.h>

namespace s4pkg::internal::streams {

void readBytes(std::istream& stream, uint8_t* buffer, int size) {
    for (int i = 0; i < size; i++) {
        stream.read((char*)(buffer + i), 1);

        if (!stream.good()) {
            throw PackageException(fmt::format(
                "Unexpected end of stream! Tried reading {} bytes.", size));
        }
    }
}

void readUint8(std::istream& stream, uint8_t& value) {
    uint8_t temp;
    stream.read((char*)&temp, 1);

    if (!stream.good()) {
        throw PackageException(
            "Unexpected end of stream! Tried reading 1 uint8_t.");
    }

    value = temp;
}

void readUint32(std::istream& stream, uint32_t& value) {
    uint8_t buffer[4];
    readBytes(stream, buffer, 4);

    value = ((uint32_t)buffer[3] << 24 | (uint32_t)buffer[2] << 16 |
             (uint32_t)buffer[1] << 8 | (uint32_t)buffer[0]);
}

void readInt32(std::istream& stream, int32_t& value) {
    uint32_t temp;
    readUint32(stream, temp);

    value = (int32_t)temp;
}

void readUint64(std::istream& stream, uint64_t& value) {
    uint8_t buffer[8];
    readBytes(stream, buffer, 8);
    value = ((uint64_t)buffer[7] << 56 | (uint64_t)buffer[6] << 48 |
             (uint64_t)buffer[5] << 40 | (uint64_t)buffer[4] << 32 |
             (uint64_t)buffer[3] << 24 | (uint64_t)buffer[2] << 16 |
             (uint64_t)buffer[1] << 8 | (uint64_t)buffer[0]);
}

void readUint16(std::istream& stream, uint16_t& value) {
    uint8_t buffer[2];
    readBytes(stream, buffer, 2);

    value = ((uint16_t)buffer[1] << 8 | (uint16_t)buffer[0]);
}

void readUint32Array(std::istream& stream, uint32_t* buffer, int size) {
    for (int i = 0; i < size; i++) {
        readUint32(stream, buffer[i]);
    }
}

void readPackageTime(std::istream& stream, package_time_t& value) {
    readInt32(stream, value);
}

void readPackageVersion(std::istream& stream, package_version_t& value) {
    readUint32(stream, value.m_major);
    readUint32(stream, value.m_minor);
}

void readPackageHeader(std::istream& stream, package_header_t& value) {
    readBytes(stream, value.m_fileIdentifier, 4);

    uint8_t expectedIdentifier[] = {'D', 'B', 'P', 'F'};

    for (int i = 0; i < 4; i++) {
        if (value.m_fileIdentifier[i] != expectedIdentifier[i]) {
            throw PackageException("Invalid file identifier!");
        }
    }

    readPackageVersion(stream, value.m_fileVersion);
    readPackageVersion(stream, value.m_userVersion);

    readUint32(stream, value.m_unused1);

    readPackageTime(stream, value.m_creationTime);
    readPackageTime(stream, value.m_updatedTime);

    readUint32(stream, value.m_unused2);

    readUint32(stream, value.m_indexRecordEntryCount);
    readUint32(stream, value.m_indexRecordPositionLow);
    readUint32(stream, value.m_indexRecordSize);

    readUint32Array(stream, value.m_unused3, 3);
    readUint32(stream, value.m_unused4);

    readUint64(stream, value.m_indexRecordPosition);

    readUint32Array(stream, value.m_unused5, 6);
}

void readPackageFlags(std::istream& stream, flags_t& value) {
    uint32_t bitField;

    readUint32(stream, bitField);

    uint32_t constantType = bitField & 1;
    uint32_t constantGroup = bitField >> 1 & 1;
    uint32_t constantInstanceEx = bitField >> 2 & 1;
    uint32_t reserved = bitField >> 29;

    value.m_constantGroup = constantGroup;
    value.m_constantType = constantType;
    value.m_constantInstanceEx = constantInstanceEx;
    value.m_reserved = reserved;
}

void readIndexEntry(std::istream& stream,
                    const flags_t& flags,
                    index_entry_t& value) {
    if (flags.m_constantType == 0) {
        readUint32(stream, value.m_type);
    }

    if (flags.m_constantGroup == 0) {
        readUint32(stream, value.m_group);
    }

    if (flags.m_constantInstanceEx == 0) {
        readUint32(stream, value.m_instanceEx);
    }

    readUint32(stream, value.m_instance);
    readUint32(stream, value.m_position);

    uint32_t sizeCompressionBitField;
    readUint32(stream, sizeCompressionBitField);

    value.m_size = (sizeCompressionBitField << 1) >> 1;
    value.m_extendedCompressionType = sizeCompressionBitField >> 31;

    readUint32(stream, value.m_sizeDecompressed);

    if (value.m_extendedCompressionType > 0) {
        uint16_t compressionType;
        readUint16(stream, compressionType);
        value.m_compressionType = (compression_type_t)compressionType;

        readUint16(stream, value.m_committed);
    }
}

void readIndex(std::istream& stream,
               const flags_t& flags,
               uint32_t indexRecordCount,
               index_t& value) {
    for (int i = 0; i < indexRecordCount; i++) {
        index_entry_t indexEntry{};

        readIndexEntry(stream, flags, indexEntry);
        value.m_entries.push_back(indexEntry);
    }
}

void readRecord(std::istream& stream,
                const index_t& packageIndex,
                uint32_t index,
                raw_record_t& value) {
    if (index >= packageIndex.m_entries.size()) {
        throw PackageException("index >= packageIndex.m_entries.size()");
    }

    if (index < 0) {
        throw PackageException("index < 0");
    }

    index_entry_t indexEntry = packageIndex.m_entries[index];

    value.m_index = index;
    value.m_size = indexEntry.m_size;

    stream.seekg(indexEntry.m_position);
    if (indexEntry.m_size > 0) {
        std::vector<uint8_t> compressedBuffer(indexEntry.m_size);

        uint8_t byte;
        for (int i = 0; i < indexEntry.m_size; i++) {
            readUint8(stream, byte);
            compressedBuffer[i] = byte;
        }

        if (indexEntry.m_compressionType == compression_type_t::DELETED) {
            throw PackageException("Unimplemented compression type: DELETED");
        } else if (indexEntry.m_compressionType ==
                   compression_type_t::INTERNAL) {
            throw PackageException("Unimplemented compression type: INTERNAL");
        } else if (indexEntry.m_compressionType ==
                   compression_type_t::STREAMABLE) {
            throw PackageException(
                "Unimplemented compression type: STREAMABLE");
        } else if (indexEntry.m_compressionType == compression_type_t::ZLIB) {
            mz_stream zInflateStream;
            zInflateStream.zalloc = nullptr;
            zInflateStream.zfree = nullptr;
            zInflateStream.opaque = nullptr;

            zInflateStream.avail_in = (unsigned int)indexEntry.m_size;
            zInflateStream.next_in = compressedBuffer.data();

            std::vector<uint8_t> buffer(indexEntry.m_sizeDecompressed);

            zInflateStream.avail_out =
                (unsigned int)indexEntry.m_sizeDecompressed;
            zInflateStream.next_out = buffer.data();

            mz_inflateInit(&zInflateStream);

            int inflateResult = mz_inflate(&zInflateStream, MZ_NO_FLUSH);
            if (inflateResult != MZ_OK && inflateResult != MZ_STREAM_END) {
                std::string errorName;
                switch (inflateResult) {
                    case MZ_OK:
                        errorName = "MZ_OK";
                        break;

                    case MZ_STREAM_END:
                        errorName = "MZ_STREAM_END";
                        break;

                    case MZ_STREAM_ERROR:
                        errorName = "MZ_STREAM_ERROR";
                        break;

                    case MZ_DATA_ERROR:
                        errorName = "MZ_DATA_ERROR";
                        break;

                    case MZ_PARAM_ERROR:
                        errorName = "MZ_PARAM_ERROR";
                        break;

                    case MZ_BUF_ERROR:
                        errorName = "MZ_BUF_ERROR";
                        break;

                    default:
                        errorName = "?";
                }

                throw PackageException(fmt::format(
                    "Failed to decompress resource {}, result is {}",
                    indexEntry.m_instance, errorName));
            }
            mz_inflateEnd(&zInflateStream);

            value.m_data = buffer;
        } else {
            value.m_data = compressedBuffer;
        }
    }
}

void readRecords(std::istream& stream, const index_t& index, records_t& value) {
    for (uint32_t i = 0; i < index.m_entries.size(); i++) {
        raw_record_t record{};
        readRecord(stream, index, i, record);

        value.m_records.push_back(record);
    }
}

}  // namespace s4pkg::internal::streams
