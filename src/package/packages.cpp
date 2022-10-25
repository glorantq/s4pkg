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

#include <s4pkg/package/packages.h>

#include <s4pkg/internal/inmemorypackage.h>
#include <s4pkg/packageexception.h>

#include <tuple>

namespace s4pkg {

S4PKG_EXPORT const PackageLoadResult loadPackage(std::istream& stream) {
    try {
        return {std::make_shared<internal::InMemoryPackage>(
                    internal::InMemoryPackage(stream)),
                ""};
    } catch (PackageException e) {
        return {nullptr, e.what()};
    }

    return {nullptr, ""};
}

}  // namespace s4pkg
