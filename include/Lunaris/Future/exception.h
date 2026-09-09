#pragma once

#include <stdexcept>

namespace Lunaris {
namespace Future {

    class FutureException : public std::runtime_error {
    public:
        explicit FutureException(const std::string&) noexcept;
        explicit FutureException(const char*) noexcept;

        const char* what() const noexcept;
    };

} // namespace Future
} // namespace Lunaris