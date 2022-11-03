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

#include <s4pkg/internal/globals.h>
#include <s4pkg/resources/fallbackresourcefactory.h>
#include <s4pkg/resources/ts4/thumbnailresourcefactory.h>

namespace s4pkg::internal::globals {

const std::map<s4pkg::ResourceType, std::shared_ptr<s4pkg::IResourceFactory>>
    g_resourceFactoryMapping = {
        {s4pkg::ResourceType::UNKNOWN,
         std::make_shared<s4pkg::factories::FallbackResourceFactory>()},
        {s4pkg::ResourceType::THUMBNAIL_IMAGE,
         std::make_shared<s4pkg::factories::ts4::ThumbnailResourceFactory>()},
};

const std::shared_ptr<s4pkg::IResourceFactory> getResourceFactoryFor(
    s4pkg::ResourceType type) {
    if (g_resourceFactoryMapping.find(type) != g_resourceFactoryMapping.end()) {
        return g_resourceFactoryMapping.at(type);
    } else {
        if (type == s4pkg::ResourceType::UNKNOWN) {
            return nullptr;
        }

        return getResourceFactoryFor(s4pkg::ResourceType::UNKNOWN);
    }
}
};  // namespace s4pkg::internal::globals
