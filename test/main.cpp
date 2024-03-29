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

#include <s4pkg/internal/dds.h>
#include <s4pkg/internal/imagecoder.h>
#include <s4pkg/internal/rle.h>
#include <s4pkg/package/ipackage.h>
#include <s4pkg/package/packages.h>
#include <s4pkg/version.h>

#include <fstream>
#include <iostream>
#include <istream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

TEST_CASE("Test RLE2", "imagecoder") {
    std::ifstream rleStream("./test.rle2", std::ios_base::binary);

    REQUIRE(rleStream.good());

    auto rleImage = s4pkg::internal::rle::readFile(rleStream);
    rleStream.close();

    s4pkg::lib::ByteBuffer ddsData =
        s4pkg::internal::dds::writeFile(rleImage.m_ddsFile);

    std::ofstream ddsStream("./test.rle2.dds", std::ios_base::binary);
    ddsStream.write((const char*)ddsData.data(), ddsData.size());
    ddsStream.close();
}

TEST_CASE("Test image coder", "data") {
    std::cout << "Using s4pkg version " << S4PKG_VERSION << std::endl;

    std::ifstream thumbnailStream("./thumbnail.jfif", std::ios_base::binary);
    std::vector<uint8_t> fileContents0(
        (std::istreambuf_iterator<char>(thumbnailStream)),
        std::istreambuf_iterator<char>());
    thumbnailStream.close();

    auto fileContents =
        s4pkg::lib::ByteBuffer(fileContents0.data(), fileContents0.size());

    auto decodedThumbnail = s4pkg::internal::imagecoder::decode(
        fileContents, s4pkg::internal::imagecoder::JFIF_WITH_ALPHA);

    REQUIRE(decodedThumbnail != nullptr);

    std::cout << "Decoded thumbnail: " << decodedThumbnail->toString().c_str()
              << std::endl;

    REQUIRE(decodedThumbnail->getWidth() == 104);
    REQUIRE(decodedThumbnail->getHeight() == 148);

    stbi_write_png("./thumbnail_out.png", decodedThumbnail->getWidth(),
                   decodedThumbnail->getHeight(), 4,
                   decodedThumbnail->getPixelData().data(),
                   4 * decodedThumbnail->getWidth());

    std::cout << "Re-encoding thumbnail..." << std::endl;
    auto encodedThumbnail = s4pkg::internal::imagecoder::encode(
        *decodedThumbnail, s4pkg::internal::imagecoder::JFIF_WITH_ALPHA);

    REQUIRE(encodedThumbnail.size() > 0);

    std::ofstream thumbnailOut("./thumbnail_out.jfif", std::ios_base::binary);
    for (int i = 0; i < encodedThumbnail.size(); i++) {
        thumbnailOut.write((const char*)&encodedThumbnail[i], 1);
    }
    thumbnailOut.close();

    std::ifstream dstStream("./dst5.dds", std::ios_base::binary);
    std::vector<uint8_t> dstContents0(
        (std::istreambuf_iterator<char>(dstStream)),
        std::istreambuf_iterator<char>());
    dstStream.close();

    s4pkg::lib::ByteBuffer dstContents(dstContents0.data(),
                                       dstContents0.size());

    auto decodedDst = s4pkg::internal::imagecoder::decode(
        dstContents, s4pkg::internal::imagecoder::DST5);

    REQUIRE(decodedDst != nullptr);

    std::cout << "Decoded DST5: " << decodedDst->toString().c_str()
              << std::endl;

    std::cout << "Re-encoding DST5..." << std::endl;

    s4pkg::lib::ByteBuffer dstThumbnail = s4pkg::internal::imagecoder::encode(
        *decodedDst, s4pkg::internal::imagecoder::DST5);

    std::ofstream dstOut("./dst5_out.dds", std::ios_base::binary);
    for (int i = 0; i < dstThumbnail.size(); i++) {
        dstOut.write((const char*)&dstThumbnail[i], 1);
    }
    dstOut.close();
}

TEST_CASE("Test good in-memory package", "package") {
    std::ifstream packageStream("./TURBODRIVER_WickedWhims_Tuning.package",
                                std::ios_base::binary);
    s4pkg::PackageLoadResult package = s4pkg::loadPackage(packageStream);
    packageStream.close();

    INFO(package.m_errorMessage.c_str());
    REQUIRE(package.m_package != nullptr);
    REQUIRE(package.m_package->isValid());

    s4pkg::PackageVersion fileVersion = package.m_package->getFileVersion();
    REQUIRE(fileVersion.m_majorVersion == 2);
    REQUIRE(fileVersion.m_minorVersion == 1);
    std::cout << "Package file version: " << fileVersion.toString().c_str()
              << std::endl;

    s4pkg::PackageVersion userVersion = package.m_package->getUserVersion();
    REQUIRE(userVersion.m_majorVersion == 0);
    REQUIRE(userVersion.m_minorVersion == 0);
    std::cout << "Package user version: " << userVersion.toString().c_str()
              << std::endl;

    uint64_t createdTime = package.m_package->getCreationTime();
    REQUIRE(createdTime == 0LL);
    std::cout << "Created date: " << createdTime << std::endl;

    uint64_t updatedTime = package.m_package->getModifiedTime();
    REQUIRE(updatedTime == 0LL);
    std::cout << "Updated date: " << updatedTime << std::endl;

    s4pkg::PackageHeader header = package.m_package->getPackageHeader();
    // REQUIRE(header.m_indexRecordEntryCount == 7);
    // REQUIRE(header.m_indexRecordPositionLow == 0);
    // REQUIRE(header.m_indexRecordSize == 228);
    // REQUIRE(header.m_indexRecordPosition == 204380);
    std::cout << "Package header: " << header.toString().c_str() << std::endl;

    s4pkg::PackageFlags flags = package.m_package->getPackageFlags();
    REQUIRE_FALSE(flags.m_isConstantGroup);
    REQUIRE_FALSE(flags.m_isConstantType);
    REQUIRE_FALSE(flags.m_isConstantInstance);
    std::cout << "Package flags: " << flags.toString().c_str() << std::endl;

    std::vector<s4pkg::IndexEntry> index = package.m_package->getPackageIndex();

    // REQUIRE(index.size() == 7);
    // REQUIRE(index[0].m_type == (s4pkg::ResourceType)0x34AEECB);
    // REQUIRE(index[4].m_group == 0x2);
    // REQUIRE(index[2].m_instanceEx == 0xFFA38199);
    // REQUIRE(index[1].m_compressionType == s4pkg::CompressionType::ZLIB);
    // REQUIRE(index[3].m_sizeDecompressed == 200400);
    // REQUIRE(index[5].m_isExtendedCompressionType == 1);
    // REQUIRE(index[6].m_instance == 0x68699D89);

    for (const auto& entry : index) {
        std::cout << "Entry: " << entry.toString().c_str() << std::endl;
    }

    for (auto& resource : package.m_package->getResources()) {
        std::cout << "Resource: " << resource->toString().c_str() << " ("
                  << resource->getFriendlyName().c_str() << ")" << std::endl;
    }

    /*std::shared_ptr<s4pkg::IResource> lastResource =
        *(package.m_package->getResources().end() - 1);

    size_t sizeBefore = package.m_package->getResources().size();
    REQUIRE(package.m_package->deleteResource(lastResource));
    REQUIRE(package.m_package->getResources().size() == sizeBefore - 1);

    for (auto& resource : package.m_package->getResources()) {
        std::cout << "Resource: " << resource->toString() << " ("
                  << resource->getFriendlyName() << ")" << std::endl;
    } */

    std::cout << package.m_package->toString().c_str() << std::endl;

    // std::ofstream outputStream("./output.package", std::ios_base::binary);
    // package.m_package->write(outputStream);
    // outputStream.close();
}

TEST_CASE("Test bad in-memory package", "package") {
    std::ifstream packageStream("./bad.package");
    s4pkg::PackageLoadResult package = s4pkg::loadPackage(packageStream);

    INFO(package.m_errorMessage.c_str());
    REQUIRE(package.m_package == nullptr);
}
