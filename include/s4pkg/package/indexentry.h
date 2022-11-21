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

#pragma once

#include <s4pkg/internal/export.h>
#include <s4pkg/lib/string.h>
#include <s4pkg/object.h>
#include <s4pkg/package/enums.h>

extern "C" {
namespace s4pkg {

class S4PKG_EXPORT IndexEntry : public Object {
   public:
    ResourceType m_type;
    unsigned long m_group;
    unsigned long m_instanceEx;

    unsigned long m_instance;
    unsigned long m_position;
    unsigned long m_size;

    bool m_isExtendedCompressionType;
    unsigned long m_sizeDecompressed;

    CompressionType m_compressionType;
    unsigned int m_committed;

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
    const lib::String toString() const override;
};

}  // namespace s4pkg
}
