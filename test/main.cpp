#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_WINDOWS_CRTDBG 1
#include "catch.hpp"

#include <s4pkg/package/ipackage.h>
#include <s4pkg/package/packages.h>

#include <fstream>

TEST_CASE("Test streams", "[io]") {}

TEST_CASE("Test fundamental types", "[types]") {
    s4pkg::PackageVersion version = {1, 2};
    INFO("PackageVersion::toString(): " << version.toString());
    REQUIRE(version.m_majorVersion == 1);
    REQUIRE(version.m_minorVersion == 2);

    s4pkg::PackageTime time = {1666719234LL};
    INFO("PackageTime::toString(): " << time.toString());
    REQUIRE(std::chrono::duration_cast<std::chrono::seconds>(
                time.m_timePoint.time_since_epoch())
                .count() == 1666719234LL);

    s4pkg::PackageHeader header = {1, 2, 3, 4};
    INFO("PackageHeader::toString(): " << header.toString());
    REQUIRE(header.m_indexRecordEntryCount == 1);
    REQUIRE(header.m_indexRecordPositionLow == 2);
    REQUIRE(header.m_indexRecordSize == 3);
    REQUIRE(header.m_indexRecordPosition == 4);
}

TEST_CASE("Test good in-memory package", "[package]") {
    std::ifstream packageStream("./test.package");
    s4pkg::PackageLoadResult package = s4pkg::loadPackage(packageStream);
    packageStream.close();

    INFO(package.m_errorMessage);
    REQUIRE(package.m_package != nullptr);
    REQUIRE(package.m_package->isValid());

    s4pkg::PackageVersion fileVersion = package.m_package->getFileVersion();
    REQUIRE(fileVersion.m_majorVersion == 2);
    REQUIRE(fileVersion.m_minorVersion == 1);

    s4pkg::PackageVersion userVersion = package.m_package->getUserVersion();
    REQUIRE(userVersion.m_majorVersion == 0);
    REQUIRE(userVersion.m_minorVersion == 0);

    s4pkg::PackageTime createdTime = package.m_package->getCreationTime();
    REQUIRE(std::chrono::duration_cast<std::chrono::seconds>(
                createdTime.m_timePoint.time_since_epoch())
                .count() == 0LL);

    s4pkg::PackageTime updatedTime = package.m_package->getModifiedTime();
    REQUIRE(std::chrono::duration_cast<std::chrono::seconds>(
                updatedTime.m_timePoint.time_since_epoch())
                .count() == 0LL);

    s4pkg::PackageHeader header = package.m_package->getPackageHeader();
    REQUIRE(header.m_indexRecordEntryCount == 7);
    REQUIRE(header.m_indexRecordPositionLow == 0);
    REQUIRE(header.m_indexRecordSize == 228);
    REQUIRE(header.m_indexRecordPosition == 204380);
}

TEST_CASE("Test bad in-memory package", "[package]") {
    std::ifstream packageStream("./bad.package");
    s4pkg::PackageLoadResult package = s4pkg::loadPackage(packageStream);

    INFO(package.m_errorMessage);
    REQUIRE(package.m_package == nullptr);
}
