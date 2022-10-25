#include <s4pkg/package/ipackage.h>

#include <fmt/chrono.h>
#include <fmt/core.h>

namespace s4pkg {

const std::string PackageVersion::toString() const {
    return fmt::format("{}.{}", this->m_majorVersion, this->m_minorVersion);
}

const std::string PackageTime::toString() const {
    return fmt::format("{:%Y-%m-%d %H:%M:%S}", this->m_timePoint);
}

const std::string PackageHeader::toString() const {
    return fmt::format(
        "[ recordEntryCount={}, recordPositionLow={}, recordPosition={}, "
        "recordSize={} ]",
        this->m_indexRecordEntryCount, this->m_indexRecordPositionLow,
        this->m_indexRecordPosition, this->m_indexRecordSize);
}

const std::string PackageFlags::toString() const {
    return fmt::format(
        "[ isConstantType={}, isConstantGroup={}, isConstantInstance={} ]",
        this->m_isConstantType, this->m_isConstantGroup,
        this->m_isConstantInstance);
}

const std::string IndexEntry::toString() const {
    std::string resourceTypeName;
    switch (this->m_type) {
        case ResourceType::CAS_PART_RESOURCE_v42:
            resourceTypeName = "CAS Part Resource v42";
            break;

        case ResourceType::UNKNOWN:
        default:
            resourceTypeName = "UNKNOWN";
    }

    std::string compressionTypeName;
    switch (this->m_compressionType) {
        case CompressionType::UNCOMPRESSED:
            compressionTypeName = "UNCOMPRESSED";
            break;

        case CompressionType::STREAMABLE:
            compressionTypeName = "STREAMABLE";
            break;

        case CompressionType::INTERNAL:
            compressionTypeName = "INTERNAL";
            break;

        case CompressionType::DELETED:
            compressionTypeName = "DELETED";
            break;

        case CompressionType::ZLIB:
            compressionTypeName = "ZLIB";
            break;

        default:
            compressionTypeName = "UNKNOWN";
    }

    return fmt::format(
        "[ type={}, group={}, instanceEx={}, instance={}, position={}, "
        "size={}, isExtendedCompressionType={}, sizeDecompressed={}, "
        "compressionType={}, committed={} ]",
        resourceTypeName, this->m_group, this->m_instanceEx, this->m_instance,
        this->m_position, this->m_size, this->m_isExtendedCompressionType,
        this->m_sizeDecompressed, compressionTypeName, this->m_committed);
}

};  // namespace s4pkg
