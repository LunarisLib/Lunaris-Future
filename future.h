#pragma once

#include <future>
#include <functional>
#include <stdexcept>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace Lunaris {

	namespace detail_future {

		template<typename T>
		struct _tunnel {
			T m_store{};
			std::function<void(T)> m_get = [this](T a) {m_store = std::move(a); }; // on set, this is called. By default, copy to m_store
			mutable std::mutex m_safety;
			mutable std::condition_variable m_cond;
			bool m_redir_enabled = false;
			bool m_store_set = false;

			void set(T);
			void redirect_nolock(std::function<void(T)>);
			void wait() const;
			template<typename _Rep, typename _Period> void wait_for(const std::chrono::duration<_Rep, _Period>) const;
		};

		template<>
		struct _tunnel<void> {
			std::function<void()> m_get = []() {};
			mutable std::mutex m_safety;
			mutable std::condition_variable m_cond;
			bool m_redir_enabled = false;
			bool m_store_set = false;

			void set();
			void redirect_nolock(std::function<void()>);
			void wait() const;
			template<typename _Rep, typename _Period> void wait_for(const std::chrono::duration<_Rep, _Period>) const;
		};
	}

	/// <summary>
	/// <para>This holds a not yet set variable. You can get the value itself or link a function to run when the variable is ready.</para>
	/// </summary>
	template<typename T>
	class future {
		std::shared_ptr<detail_future::_tunnel<T>> m_next = std::make_shared<detail_future::_tunnel<T>>();

		// this makes void coexist with const something&
		using cr_T = std::add_lvalue_reference_t<std::add_const_t<std::remove_pointer_t<T*>>>;
		template<typename Any> friend class future;

	protected:
		/// <summary>
		/// <para>Get next future that'll have the value set when this is set.</para>
		/// </summary>
		/// <returns>{future&lt;T&gt;}</returns>
		template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
		future<T> get_future();

		/// <summary>
		/// <para>Get next future that'll have the value set when this is set.</para>
		/// </summary>
		/// <returns>{future&lt;T&gt;}</returns>
		template<typename Q = T, std::enable_if_t< std::is_void_v<Q>, int> = 0>
		future<T> get_future();

		/// <summary>
		/// <para>Set value and send to future (if any).</para>
		/// </summary>
		/// <param name="{Q}">Value to be set.</param>
		template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
		void set_value(Q);

		/// <summary>
		/// <para>Set value and send to future (if any).</para>
		/// </summary>
		template<typename Q = T, std::enable_if_t< std::is_void_v<Q>, int> = 0>
		void set_value();
	public:
		future() = default;
		future(future&&) noexcept;

		future(const future&) = delete;
		void operator=(const future&) = delete;
		void operator=(future&&) = delete;

		/// <summary>
		/// <para>Get the value set, or wait for it, in a const reference way, or void if T is void.</para>
		/// </summary>
		/// <returns>{const T&amp;} Reference to variable internally if valid or void if T is void.</returns>
		template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
		typename future<T>::cr_T get() const;

		/// <summary>
		/// <para>Wait for set.</para>
		/// </summary>
		template<typename Q = T, std::enable_if_t< std::is_void_v<Q>, int> = 0>
		void get() const;

		/// <summary>
		/// <para>Wait for the value to be set. Valid only if you have not called then().</para>
		/// </summary>
		/// <returns>{bool} Returns true if it was set properly. May return false if then() was called and exception didn't trigger somehow.</returns>
		bool wait() const;

		/// <summary>
		/// <para>Wait for the value to be set for a period of time only. Valid only if you have not called then().</para>
		/// <para>.</para>
		/// </summary>
		/// <returns>{bool} Returns true if it was set.</returns>
		template<typename _Rep, typename _Period> bool wait(const std::chrono::duration<_Rep, _Period>) const;

		/// <summary>
		/// <para>Get and take the value permanently from it, like a move.</para>
		/// <para>After this, this will be unset and invalid to get().</para>
		/// </summary>
		/// <returns>{T} The value moved.</returns>
		template<typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0>
		T get_take() const;

		/// <summary>
		/// <para>Get and reset.</para>
		/// <para>After this, this will be unset and invalid to get().</para>
		/// </summary>
		template<typename Q = T, std::enable_if_t< std::is_void_v<Q>, int> = 0>
		void get_take() const;

		/// <summary>
		/// <para>Instead of get(), you can automatically call a function with the value when set.</para>
		/// <para>The function, on return, will set the next future.</para>
		/// </summary>
		/// <param name="{V}">A function to handle the situation.</param>
		/// <returns>{future} A future of function's return type.</returns>
		template<typename V, typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V(Q)>, std::enable_if_t<!std::is_void_v<Res>, int> = 0>
		auto then(V);

		/// <summary>
		/// <para>Instead of get(), you can automatically call a function with the value when set.</para>
		/// <para>The function, on return, will set the next future.</para>
		/// </summary>
		/// <param name="{V}">A function to handle the situation.</param>
		/// <returns>{future} A future of function's return type.</returns>
		template<typename V, typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V()>, std::enable_if_t<!std::is_void_v<Res>, int> = 0>
		auto then(V);

		/// <summary>
		/// <para>Instead of get(), you can automatically call a function with the value when set.</para>
		/// <para>The function, on return, will set the next future.</para>
		/// </summary>
		/// <param name="{V}">A function to handle the situation.</param>
		/// <returns>{future} A future of function's return type.</returns>
		template<typename V, typename Q = T, std::enable_if_t<!std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V(Q)>, std::enable_if_t<std::is_void_v<Res>, int> = 0>
		auto then(V);

		/// <summary>
		/// <para>Instead of get(), you can automatically call a function with the value when set.</para>
		/// <para>The function, on return, will set the next future.</para>
		/// </summary>
		/// <param name="{V}">A function to handle the situation.</param>
		/// <returns>{future} A future of function's return type.</returns>
		template<typename V, typename Q = T, std::enable_if_t<std::is_void_v<Q>, int> = 0, typename Res = std::result_of_t<V()>, std::enable_if_t<std::is_void_v<Res>, int> = 0>
		auto then(V);
	};

	/// <summary>
	/// <para>You promise you'll have the value later, but not now!</para>
	/// <para>Create a future from this and set the value in the future, somewhere else, in the future.</para>
	/// </summary>
	template<typename T>
	class promise : protected future<T> {
	public:
		promise() = default;

		using future<T>::get_future;
		using future<T>::set_value;
	};

	/// <summary>
	/// <para>If you were about to do something, but it didn't work even before knowing it, you can promise with a response already set (future with value set already).</para>
	/// </summary>
	/// <param name="{T}">The pre-determined value for the future.</param>
	/// <returns>{future} The future with value already set.</returns>
	template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0>
	future<T> make_empty_future(const T&);

	/// <summary>
	/// <para>If you were about to do something, but it didn't work even before knowing it, you can promise with a response already set (future with value set already).</para>
	/// </summary>
	/// <returns>{future} The future with value already set.</returns>
	template<typename T, std::enable_if_t<std::is_void_v<T>, int> = 0>
	future<T> make_empty_future();
}

#include "future.ipp"