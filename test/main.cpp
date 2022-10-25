#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_WINDOWS_CRTDBG 1
#include "catch.hpp"

#include <s4pkg/package/ipackage.h>

TEST_CASE("Test fundamental types", "[package]") {
    s4pkg::PackageVersion version = {1, 2};
    REQUIRE(version.m_majorVersion == 1);
    REQUIRE(version.m_minorVersion == 2);

    s4pkg::PackageTime time = {1666719234LL};
    REQUIRE(time.m_unixTime == 1666719234LL);
}
