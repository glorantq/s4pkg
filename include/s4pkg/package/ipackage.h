#pragma once

#include <s4pkg/internal/export.h>
#include <s4pkg/object.h>

namespace s4pkg {

class S4PKG_EXPORT PackageVersion : public Object {
   public:
    const unsigned int m_majorVersion;
    const unsigned int m_minorVersion;

    PackageVersion(unsigned int majorVersion, unsigned int minorVersion)
        : m_majorVersion(majorVersion), m_minorVersion(minorVersion){};

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

class S4PKG_EXPORT PackageTime : public Object {
   public:
    const long long m_unixTime;

    PackageTime(long long unixTime) : m_unixTime(unixTime){};

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

class S4PKG_EXPORT IPackage : public Object {
   public:
    virtual bool isValid() const = 0;

    // Getters

    virtual const PackageVersion getFileVersion() const = 0;
    virtual const PackageVersion getUserVersion() const = 0;

    virtual const PackageTime getCreationTime() const = 0;
    virtual const PackageTime getModifiedTime() const = 0;
};

};  // namespace s4pkg
