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

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_WINDOWS_CRTDBG 1
#include "catch.hpp"

#include <s4pkg/internal/imagecoder.h>
#include <s4pkg/package/ipackage.h>
#include <s4pkg/package/packages.h>

#include <fstream>
#include <iostream>
#include <istream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

TEST_CASE("Test image coder", "data") {
    std::ifstream thumbnailStream("./thumbnail.jfif", std::ios_base::binary);
    std::vector<uint8_t> fileContents(
        (std::istreambuf_iterator<char>(thumbnailStream)),
        std::istreambuf_iterator<char>());
    thumbnailStream.close();

    auto decodedThumbnail = s4pkg::internal::imagecoder::decode(
        fileContents, s4pkg::internal::imagecoder::JFIF_WITH_ALPHA);

    if (decodedThumbnail != nullptr) {
        std::cout << "Decoded thumbnail: " << decodedThumbnail->toString()
                  << std::endl;

        stbi_write_png("./thumbnail_out.png", decodedThumbnail->getWidth(),
                       decodedThumbnail->getHeight(), 4,
                       decodedThumbnail->getPixelData().data(),
                       4 * decodedThumbnail->getWidth());
    } else {
        std::cerr << "Failed to decode thumbnail!" << std::endl;
    }
}

TEST_CASE("Test good in-memory package", "package") {
    std::ifstream packageStream("./test.package", std::ios_base::binary);
    s4pkg::PackageLoadResult package = s4pkg::loadPackage(packageStream);
    packageStream.close();

    INFO(package.m_errorMessage);
    REQUIRE(package.m_package != nullptr);
    REQUIRE(package.m_package->isValid());

    s4pkg::PackageVersion fileVersion = package.m_package->getFileVersion();
    REQUIRE(fileVersion.m_majorVersion == 2);
    REQUIRE(fileVersion.m_minorVersion == 1);
    std::cout << "Package file version: " << fileVersion.toString()
              << std::endl;

    s4pkg::PackageVersion userVersion = package.m_package->getUserVersion();
    REQUIRE(userVersion.m_majorVersion == 0);
    REQUIRE(userVersion.m_minorVersion == 0);
    std::cout << "Package user version: " << userVersion.toString()
              << std::endl;

    s4pkg::PackageTime createdTime = package.m_package->getCreationTime();
    REQUIRE(createdTime.toTimestamp() == 0LL);
    std::cout << "Created date: " << createdTime.toString() << std::endl;

    s4pkg::PackageTime updatedTime = package.m_package->getModifiedTime();
    REQUIRE(updatedTime.toTimestamp() == 0LL);
    std::cout << "Updated date: " << updatedTime.toString() << std::endl;

    s4pkg::PackageHeader header = package.m_package->getPackageHeader();
    REQUIRE(header.m_indexRecordEntryCount == 7);
    REQUIRE(header.m_indexRecordPositionLow == 0);
    REQUIRE(header.m_indexRecordSize == 228);
    REQUIRE(header.m_indexRecordPosition == 204380);
    std::cout << "Package header: " << header.toString() << std::endl;

    s4pkg::PackageFlags flags = package.m_package->getPackageFlags();
    REQUIRE_FALSE(flags.m_isConstantGroup);
    REQUIRE_FALSE(flags.m_isConstantType);
    REQUIRE_FALSE(flags.m_isConstantInstance);
    std::cout << "Package flags: " << flags.toString() << std::endl;

    std::vector<s4pkg::IndexEntry> index = package.m_package->getPackageIndex();
    REQUIRE(index.size() == 7);
    REQUIRE(index[0].m_type == (s4pkg::ResourceType)0x34AEECB);
    REQUIRE(index[4].m_group == 0x2);
    REQUIRE(index[2].m_instanceEx == 0xFFA38199);
    REQUIRE(index[1].m_compressionType == s4pkg::CompressionType::ZLIB);
    REQUIRE(index[3].m_sizeDecompressed == 200400);
    REQUIRE(index[5].m_isExtendedCompressionType == 1);
    REQUIRE(index[6].m_instance == 0x68699D89);

    for (const auto& entry : index) {
        std::cout << "Entry: " << entry.toString() << std::endl;
    }

    for (const auto& resource : package.m_package->getResources()) {
        std::cout << "Resource: " << resource->toString() << std::endl;
    }

    std::cout << package.m_package->toString() << std::endl;

    std::ofstream outputStream("./output.package", std::ios_base::binary);
    package.m_package->write(outputStream);
    outputStream.close();
}

TEST_CASE("Test bad in-memory package", "package") {
    std::ifstream packageStream("./bad.package");
    s4pkg::PackageLoadResult package = s4pkg::loadPackage(packageStream);

    INFO(package.m_errorMessage);
    REQUIRE(package.m_package == nullptr);
}
