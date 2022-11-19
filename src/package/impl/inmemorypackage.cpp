/*
 * Copyright (c) 2022- Gerber Lóránt Viktor
 * Author: Gerber Lóránt Viktor <glorantv@student.elte.hu>
 *
 * This file is part of s4pkg.
 *
 * s4pkg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <s4pkg/internal/inmemorypackage.h>

#include <s4pkg/internal/globals.h>
#include <s4pkg/internal/streams.h>
#include <s4pkg/packageexception.h>
#include <s4pkg/resources/iresourcefactory.h>

#include <istream>

#include <fmt/core.h>
#include <fmt/printf.h>

namespace s4pkg {

internal::InMemoryPackage::InMemoryPackage(std::istream& stream) {
    if (!stream.good()) {
        throw PackageException("stream.good() == false");
    }

    try {
        streams::readPackageHeader(stream, this->m_packageHeader);
    } catch (PackageException e) {
        throw PackageException(fmt::format(
            "Exception while reading package header: {}", e.what()));
    }

    uint64_t indexPosition;
    if (this->m_packageHeader.m_indexRecordPosition != 0) {
        indexPosition = this->m_packageHeader.m_indexRecordPosition;
    } else {
        indexPosition = this->m_packageHeader.m_indexRecordPositionLow;
    }

    stream.seekg(indexPosition);

    try {
        streams::readPackageFlags(stream, this->m_flags);

        if (this->m_flags.m_constantType != 0) {
            streams::readUint32(stream, this->m_constantTypeId);
        }

        if (this->m_flags.m_constantGroup != 0) {
            streams::readUint32(stream, this->m_constantGroupId);
        }

        if (this->m_flags.m_constantInstanceEx != 0) {
            streams::readUint32(stream, this->m_constantInstanceIdEx);
        }
    } catch (PackageException e) {
        throw PackageException(
            fmt::format("Exception while reading package flags: {}", e.what()));
    }

    try {
        streams::readIndex(stream, this->m_flags,
                           this->m_packageHeader.m_indexRecordEntryCount,
                           this->m_index);
    } catch (PackageException e) {
        throw PackageException(
            fmt::format("Exception while reading package index: {}", e.what()));
    }

    try {
        streams::readRecords(stream, this->m_index, this->m_records);
    } catch (PackageException e) {
        throw PackageException(fmt::format(
            "Exception while reading package records: {}", e.what()));
    }

    for (const auto& record : this->m_records.m_records) {
        index_entry_t associatedEntry = this->m_index.m_entries[record.m_index];

        const IResourceFactory* resourceFactory =
            internal::globals::getResourceFactoryFor(
                (ResourceType)associatedEntry.m_type);

        if (resourceFactory != nullptr) {
            try {
                std::shared_ptr<IResource> parsedResource =
                    resourceFactory->create(
                        associatedEntry.m_type, associatedEntry.m_instanceEx,
                        associatedEntry.m_instance, associatedEntry.m_group,
                        record.m_data);

                if (parsedResource != nullptr) {
                    this->m_resources.push_back(parsedResource);
                } else {
                    throw PackageException(
                        fmt::format("Resource of type {:#x} returned no parsed "
                                    "implementation.",
                                    associatedEntry.m_type));
                }
            } catch (PackageException e) {
                throw PackageException(fmt::format(
                    "Exception while reading resource {:#x} ({}): {}",
                    associatedEntry.m_type, resourceFactory->toString(),
                    e.what()));
            }
        } else {
            throw PackageException(
                fmt::format("Unknown resource {:#x}, and for some reason no "
                            "fallback factory was returned.",
                            associatedEntry.m_type));
        }
    }

    this->m_valid = true;  // There should be better validation here, but
                           // for the time being, this is fine™
}

bool internal::InMemoryPackage::deleteResource(
    const std::shared_ptr<const IResource> resource) {
    if (!resource) {
        return false;
    }

    for (auto it = this->m_resources.begin(); it != this->m_resources.end();) {
        if (it->get() != nullptr && resource->equals(it->get())) {
            this->m_resources.erase(it);

            return true;
        }

        it++;
    }

    return false;
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

const PackageFlags internal::InMemoryPackage::getPackageFlags() const {
    return {this->m_flags.m_constantType != 0,
            this->m_flags.m_constantGroup != 0,
            this->m_flags.m_constantInstanceEx != 0};
}

const std::vector<IndexEntry> internal::InMemoryPackage::getPackageIndex()
    const {
    std::vector<IndexEntry> entries;

    for (const auto& index : this->m_index.m_entries) {
        IndexEntry entry = {(ResourceType)index.m_type,
                            index.m_group,
                            index.m_instanceEx,
                            index.m_instance,
                            index.m_position,
                            index.m_size,
                            index.m_extendedCompressionType != 0,
                            index.m_sizeDecompressed,
                            (CompressionType)index.m_compressionType,
                            index.m_committed};

        entries.push_back(entry);
    }

    return entries;
}

const std::vector<std::shared_ptr<IResource>>
internal::InMemoryPackage::getResources() const {
    return this->m_resources;
}

const std::string internal::InMemoryPackage::toString() const {
    return fmt::format(
        "(InMemoryPackage) [ header={}, fileVersion={}, userVersion={}, "
        "createdTime={}, modifiedTime={}, flags={} ]",
        this->getPackageHeader().toString(), this->getFileVersion().toString(),
        this->getUserVersion().toString(), this->getCreationTime().toString(),
        this->getModifiedTime().toString(), this->getPackageFlags().toString());
}

};  // namespace s4pkg
