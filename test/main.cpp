#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_WINDOWS_CRTDBG 1
#include "catch.hpp"

#include <s4pkg/package/ipackage.h>
#include <s4pkg/package/packages.h>

#include <fstream>
#include <iostream>

TEST_CASE("Test good in-memory package", "[package]") {
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
    REQUIRE(std::chrono::duration_cast<std::chrono::seconds>(
                createdTime.m_timePoint.time_since_epoch())
                .count() == 0LL);
    std::cout << "Created date: " << createdTime.toString() << std::endl;

    s4pkg::PackageTime updatedTime = package.m_package->getModifiedTime();
    REQUIRE(std::chrono::duration_cast<std::chrono::seconds>(
                updatedTime.m_timePoint.time_since_epoch())
                .count() == 0LL);
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
}

TEST_CASE("Test bad in-memory package", "[package]") {
    std::ifstream packageStream("./bad.package");
    s4pkg::PackageLoadResult package = s4pkg::loadPackage(packageStream);

    INFO(package.m_errorMessage);
    REQUIRE(package.m_package == nullptr);
}
