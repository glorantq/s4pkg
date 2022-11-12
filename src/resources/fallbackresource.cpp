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

#include <s4pkg/resources/fallbackresource.h>

#include <fmt/core.h>

namespace s4pkg::resources {

std::vector<uint8_t> FallbackResource::write() const {
    return this->m_data;
}

const std::string FallbackResource::toString() const {
    return fmt::format("FallbackResource [ size={}, type={:#x} ]",
                       this->m_data.size(), this->getResourceType());
}

}  // namespace s4pkg::resources
