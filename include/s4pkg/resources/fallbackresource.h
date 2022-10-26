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

#include <s4pkg/resources/iresource.h>

namespace s4pkg::resources {

/**
 * @brief Fallback resource for unimplemented types
 */
class FallbackResource : public IResource {
   private:
    std::vector<uint8_t> m_data{};

    uint32_t m_type;
    uint32_t m_instanceEx;
    uint32_t m_instance;
    uint32_t m_group;

   public:
    FallbackResource(uint32_t type,
                     uint32_t instanceEx,
                     uint32_t instance,
                     uint32_t group,
                     std::vector<uint8_t> data)
        : m_type(type),
          m_instanceEx(instanceEx),
          m_instance(instance),
          m_group(group),
          m_data(std::move(data)) {}

    // IResource interface
   public:
    ResourceType getResourceType() const override {
        return (ResourceType)this->m_type;
    }
    uint32_t getInstanceEx() const override { return this->m_instanceEx; }
    uint32_t getInstance() const override { return this->m_instance; }
    uint32_t getGroup() const override { return this->m_group; }

    std::vector<uint8_t> write() const override;

    // Object interface
   public:
    const std::string toString() const override;
};

}  // namespace s4pkg::resources
