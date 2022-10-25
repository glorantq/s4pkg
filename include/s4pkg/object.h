#pragma once

#include <string>

#include <s4pkg/internal/export.h>

namespace s4pkg {

/**
 * @brief Base class for almost every object this library exports. Contains some
 * useful methods shared between them.
 */
class S4PKG_EXPORT Object {
   public:
    /**
     * @brief Returns a human-readable representation of this object
     * @return readable state of this object
     */
    virtual const std::string toString() const = 0;
};

}  // namespace s4pkg
