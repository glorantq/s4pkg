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
#include <s4pkg/resources/imageresource.h>

namespace s4pkg::resources::ts4 {

class S4PKG_EXPORT DSTResource : public IImageResource {
   public:
    DSTResource(uint32_t instanceEx,
                uint32_t instance,
                uint32_t group,
                const std::vector<uint8_t>& data)
        : IImageResource(instanceEx, instance, group, ResourceType::DST_IMAGE) {
        setDataWithFormat(
            internal::imagecoder::ImageFormat::DST5,  // TODO: Handle other DST
                                                      // images
            data);
    }

    // IResource interface
   public:
    std::string getFriendlyName() const override { return "DST Image"; }

    // Object interface
   public:
    const std::string toString() const override;
};

}  // namespace s4pkg::resources::ts4
