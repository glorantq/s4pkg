#pragma once

#include <s4pkg/internal/export.h>
#include <s4pkg/package/ipackage.h>

#include <istream>

namespace s4pkg {

typedef struct S4PKG_EXPORT PackageLoadResult {
    std::shared_ptr<IPackage> m_package;
    std::string m_errorMessage;
} PackageLoadResult;

S4PKG_EXPORT const PackageLoadResult loadPackage(std::istream& stream);

}  // namespace s4pkg
