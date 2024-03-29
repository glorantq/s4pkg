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

#include <string>

#include <s4pkg/internal/export.h>
#include <s4pkg/lib/string.h>

namespace s4pkg {

/**
 * @brief Base class for almost every object this library exports. Contains some
 * useful methods shared between them.
 */
class S4PKG_EXPORT Object {
   public:
    /**
     * @brief Returns a human-readable representation of this object
     * @return readable state of this object
     */
    virtual const lib::String toString() const = 0;

    bool operator==(const Object& obj) const { return equals(&obj); }

    virtual bool equals(const Object* other) const {
        return other != nullptr && this == other;
    }
};

}  // namespace s4pkg
