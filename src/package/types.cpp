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

long PackageTime::toTimestamp() const {
    return (long)std::chrono::duration_cast<std::chrono::seconds>(
               this->m_timePoint.time_since_epoch())
        .count();
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
        case ResourceType::THUMBNAIL_IMAGE:
            resourceTypeName = "Thumbnail Image";
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

}  // namespace s4pkg
