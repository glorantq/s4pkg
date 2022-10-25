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
#include <s4pkg/package/ipackage.h>

#include <istream>

namespace s4pkg {

/**
 * @brief If m_package is nullptr, then m_errorMessage contains the reason why
 * loading failed.
 */
struct S4PKG_EXPORT PackageLoadResult {
    std::shared_ptr<IPackage> m_package;
    std::string m_errorMessage;
};

/**
 * @brief Loads a package from stream, and stores it in memory. It is safe to
 * close the stream after this method returns.
 * @param stream: the stream to read from
 * @return A struct with either the package object, or an error message
 */
S4PKG_EXPORT const PackageLoadResult loadPackage(std::istream& stream);

}  // namespace s4pkg
