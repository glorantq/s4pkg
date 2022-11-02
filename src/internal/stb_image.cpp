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

#include <miniz.h>

static unsigned char* mz_stbi_zlib_compress(unsigned char* data,
                                            int data_len,
                                            int* out_len,
                                            int quality) {
    mz_ulong buflen = mz_compressBound(data_len);

    unsigned char* buf = (unsigned char*)malloc(buflen);

    if (buf == NULL ||
        mz_compress2(buf, &buflen, data, data_len, quality) != 0) {
        free(buf);
        return NULL;
    }

    *out_len = buflen;
    return buf;
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STBIW_ZLIB_COMPRESS mz_stbi_zlib_compress
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
