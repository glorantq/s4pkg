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
#include <string>

extern "C" {
namespace s4pkg::lib {

class S4PKG_EXPORT String {
   private:
    char* m_data;
    uint64_t m_length;

   public:
    String(std::string string) : String(string.c_str(), string.length()) {}

    String(const char* string) : String(string, -1) {}

    String(const char* string, int64_t length) {
        if (string == nullptr) {
            m_data = nullptr;
            m_length = 0;
        } else {
            if (length < 0) {
                length = strlen(string);
            }

            m_data = new char[length + 1];
            memcpy(m_data, string, length);
            m_data[length] = 0;

            m_length = length;
        }
    }

    String(const String& other) : String(other.m_data, other.m_length) {}

    ~String() { delete[] m_data; }

    const char* c_str() const { return m_data; }

    uint64_t length() const { return m_length; }

    String& operator=(const String& other) {
        delete[] m_data;

        m_data = new char[other.m_length + 1];
        memcpy(m_data, other.m_data, other.m_length);
        m_data[other.m_length] = 0;

        m_length = other.m_length;

        return *this;
    }

    String& operator=(String&& other) {
        if (this == &other) {
            return *this;
        }

        delete[] m_data;
        m_data = other.m_data;
        m_length = other.m_length;

        other.m_data = nullptr;
        other.m_length = 0;

        return *this;
    }
};
}  // namespace s4pkg::lib
}

#if __has_include("fmt/format.h")
#include <fmt/format.h>

template <>
struct fmt::formatter<s4pkg::lib::String> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(s4pkg::lib::String const& string, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", string.c_str());
    }
};
#endif
