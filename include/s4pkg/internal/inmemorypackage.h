#pragma once

#include <s4pkg/internal/types.h>
#include <s4pkg/package/ipackage.h>

#include <s4pkg/packageexception.h>

namespace s4pkg::internal {

class InMemoryPackage : public s4pkg::IPackage {
   private:
    // File header
    // Unused or deprecated fields should be set to 0, unless marked otherwise

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

    bool m_valid = false;

   public:
    InMemoryPackage(std::istream&);

    // s4pkg::IPackage interface
   public:
    bool isValid() const override;

    const PackageVersion getFileVersion() const override;
    const PackageVersion getUserVersion() const override;
    const PackageTime getCreationTime() const override;
    const PackageTime getModifiedTime() const override;
    const PackageHeader getPackageHeader() const override;

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

}  // namespace s4pkg::internal
