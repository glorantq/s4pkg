#include <s4pkg/internal/inmemorypackage.h>

#include <s4pkg/internal/streams.h>
#include <s4pkg/packageexception.h>

#include <istream>

namespace s4pkg {

internal::InMemoryPackage::InMemoryPackage(std::istream& stream) {
    if (!stream.good()) {
        throw PackageException("stream.good() == false");
    }

    streams::readBytes(stream, this->m_fileIdentifier, 4);

    uint8_t expectedIdentifier[] = {'D', 'B', 'P', 'F'};

    for (int i = 0; i < 4; i++) {
        if (this->m_fileIdentifier[i] != expectedIdentifier[i]) {
            throw PackageException("Invalid file identifier");
        }
    }

    streams::readPackageVersion(stream, this->m_fileVersion);
    streams::readPackageVersion(stream, this->m_userVersion);

    streams::readUint32(stream, this->m_unused1);

    streams::readPackageTime(stream, this->m_creationTime);
    streams::readPackageTime(stream, this->m_updatedTime);

    streams::readUint32(stream, this->m_unused2);

    streams::readUint32(stream, this->m_indexRecordEntryCount);
    streams::readUint32(stream, this->m_indexRecordPositionLow);
    streams::readUint32(stream, this->m_indexRecordSize);

    streams::readUint32Array(stream, this->m_unused3, 3);
    streams::readUint32(stream, this->m_unused4);

    streams::readUint64(stream, this->m_indexRecordPosition);

    streams::readUint32Array(stream, this->m_unused5, 6);

    this->m_valid = true;
}

bool internal::InMemoryPackage::isValid() const {
    return this->m_valid;
}

const PackageVersion internal::InMemoryPackage::getFileVersion() const {
    return {this->m_fileVersion.m_major, this->m_fileVersion.m_minor};
}

const PackageVersion internal::InMemoryPackage::getUserVersion() const {
    return {this->m_userVersion.m_major, this->m_userVersion.m_minor};
}

const PackageTime internal::InMemoryPackage::getCreationTime() const {
    return {this->m_creationTime};
}

const PackageTime internal::InMemoryPackage::getModifiedTime() const {
    return {this->m_updatedTime};
}

const PackageHeader internal::InMemoryPackage::getPackageHeader() const {
    return {this->m_indexRecordEntryCount, this->m_indexRecordPositionLow,
            this->m_indexRecordSize, this->m_indexRecordPosition};
}

const std::string internal::InMemoryPackage::toString() const {
    return "[ not implemented ]";  // TODO: Implement
}

};  // namespace s4pkg
