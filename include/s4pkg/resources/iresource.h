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
#include <s4pkg/package/enums.h>

#include <vector>

namespace s4pkg {

class S4PKG_EXPORT IResource : public Object {
   protected:
    uint32_t m_instanceEx;
    uint32_t m_instance;
    uint32_t m_group;
    ResourceType m_resourceType;

    IResource(uint32_t instanceEx,
              uint32_t instance,
              uint32_t group,
              ResourceType resourceType)
        : m_instanceEx(instanceEx),
          m_instance(instance),
          m_group(group),
          m_resourceType(resourceType) {}

   public:
    virtual uint32_t getInstanceEx() const { return this->m_instanceEx; };
    virtual uint32_t getInstance() const { return this->m_instance; }
    virtual uint32_t getGroup() const { return this->m_group; }
    virtual ResourceType getResourceType() const {
        return this->m_resourceType;
    }
    virtual std::vector<uint8_t> write() const = 0;

    virtual std::string getFriendlyName() const = 0;
};

}  // namespace s4pkg
