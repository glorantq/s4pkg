#pragma once

#include <s4pkg/internal/export.h>
#include <s4pkg/object.h>

#include <chrono>
#include <vector>

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

class S4PKG_EXPORT PackageFlags : public Object {
   public:
    const bool m_isConstantType;
    const bool m_isConstantGroup;
    const bool m_isConstantInstance;

    PackageFlags(bool isConstantType,
                 bool isConstantGroup,
                 bool isConstantInstance)
        : m_isConstantType(isConstantType),
          m_isConstantGroup(isConstantGroup),
          m_isConstantInstance(isConstantInstance) {}

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

/**
 * @brief Type of a resource
 * @see https://forums.thesims.com/en_US/discussion/858947/maxis-info-index
 */
enum ResourceType { UNKNOWN = 0, CAS_PART_RESOURCE_v42 = 0x034AEECB };

/**
 * @brief CompressionType of a resource
 */
enum CompressionType {
    UNCOMPRESSED = 0x0000,
    STREAMABLE = 0xfffe,
    INTERNAL = 0xffff,
    DELETED = 0xffe0,
    ZLIB = 0x5a42
};

class S4PKG_EXPORT IndexEntry : public Object {
   public:
    const ResourceType m_type;
    const unsigned long m_group;
    const unsigned long m_instanceEx;

    const unsigned long m_instance;
    const unsigned long m_position;
    const unsigned long m_size;

    const bool m_isExtendedCompressionType;
    const unsigned long m_sizeDecompressed;

    const CompressionType m_compressionType;
    const unsigned int m_committed;

    IndexEntry(ResourceType resourceType,
               unsigned long group,
               unsigned long instanceEx,
               unsigned long instance,
               unsigned long position,
               unsigned long size,
               bool isExtendedCompressionType,
               unsigned long sizeDecompressed,
               CompressionType compressionType,
               unsigned int committed)
        : m_type(resourceType),
          m_group(group),
          m_instanceEx(instanceEx),
          m_instance(instance),
          m_position(position),
          m_size(size),
          m_isExtendedCompressionType(isExtendedCompressionType),
          m_sizeDecompressed(sizeDecompressed),
          m_compressionType(compressionType),
          m_committed(committed) {}

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

/**
 * @brief The main interface for package files
 */
class S4PKG_EXPORT IPackage : public Object {
   public:
    virtual bool isValid() const = 0;

    // Getters

    virtual const PackageVersion getFileVersion() const = 0;
    virtual const PackageVersion getUserVersion() const = 0;

    virtual const PackageTime getCreationTime() const = 0;
    virtual const PackageTime getModifiedTime() const = 0;

    virtual const PackageHeader getPackageHeader() const = 0;
    virtual const PackageFlags getPackageFlags() const = 0;

    virtual const std::vector<IndexEntry> getPackageIndex() const = 0;
};

};  // namespace s4pkg
