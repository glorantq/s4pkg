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

#include <chrono>

namespace s4pkg {

class S4PKG_EXPORT PackageTime : public Object {
   public:
    std::chrono::time_point<std::chrono::system_clock> m_timePoint;

    PackageTime(long long unixTime)
        : m_timePoint(std::chrono::time_point<std::chrono::system_clock>() +
                      std::chrono::seconds(unixTime)){};

    PackageTime(std::chrono::time_point<std::chrono::system_clock> timePoint)
        : m_timePoint(timePoint){};

    long toTimestamp() const;

    // s4pkg::Object interface
   public:
    const std::string toString() const override;
};

}  // namespace s4pkg
