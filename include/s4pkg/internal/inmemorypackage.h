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

#include <s4pkg/internal/types.h>
#include <s4pkg/package/ipackage.h>

#include <s4pkg/packageexception.h>

namespace s4pkg::internal {

/**
 * @brief A package implementation which reads in and stores the whole package
 * in memory after being constructed.
 */
class InMemoryPackage : public s4pkg::IPackage {
   private:
    package_header_t m_packageHeader{};
    flags_t m_flags{};

    uint32_t m_constantTypeId;
    uint32_t m_constantGroupId;
    uint32_t m_constantInstanceIdEx;

    index_t m_index{};
    records_t m_records{};

    bool m_valid = false;

   public:
    InMemoryPackage(std::istream&);

    // s4pkg::IPackage interface
   public:
    bool isValid() const override;

    const PackageVersion getFileVersion() const override;
    const PackageVersion getUserVersion() const override;
    const PackageTime getCreationTime() const override;
    const PackageTime getModifiedTime() const override;
    const PackageHeader getPackageHeader() const override;
    const PackageFlags getPackageFlags() const override;
    const std::vector<IndexEntry> getPackageIndex() const override;

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

}  // namespace s4pkg::internal
