#pragma once

#include <s4pkg/internal/types.h>

#include <istream>

namespace s4pkg::internal::streams {

void readBytes(std::istream&, uint8_t* buffer, int size);
void readUint32(std::istream&, uint32_t& value);
void readInt32(std::istream&, int32_t& value);
void readUint64(std::istream&, uint64_t& value);

void readUint32Array(std::istream&, uint32_t* buffer, int size);

void readPackageTime(std::istream&, package_time_t& value);
void readPackageVersion(std::istream&, package_version_t& value);

void readPackageHeader(std::istream&, package_header_t& value);

}  // namespace s4pkg::internal::streams
