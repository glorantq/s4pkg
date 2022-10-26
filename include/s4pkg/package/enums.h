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

namespace s4pkg {

/**
 * @brief Type of a resource
 * @see https://forums.thesims.com/en_US/discussion/858947/maxis-info-index
 */
enum ResourceType {
    UNKNOWN = 0,
    CAS_PART_RESOURCE_v42 = 0x034AEECB,
    STRING_TABLE_v5 = 0x220557da
};

/**
 * @brief CompressionType of a resource
 */
enum CompressionType {
    UNCOMPRESSED = 0x0000,
    STREAMABLE = 0xfffe,
    INTERNAL = 0xffff,
    DELETED = 0xffe0,
    ZLIB = 0x5a42
};

}  // namespace s4pkg
