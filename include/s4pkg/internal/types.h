#pragma once

#include <inttypes.h>
#include <memory>
#include <vector>

typedef int32_t package_time_t;

typedef struct package_version_t {
    uint32_t m_major;
    uint32_t m_minor;
} package_version_t;

// File header
// Unused or deprecated fields should be set to 0, unless marked otherwise
typedef struct package_header_t {
    uint8_t m_fileIdentifier[4];  // Should be [ 'D', 'B', 'P', 'F' ]
    package_version_t m_fileVersion;
    package_version_t m_userVersion;
    uint32_t m_unused1;
    package_time_t m_creationTime;  // Typically not set
    package_time_t m_updatedTime;   // Typically not set
    uint32_t m_unused2;
    uint32_t m_indexRecordEntryCount;
    uint32_t m_indexRecordPositionLow;
    uint32_t m_indexRecordSize;
    uint32_t m_unused3[3];
    uint32_t m_unused4;  // Always 3 for historical reasons
    uint64_t m_indexRecordPosition;
    uint32_t m_unused5[6];
} package_header_t;

typedef struct flags_t {
    uint32_t m_constantType : 1;
    uint32_t m_constantGroup : 1;
    uint32_t m_constantInstanceEx : 1;
    uint32_t m_reserved : 29;
} flags_t;

typedef struct index_entry_t {
    uint32_t m_type;
    uint32_t m_group;
    uint32_t m_instanceEx;

    uint32_t m_instance;
    uint32_t m_position;

    uint32_t m_size : 31;
    uint32_t m_extendedCompressionType : 1;
    uint32_t m_sizeDecompressed;

    uint16_t m_compressionType;
    uint16_t m_committed;  // Typically 1
} index_entry_t;

typedef struct index_t {
    std::vector<index_entry_t> m_entries;
} index_t;

typedef struct raw_record_t {
    uint32_t m_index;
    uint32_t m_size;
    std::shared_ptr<uint8_t> m_data;
} raw_record_t;

typedef struct records_t {
    std::vector<raw_record_t> m_records;
} records_t;
