#include <s4pkg/internal/inmemorypackage.h>

#include <s4pkg/internal/streams.h>
#include <s4pkg/packageexception.h>

#include <istream>

namespace s4pkg {

internal::InMemoryPackage::InMemoryPackage(std::istream& stream) {
    if (!stream.good()) {
        throw PackageException("stream.good() == false");
    }

    streams::readPackageHeader(stream, this->m_packageHeader);

    this->m_valid = true;
}

bool internal::InMemoryPackage::isValid() const {
    return this->m_valid;
}

const PackageVersion internal::InMemoryPackage::getFileVersion() const {
    return {this->m_packageHeader.m_fileVersion.m_major,
            this->m_packageHeader.m_fileVersion.m_minor};
}

const PackageVersion internal::InMemoryPackage::getUserVersion() const {
    return {this->m_packageHeader.m_userVersion.m_major,
            this->m_packageHeader.m_userVersion.m_minor};
}

const PackageTime internal::InMemoryPackage::getCreationTime() const {
    return {this->m_packageHeader.m_creationTime};
}

const PackageTime internal::InMemoryPackage::getModifiedTime() const {
    return {this->m_packageHeader.m_updatedTime};
}

const PackageHeader internal::InMemoryPackage::getPackageHeader() const {
    return {this->m_packageHeader.m_indexRecordEntryCount,
            this->m_packageHeader.m_indexRecordPositionLow,
            this->m_packageHeader.m_indexRecordSize,
            this->m_packageHeader.m_indexRecordPosition};
}

const std::string internal::InMemoryPackage::toString() const {
    return "[ not implemented ]";  // TODO: Implement
}

};  // namespace s4pkg
