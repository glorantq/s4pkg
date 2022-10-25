#pragma once

#include <s4pkg/internal/types.h>
#include <s4pkg/package/ipackage.h>

#include <s4pkg/packageexception.h>

namespace s4pkg::internal {

class InMemoryPackage : public s4pkg::IPackage {
   private:
    package_header_t m_packageHeader;
    flags_t m_flags;

    uint32_t m_constantTypeId;
    uint32_t m_constantGroupId;
    uint32_t m_constantInstanceIdEx;

    index_t m_index;
    records_t m_records;

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
    const PackageFlags getPackageFlags() const override;
    const std::vector<IndexEntry> getPackageIndex() const override;

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

}  // namespace s4pkg::internal
