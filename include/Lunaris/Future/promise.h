#pragma once

#include <vector>

#include <Lunaris/Future/future.h>

namespace Lunaris {
namespace Future {

    /**
     * @brief A Promise is a promise that a value will be set in the future
     * 
     * From this, you get a Future object to handle the return of this when this is set.
     */
    template<typename T>
    class Promise {
        Future<T> m_fut;
    public:
        /**
         * @brief Set the value of this Promise, setting its Future flag
         */
        template<typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0>
        void set();

        /**
         * @brief Set the value of this Promise, setting its Future with this value
         * 
         * @param `val` value to send to Future somewhere
         */
        template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
        void set(Q val);

        /**
         * @brief Get the Future object related to this Promise
         * 
         * NOTE: recalling this makes the prior returned Future detached
         * 
         * @return `Future` the Future object
         */
        Future<T> get_future();

        /**
         * @brief Make multiple Futures of this Promise at once
         * 
         * NOTE: recalling this makes the prior returned Futures detached
         * 
         * @param `amount` how many to spawn
         * @return `std::vector<Future>` the Future objects
         */
        template<typename Q = T, std::enable_if_t<std::is_copy_constructible_v<Q> && std::is_copy_assignable_v<Q>, int> = 0>
        std::vector<Future<T>> get_multiple_future(const size_t amount);
    };

	/**
	 * @brief Create a Future object that is already set with value
	 * 
	 * @param `value` set Future with this value already
	 * @return `Future` the Future with value already set
	 */
	template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
	Future<T> make_empty_future(T&& value);

	/**
	 * @brief Create a Future object that is already set
	 * 
	 * @return `Future` the Future with value already set
	 */
	template<typename T, std::enable_if_t<std::is_void_v<T>, int> = 0>
	Future<void> make_empty_future();

} // namespace Future
} // namespace Lunaris

#include <Lunaris/Future/impl/promise.ipp>