#include <s4pkg/package/packages.h>

#include <s4pkg/internal/inmemorypackage.h>
#include <s4pkg/packageexception.h>

#include <tuple>

namespace s4pkg {

S4PKG_EXPORT const PackageLoadResult loadPackage(std::istream& stream) {
    try {
        return {std::make_shared<internal::InMemoryPackage>(
                    internal::InMemoryPackage(stream)),
                ""};
    } catch (PackageException e) {
        return {nullptr, e.what()};
    }

    return {nullptr, ""};
}

}  // namespace s4pkg
