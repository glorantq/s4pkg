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

#include <s4pkg/package/enums.h>
#include <s4pkg/resources/iresourcefactory.h>

#include <map>

namespace s4pkg::internal::globals {

extern const std::map<s4pkg::ResourceType,
                      std::shared_ptr<s4pkg::IResourceFactory>>
    g_resourceFactoryMapping;

extern const std::shared_ptr<s4pkg::IResourceFactory> getResourceFactoryFor(
    s4pkg::ResourceType);

};  // namespace s4pkg::internal::globals
