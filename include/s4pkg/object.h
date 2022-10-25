#pragma once

#include <string>

#include <s4pkg/internal/export.h>

namespace s4pkg {

class S4PKG_EXPORT Object {
   public:
    virtual const std::string toString() const = 0;
};

}  // namespace s4pkg
