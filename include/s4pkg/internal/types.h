#pragma once

#include <inttypes.h>

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
