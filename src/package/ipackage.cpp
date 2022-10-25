#include <s4pkg/package/ipackage.h>

#include <fmt/chrono.h>
#include <fmt/core.h>

namespace s4pkg {

const std::string PackageVersion::toString() const {
    return fmt::format("{}.{}", this->m_majorVersion, this->m_minorVersion);
}

const std::string PackageTime::toString() const {
    return fmt::format("{:%Y-%m-%d %H:%M:%S}", this->m_timePoint);
}

const std::string PackageHeader::toString() const {
    return "[ not implemented ]";  // TODO: Implement
}

};  // namespace s4pkg
