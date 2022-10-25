#pragma once

#include <exception>
#include <string>

namespace s4pkg {
class PackageException : public std::exception {
   private:
    std::string m_message;

   public:
    explicit PackageException(const std::string message)
        : m_message(std::move(message)){};
    const char* what() const noexcept override { return m_message.c_str(); }
};
}  // namespace s4pkg
