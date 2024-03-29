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
#include <s4pkg/resources/iresource.h>

namespace s4pkg::resources {

/**
 * @brief Fallback resource for unimplemented types
 */
class S4PKG_EXPORT FallbackResource : public IResource {
   private:
    lib::ByteBuffer m_data{};

   public:
    FallbackResource(uint32_t type,
                     uint32_t instanceEx,
                     uint32_t instance,
                     uint32_t group,
                     const lib::ByteBuffer& data)
        : IResource(instanceEx, instance, group, (ResourceType)type),
          m_data(data) {}

    void setData(const lib::ByteBuffer& data) { this->m_data = data; }

    // IResource interface
   public:
    lib::ByteBuffer write() const override;

    lib::String getFriendlyName() const override { return "Unknown"; }

    // Object interface
   public:
    const lib::String toString() const override;
};

}  // namespace s4pkg::resources
