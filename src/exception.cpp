#include <Lunaris/Future/exception.h>

namespace Lunaris {
namespace Future {

    FutureException::FutureException(const std::string& msg) noexcept
        : std::runtime_error(msg)
    {
    }

    FutureException::FutureException(const char* msg) noexcept
        : std::runtime_error(msg)
    {
    }

    const char* FutureException::what() const noexcept {
        return std::runtime_error::what();
    }

} // namespace Future
} // namespace Lunaris