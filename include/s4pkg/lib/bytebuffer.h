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

#include <cinttypes>
#include <cstring>

extern "C" {
namespace s4pkg::lib {

class S4PKG_EXPORT ByteBuffer {
   private:
    uint8_t* m_buffer;
    uint64_t m_length;

   public:
    ByteBuffer(uint8_t* buffer, uint64_t length) {
        m_buffer = new uint8_t[length];
        memcpy(m_buffer, buffer, length);
        m_length = length;
    }

    ByteBuffer(uint64_t length) {
        m_buffer = new uint8_t[length];
        m_length = length;
    }

    ByteBuffer() : ByteBuffer(0) {}

    ByteBuffer(const ByteBuffer& other)
        : ByteBuffer(other.m_buffer, other.m_length) {}

    ByteBuffer& operator=(const ByteBuffer& other) {
        delete[] m_buffer;

        m_buffer = new uint8_t[other.m_length];
        memcpy(m_buffer, other.m_buffer, other.m_length);

        m_length = other.m_length;

        return *this;
    }

    ByteBuffer& operator=(ByteBuffer&& other) {
        if (this == &other) {
            return *this;
        }

        delete[] m_buffer;
        m_buffer = other.m_buffer;
        m_length = other.m_length;

        other.m_buffer = nullptr;
        other.m_length = 0;

        return *this;
    }

    uint8_t& operator[](uint64_t pos) const { return m_buffer[pos]; }

    ~ByteBuffer() { delete[] m_buffer; }

    uint64_t size() const { return m_length; }
    uint8_t* data() const { return m_buffer; }
};

};  // namespace s4pkg::lib
}
