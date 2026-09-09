#pragma once

#include <memory>

#include <Lunaris/Future/exception.h>
#include <Lunaris/Future/pipe.h>

namespace Lunaris {
namespace Future {

    template<typename T>
    class Promise;

    /**
     * @brief Future is a class that holds an object that is yet to be set
     * 
     * This has then() for automatic callback on set.
     */
    template<typename T>
    class Future {
        std::shared_ptr<pipe<T>> m_pipe;

        template<typename Any>
        friend class Future;
        template<typename Any>
        friend class Promise;

    public:
        Future();
        Future(Future&& oth) noexcept;
        void operator=(Future&& oth) noexcept;

        Future(const Future&) = delete;
        void operator=(const Future&) = delete;

        /**
         * @brief Attempts to get. Holds if not set yet.
         */
        template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
        void get() ;

        /**
         * @brief Attempts to get value. Holds if not set yet.
         * 
         * @return `T` the value stored
         */
        template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
        T get();

        /**
         * @brief Wait until something happens
         * 
         * @return `e_wait_status` what happened
         */
        e_wait_status wait();

        /**
         * @brief Wait until something happens until timeout
         * 
         * @param `wait_for` wait for how long?
         * @return `e_wait_status` what happened
         */
        template<typename Rep, typename Period>
        e_wait_status wait(const std::chrono::duration<Rep, Period>& wait_for);

        /**
         * @brief Makes this future redirect its own value to a callback.
         * 
         * NOTE: after this, getting manually from this Future is invalid.
         * 
         * @param `callback` a function to take the T type from this future and do something with it.
         * @return `Future` a Future of the result of the callback.
         */
        template<typename Function>
        auto then(Function&& callback);
    };

} // namespace Future
} // namespace Lunaris

#include <Lunaris/Future/impl/future.ipp>