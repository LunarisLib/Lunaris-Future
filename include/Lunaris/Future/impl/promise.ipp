namespace Lunaris {
namespace Future {

    template<typename T>
    template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
    inline void Promise<T>::set() {
        m_fut.m_pipe->set(true);
    }

    template<typename T>
    template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
    inline void Promise<T>::set(Q val) {
        m_fut.m_pipe->set(std::move(val));
    }

    template<typename T>
    inline Future<T> Promise<T>::get_future() {
        Future<T> next;
        m_fut.m_pipe->make_callback([next_ref = next.m_pipe](StoreType<T>&& val) {
            next_ref->set(std::move(val));
        });
        return next;
    }


	template<typename T, std::enable_if_t<!std::is_void_v<T>, int>>
	inline Future<T> make_empty_future(T&& value) {
        Promise<T> prom;
        Future<T> fut = prom.get_future();
        prom.set(std::move(value));
        return fut;
    }

	template<typename T, std::enable_if_t<std::is_void_v<T>, int>>
	inline Future<void> make_empty_future() {
        Promise<void> prom;
        Future<void> fut = prom.get_future();
        prom.set();
        return fut;
    }

} // namespace Future
} // namespace Lunaris