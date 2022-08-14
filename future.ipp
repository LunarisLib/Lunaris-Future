#pragma once

#include "future.h"

namespace Lunaris {

	namespace detail_future {

		template<typename T>
		inline void _tunnel<T>::set(T val)
		{
			std::lock_guard<std::mutex> l(m_safety);
			m_get(std::move(val));
			m_store_set = true;
		}

		template<typename T>
		inline void _tunnel<T>::redirect_nolock(std::function<void(T)> f)
		{
			m_get = f;
			m_redir_enabled = true;
		}

		template<typename T>
		inline void _tunnel<T>::wait() const
		{
			std::unique_lock<std::mutex> l(m_safety);
			while (!m_cond.wait_for(l, std::chrono::milliseconds(100), [this] { return m_store_set || m_redir_enabled; }));
			if (m_redir_enabled) throw std::runtime_error("future get_future() was already called");
		}

		template<typename T>
		template<typename _Rep, typename _Period>
		inline void _tunnel<T>::wait_for(const std::chrono::duration<_Rep, _Period> dt) const
		{
			std::unique_lock<std::mutex> l(m_safety);
			m_cond.wait_for(l, dt, [this] { return m_store_set || m_redir_enabled; });
			if (m_redir_enabled) throw std::runtime_error("future get_future() was already called");
		}

		inline void _tunnel<void>::set()
		{
			std::lock_guard<std::mutex> l(m_safety);
			m_get();
			m_store_set = true;
		}

		inline void _tunnel<void>::redirect_nolock(std::function<void()> f)
		{
			m_get = f;
			m_redir_enabled = true;
		}

		inline void _tunnel<void>::wait() const
		{
			std::unique_lock<std::mutex> l(m_safety);
			while (!m_cond.wait_for(l, std::chrono::milliseconds(50), [this] { return m_store_set || m_redir_enabled; }));
			if (m_redir_enabled) throw std::runtime_error("future get_future() was already called");
		}

		template<typename _Rep, typename _Period>
		inline void _tunnel<void>::wait_for(const std::chrono::duration<_Rep, _Period> dt) const
		{
			std::unique_lock<std::mutex> l(m_safety);
			m_cond.wait_for(l, dt, [this] { return m_store_set || m_redir_enabled; });
			if (m_redir_enabled) throw std::runtime_error("future get_future() was already called");
		}

	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	inline future<T> future<T>::get_future()
	{
		std::lock_guard<std::mutex> l(m_next->m_safety);
		if (m_next->m_redir_enabled) throw std::runtime_error("future get_future() was already called");
		future<T> next;
		m_next->redirect_nolock([next_data = next.m_next](T val) { next_data->set(std::move(val)); });
		return next;
	}

	template<typename T>
	template<typename Q, std::enable_if_t< std::is_void_v<Q>, int>>
	inline future<T> future<T>::get_future()
	{
		std::lock_guard<std::mutex> l(m_next->m_safety);
		if (m_next->m_redir_enabled) throw std::runtime_error("future get_future() was already called");
		future<T> next;
		m_next->redirect_nolock([next_data = next.m_next]() { next_data->set(); });
		return next;
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	inline void future<T>::set_value(Q val)
	{
		m_next->set(std::move(val));
	}

	template<typename T>
	template<typename Q, std::enable_if_t< std::is_void_v<Q>, int>>
	inline void future<T>::set_value()
	{
		m_next->set();
	}

	template<typename T>
	inline future<T>::future(future<T>&& e) noexcept
	{
		std::lock_guard<std::mutex> l(e.m_next->m_safety);
		m_next = std::move(e.m_next);
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	inline typename future<T>::cr_T future<T>::get() const
	{
		m_next->wait(); // already checks for redir
		return m_next->m_store;
	}

	template<typename T>
	template<typename Q, std::enable_if_t< std::is_void_v<Q>, int>>
	inline void future<T>::get() const
	{
		m_next->wait(); // already checks for redir
	}

	template<typename T>
	inline bool future<T>::wait() const
	{
		m_next->wait();
		return m_next->m_store_set;
	}

	template<typename T>
	template<typename _Rep, typename _Period>
	inline bool future<T>::wait(const std::chrono::duration<_Rep, _Period> dt) const
	{
		if (dt.count() > 0) m_next->wait_for(dt);
		return m_next->m_store_set;
	}

	template<typename T>
	template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
	inline T future<T>::get_take() const
	{
		m_next->wait(); // already checks for redir
		m_next->m_store_set = false;
		return std::move(m_next->m_store);
	}

	template<typename T>
	template<typename Q, std::enable_if_t< std::is_void_v<Q>, int>>
	inline void future<T>::get_take() const
	{
		m_next->wait(); // already checks for redir
		m_next->m_store_set = false;
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t<!std::is_void_v<Q>, int>, typename Res, std::enable_if_t<!std::is_void_v<Res>, int>>
	// Q != null, Res != null
	inline auto future<T>::then(V func) // func gets Q and return Res
	{
		future<Res> nxt;
		std::lock_guard<std::mutex> l(m_next->m_safety);
		m_next->redirect_nolock([mnxt = nxt.m_next, func](Q got) { mnxt->set(func(got)); });
		return nxt;
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t<!std::is_void_v<Q>, int>, typename Res, std::enable_if_t< std::is_void_v<Res>, int>>
	// Q != null, Res == null
	inline auto future<T>::then(V func) // func gets Q and return Res
	{
		future<Res> nxt;
		std::lock_guard<std::mutex> l(m_next->m_safety);
		m_next->redirect_nolock([mnxt = nxt.m_next, func](Q got) { func(got); mnxt->set(); });
		return nxt;
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t< std::is_void_v<Q>, int>, typename Res, std::enable_if_t<!std::is_void_v<Res>, int>>
	// Q == null, Res != null
	inline auto future<T>::then(V func) // func gets Q and return Res
	{
		future<Res> nxt;
		std::lock_guard<std::mutex> l(m_next->m_safety);
		m_next->redirect_nolock([mnxt = nxt.m_next, func]() { mnxt->set(func()); });
		return nxt;
	}

	template<typename T>
	template<typename V, typename Q, std::enable_if_t< std::is_void_v<Q>, int>, typename Res, std::enable_if_t< std::is_void_v<Res>, int>>
	// Q == null, Res == null
	inline auto future<T>::then(V func) // func gets Q and return Res
	{
		future<Res> nxt;
		std::lock_guard<std::mutex> l(m_next->m_safety);
		m_next->redirect_nolock([mnxt = nxt.m_next, func]() { func(); mnxt->set(); });
		return nxt;
	}

	template<typename T, std::enable_if_t<!std::is_void_v<T>, int>>
	inline future<T> make_empty_future(const T& v)
	{
		promise<T> _p;
		auto _f = _p.get_future();
		_p.set_value(v);
		return _f;
	}

	template<typename T, std::enable_if_t<std::is_void_v<T>, int>>
	inline future<T> make_empty_future()
	{
		promise<T> _p;
		auto _f = _p.get_future();
		_p.set_value();
		return _f;
	}
}