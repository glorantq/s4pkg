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
#include <s4pkg/object.h>
#include <s4pkg/package/types.h>
#include <s4pkg/resources/iresource.h>

#include <ostream>
#include <vector>

namespace s4pkg {

/**
 * @brief The main interface for package files
 */
class S4PKG_EXPORT IPackage : public Object {
   public:
    virtual bool isValid() const = 0;

    // Methods for modification

    virtual bool deleteResource(const std::shared_ptr<const IResource>) = 0;

    // Getters

    virtual const PackageVersion getFileVersion() const = 0;
    virtual const PackageVersion getUserVersion() const = 0;

    virtual const PackageTime getCreationTime() const = 0;
    virtual const PackageTime getModifiedTime() const = 0;

    virtual const PackageHeader getPackageHeader() const = 0;
    virtual const PackageFlags getPackageFlags() const = 0;

    virtual const uint32_t getConstantGroup() const = 0;
    virtual const uint32_t getConstantType() const = 0;
    virtual const uint32_t getConstantInstanceEx() const = 0;

    virtual const std::vector<IndexEntry> getPackageIndex() const = 0;
    virtual const std::vector<std::shared_ptr<IResource>> getResources()
        const = 0;

    void write(std::ostream& stream, bool updateTime = false) const;
};

};  // namespace s4pkg
