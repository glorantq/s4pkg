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

class S4PKG_EXPORT PackageFlags : public Object {
   public:
    bool m_isConstantType;
    bool m_isConstantGroup;
    bool m_isConstantInstance;

    PackageFlags(bool isConstantType,
                 bool isConstantGroup,
                 bool isConstantInstance)
        : m_isConstantType(isConstantType),
          m_isConstantGroup(isConstantGroup),
          m_isConstantInstance(isConstantInstance) {}

    // s4pkg::Object interface
   public:
    const lib::String toString() const override;
};

}  // namespace s4pkg
}
