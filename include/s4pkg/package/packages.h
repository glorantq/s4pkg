#pragma once

#include <s4pkg/internal/export.h>
#include <s4pkg/package/ipackage.h>

#include <istream>

namespace s4pkg {

/**
 * @brief If m_package is nullptr, then m_errorMessage contains the reason why
 * loading failed.
 */
struct S4PKG_EXPORT PackageLoadResult {
    std::shared_ptr<IPackage> m_package;
    std::string m_errorMessage;
};

/**
 * @brief Loads a package from stream, and stores it in memory. It is safe to
 * close the stream after this method returns.
 * @param stream: the stream to read from
 * @return A struct with either the package object, or an error message
 */
S4PKG_EXPORT const PackageLoadResult loadPackage(std::istream& stream);

}  // namespace s4pkg
