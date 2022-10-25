#pragma once

#include <s4pkg/internal/export.h>
#include <s4pkg/object.h>

#include <chrono>

namespace s4pkg {

class S4PKG_EXPORT PackageVersion : public Object {
   public:
    const unsigned long m_majorVersion;
    const unsigned long m_minorVersion;

    PackageVersion(unsigned int majorVersion, unsigned int minorVersion)
        : m_majorVersion(majorVersion), m_minorVersion(minorVersion){};

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

class S4PKG_EXPORT PackageTime : public Object {
   public:
    const std::chrono::time_point<std::chrono::system_clock> m_timePoint;

    PackageTime(long long unixTime)
        : m_timePoint(std::chrono::time_point<std::chrono::system_clock>() +
                      std::chrono::seconds(unixTime)){};

    PackageTime(std::chrono::time_point<std::chrono::system_clock> timePoint)
        : m_timePoint(timePoint){};

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

class S4PKG_EXPORT PackageHeader : public Object {
   public:
    const unsigned long m_indexRecordEntryCount;
    const unsigned long m_indexRecordPositionLow;
    const unsigned long m_indexRecordSize;
    const unsigned long long m_indexRecordPosition;

    PackageHeader(unsigned long indexRecordEntryCount,
                  unsigned long indexRecordPositionLow,
                  unsigned long indexRecordSize,
                  unsigned long long indexRecordPosition)
        : m_indexRecordEntryCount(indexRecordEntryCount),
          m_indexRecordPositionLow(indexRecordPositionLow),
          m_indexRecordSize(indexRecordSize),
          m_indexRecordPosition(indexRecordPosition) {}

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

    virtual const PackageHeader getPackageHeader() const = 0;
};

};  // namespace s4pkg
