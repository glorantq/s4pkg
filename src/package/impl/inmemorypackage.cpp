#include <s4pkg/internal/inmemorypackage.h>

namespace s4pkg {

bool internal::InMemoryPackage::isValid() const {
    return false;
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

const std::string internal::InMemoryPackage::toString() const {
    return "";  // TODO: Implement
}

};  // namespace s4pkg
