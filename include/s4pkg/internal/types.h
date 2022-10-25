#pragma once

#include <inttypes.h>

typedef int32_t package_time_t;

typedef struct package_version_t {
    uint32_t m_major;
    uint32_t m_minor;
} package_version_t;
