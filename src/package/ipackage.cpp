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

#include <s4pkg/internal/streams.h>
#include <s4pkg/package/ipackage.h>

#include <fmt/printf.h>

#include <chrono>

namespace s4pkg {

void IPackage::write(std::ostream& stream, bool updateTime) const {
    // Construct flags structure
    PackageFlags flags = this->getPackageFlags();

    flags_t packageFlags{
        flags.m_isConstantType, flags.m_isConstantGroup,
        flags.m_isConstantInstance,
        0  // m_reserved
    };

    // Construct a new index for this package

    index_t packageIndex{};

    // Create an index entry for every resource

    const std::vector<std::shared_ptr<IResource>> resources =
        this->getResources();

    for (int i = 0; i < resources.size(); i++) {
        std::shared_ptr<IResource> resource = resources[i];

        index_entry_t indexEntry{
            (uint32_t)resource->getResourceType(),
            resource->getGroup(),
            resource->getInstanceEx(),
            resource->getInstance(),
            0,  // m_position (set by the write method)
            0,  // m_size (set by the write method)
            1,  // m_extendedCompressionType
            0,  // m_sizeDecompressed (set by the write method)
            compression_type_t::UNCOMPRESSED,
            1  // m_committed
        };

        packageIndex.m_entries.push_back(indexEntry);
    }

    // Create a record table

    records_t recordTable{};

    // Create raw records for each resource

    for (int i = 0; i < resources.size(); i++) {
        std::shared_ptr<IResource> resource = resources[i];
        lib::ByteBuffer resourceData = resource->write();

        raw_record_t record{(uint32_t)i, (uint32_t)resourceData.size(),
                            resourceData};

        recordTable.m_records.push_back(record);
    }

    // 0-th step: make room for writing the header later
    stream.seekp(sizeof(package_header_t));

    // First we write out the resource blobs, the method in streams takes in the
    // index we have and updates the corresponding entries for size,
    // decompressed size, and position

    internal::streams::writeRecords(stream, packageIndex, recordTable);

    // We now save the current position in the stream, to later reference the
    // start of the index in the header

    uint32_t indexPosition = (uint32_t)stream.tellp();

    // Now we write out the package flags

    internal::streams::writePackageFlags(stream, packageFlags);

    // If we have a constant group, type, or instance(ex), write it here

    if (flags.m_isConstantType) {
        internal::streams::writeUint32(stream, this->getConstantType());
    }

    if (flags.m_isConstantGroup) {
        internal::streams::writeUint32(stream, this->getConstantGroup());
    }

    if (flags.m_isConstantInstance) {
        internal::streams::writeUint32(stream, this->getConstantInstanceEx());
    }

    // And the index

    internal::streams::writeIndex(stream, packageFlags, packageIndex);

    uint32_t indexEnd = (uint32_t)stream.tellp();

    // Now we create a header for the file

    package_version_t fileVersion{this->getFileVersion().m_majorVersion,
                                  this->getFileVersion().m_minorVersion};

    package_version_t userVersion{this->getUserVersion().m_majorVersion,
                                  this->getUserVersion().m_minorVersion};

    package_time_t createdTime = this->getCreationTime();
    package_time_t updatedTime =
        updateTime ? (long)std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count()
                   : this->getModifiedTime();

    package_header_t packageHeader{
        {'D', 'B', 'P', 'F'},  // file identifier
        fileVersion,
        userVersion,
        0,  // m_unused1
        createdTime,
        updatedTime,
        0,  // m_unused1
        (uint32_t)packageIndex.m_entries.size(),
        0,                         // m_indexRecordPositionLow
        indexEnd - indexPosition,  // size of the whole index (including flags,
                                   // constant values and the index itself)
        {0, 0, 0},                 // m_unused3
        3,                         // m_unused4
        indexPosition,
        {0, 0, 0, 0, 0, 0}  // m_unused5
    };

    // And write the header at the start of the file

    stream.seekp(0);
    internal::streams::writePackageHeader(stream, packageHeader);
}

};  // namespace s4pkg
