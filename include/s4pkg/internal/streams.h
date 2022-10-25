#pragma once

#include <s4pkg/internal/types.h>

#include <istream>

namespace s4pkg::internal::streams {

/**
 * @brief Reads size bytes from this stream into buffer
 * @param buffer: the array to read into
 * @param size: number of bytes to read
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readBytes(std::istream&, uint8_t* buffer, int size);

/**
 * @brief Reads a single byte from stream
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readUint8(std::istream&, uint8_t& value);

/**
 * @brief Reads 32-bit unsigned integer from stream
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readUint32(std::istream&, uint32_t& value);

/**
 * @brief Reads a 32-bit signed integer from stream
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readInt32(std::istream&, int32_t& value);

/**
 * @brief Reads a 64-bit unsigned integer from stream
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readUint64(std::istream&, uint64_t& value);

/**
 * @brief Reads a 16-bit unsigned interger from stream
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readUint16(std::istream&, uint16_t& value);

/**
 * @brief Reads [size] unsigned 32-bit integers from stream into buffer
 * @param buffer: the array to read into
 * @param size: the number of integers to read
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readUint32Array(std::istream&, uint32_t* buffer, int size);

/**
 * @brief Reads timestamp from stream. (1 signed 32-bit integer)
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readPackageTime(std::istream&, package_time_t& value);

/**
 * @brief Reads a version number from stream. (2 unsigned 32-bit integers)
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readPackageVersion(std::istream&, package_version_t& value);

/**
 * @brief Reads the complete package header from stream. The stream should be
 * positioned at the start of the file.
 * @param value: the struct to populate
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readPackageHeader(std::istream&, package_header_t& value);

/**
 * @brief Reads package flags (a bitfield) from stream. The stream should be
 * positioned at the index start position.
 * @param value: the struct (bitfield) to populate
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readPackageFlags(std::istream&, flags_t& value);

/**
 * @brief Reads a single index entry from the stream
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readIndexEntry(std::istream&, const flags_t&, index_entry_t& value);

/**
 * @brief Reads the complete package index from stream. The stream should be
 * positioned after the flags.
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readIndex(std::istream&,
               const flags_t&,
               uint32_t indexRecordCount,
               index_t& value);

/**
 * @brief Reads a single record from the stream. The stream is seeked by this
 * function.
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readRecord(std::istream&,
                const index_t&,
                uint32_t index,
                raw_record_t& value);

/**
 * @brief Reads all records of this package from the stream. The stream is
 * seeked by this function.
 * @param value: the variable to read into
 * @throws PackageException, if there aren't enough bytes left in the stream
 */
void readRecords(std::istream&, const index_t&, records_t& value);

}  // namespace s4pkg::internal::streams
