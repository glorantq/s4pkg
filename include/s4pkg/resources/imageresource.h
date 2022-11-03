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
#include <s4pkg/internal/image.h>
#include <s4pkg/internal/imagecoder.h>
#include <s4pkg/resources/iresource.h>

#include <memory>

namespace s4pkg::resources {

class S4PKG_EXPORT IImageResource : public IResource {
   private:
    std::shared_ptr<internal::Image> m_image;
    internal::imagecoder::ImageFormat m_format;

   public:
    IImageResource(uint32_t instanceEx,
                   uint32_t instance,
                   uint32_t group,
                   ResourceType resourceType,
                   internal::imagecoder::ImageFormat format,
                   const std::vector<uint8_t>& data)
        : IResource(instanceEx, instance, group, resourceType),
          m_format(format) {
        m_image = internal::imagecoder::decode(data, format);
    }

    uint32_t getWidth() const {
        if (m_image) {
            return m_image->getWidth();
        } else {
            return 0;
        }
    }

    uint32_t getHeight() const {
        if (m_image) {
            return m_image->getHeight();
        } else {
            return 0;
        }
    }

    const std::vector<uint8_t> getPixelData() const {
        if (m_image) {
            return m_image->getPixelData();
        } else {
            return {};
        }
    }

    // IResource interface
   public:
    std::vector<uint8_t> write() const override;
};

}  // namespace s4pkg::resources
