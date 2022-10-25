#pragma once

#include <s4pkg/internal/types.h>

#include <istream>

namespace s4pkg::internal::streams {

void readBytes(std::istream&, uint8_t* buffer, int size);
void readUint8(std::istream&, uint8_t& value);
void readUint32(std::istream&, uint32_t& value);
void readInt32(std::istream&, int32_t& value);
void readUint64(std::istream&, uint64_t& value);
void readUint16(std::istream&, uint16_t& value);

void readUint32Array(std::istream&, uint32_t* buffer, int size);

void readPackageTime(std::istream&, package_time_t& value);
void readPackageVersion(std::istream&, package_version_t& value);

void readPackageHeader(std::istream&, package_header_t& value);

void readPackageFlags(std::istream&, flags_t& value);

void readIndexEntry(std::istream&, const flags_t&, index_entry_t& value);
void readIndex(std::istream&,
               const flags_t&,
               uint32_t indexRecordCount,
               index_t& value);
void readRecord(std::istream&,
                const index_t&,
                uint32_t index,
                raw_record_t& value);
void readRecords(std::istream&, const index_t&, records_t& value);

}  // namespace s4pkg::internal::streams
