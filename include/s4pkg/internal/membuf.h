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

#include <inttypes.h>
#include <streambuf>

namespace s4pkg::internal {

/**
 * @brief Utility to make an std::istream from a memory buffer (for usage with
 * internal::streams functions)
 */
struct membuf : std::streambuf {
   private:
    const uint8_t* m_begin;
    uint64_t m_size;

   public:
    membuf(const uint8_t* begin, uint64_t size) : m_begin(begin), m_size(size) {
        this->setg((char*)begin, (char*)begin, (char*)begin + m_size);
    }

    virtual pos_type seekoff(
        off_type off,
        std::ios_base::seekdir dir,
        std::ios_base::openmode which = std::ios_base::in) override {
        if (dir == std::ios_base::cur) {
            gbump((int)off);
        } else if (dir == std::ios_base::end) {
            setg((char*)m_begin, (char*)m_begin + m_size + off,
                 (char*)m_begin + m_size);
        } else if (dir == std::ios_base::beg) {
            setg((char*)m_begin, (char*)m_begin + off, (char*)m_begin + m_size);
        }

        return gptr() - eback();
    }

    virtual pos_type seekpos(std::streampos pos,
                             std::ios_base::openmode mode) override {
        return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
    }
};

}  // namespace s4pkg::internal
