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
#include <s4pkg/packageexception.h>
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
                   ResourceType resourceType)
        : IResource(instanceEx, instance, group, resourceType) {}

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

    const lib::ByteBuffer getPixelData() const {
        if (m_image) {
            return m_image->getPixelData();
        } else {
            return {};
        }
    }

    const internal::imagecoder::ImageFormat getFormat() const {
        return this->m_format;
    }

    void setImage(uint32_t width, uint32_t height, lib::ByteBuffer pixelData) {
        m_image = std::make_shared<internal::Image>(width, height, pixelData);
    }

   protected:
    void setDataWithFormat(internal::imagecoder::ImageFormat format,
                           const lib::ByteBuffer& data) {
        this->m_format = format;
        this->m_image = internal::imagecoder::decode(data, format);
    }

    // IResource interface
   public:
    lib::ByteBuffer write() const override;
};

}  // namespace s4pkg::resources
