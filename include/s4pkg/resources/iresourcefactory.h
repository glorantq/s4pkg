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

#include <s4pkg/object.h>
#include <s4pkg/resources/iresource.h>

#include <memory>
#include <vector>

namespace s4pkg {

class IResourceFactory : public Object {
   public:
    virtual std::shared_ptr<IResource> create(
        uint32_t type, /**< This is purely for fallback */
        uint32_t instanceEx,
        uint32_t instance,
        uint32_t group,
        const std::vector<uint8_t>&) const = 0;
};

}  // namespace s4pkg
