#pragma once

#include <condition_variable>
#include <type_traits>
#include <functional>
#include <optional>
#include <atomic>
#include <mutex>

#include <Lunaris/Future/exception.h>

namespace Lunaris {
namespace Future {

    /**
     * @brief Type to avoid void when type is void, using bool instead
     */
    template<typename T>
    using StoreType = std::conditional_t<std::is_void_v<T>, bool, T>;

    /**
     * @brief Status of the hold object
     */
    enum class e_wait_status {
        VALUE_SET, // the value is set and ready to read
        VALUE_UNSET_TIMEOUT, // the value is still to be set or already withdrawn
        VALUE_TUNNELED // the value has been read already or redirected 
    };

    /**
     * @brief Pipe is a pipe-able variable.
     * 
     * This means that by default it stores the value you assign to it
     * 
     * If you make_callback(), the variable will be moved to the callback instead
     */
    template<typename T>
    class pipe {
    public:
        using BaseType = StoreType<T>;
    private:
        using StoreMaybeType = std::optional<BaseType>;
        using MoveCBType = std::function<void(BaseType&&)>;

        StoreMaybeType m_value;
        MoveCBType m_redirect;

        std::condition_variable m_cond;
        std::mutex m_mtx;
        std::atomic<bool> m_redirected{false};
    public:
        pipe();
        pipe(const pipe&) = delete;
        pipe(pipe&&) = delete;
        void operator=(const pipe&) = delete;
        void operator=(pipe&&) = delete;

        /**
         * @brief Makes this redirect the set to a callback instead
         * 
         * @param `callback` callback called instead of moving value to variable inside
         */
        template<typename Function>
        void make_callback(Function&& callback);

        /**
         * @brief Set this object's value (or call the callback with this)
         * 
         * @param `move` variable being moved to this
         */
        void set(BaseType&& move);
        
        /**
         * @brief Checks if value contains something.
         * 
         * @return `bool` true if holds something
         */
        bool can_get();

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
         * @brief Take stored variable from it
         * 
         * NOTE: if make_callback was used, this will throw FutureException
         * 
         * NOTE: this can only be called once.
         * 
         * @return `BaseType` the value moved off this object
         */
        BaseType get();
    };

} // namespace Future
} // namespace Lunaris

#include <Lunaris/Future/impl/pipe.ipp>