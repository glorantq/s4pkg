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

extern "C" {
namespace s4pkg {

class S4PKG_EXPORT PackageHeader : public Object {
   public:
    unsigned long m_indexRecordEntryCount;
    unsigned long m_indexRecordPositionLow;
    unsigned long m_indexRecordSize;
    unsigned long long m_indexRecordPosition;

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
    const lib::String toString() const override;
};

}  // namespace s4pkg
}
