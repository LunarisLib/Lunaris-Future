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

    template<typename T>
    template<typename Q, std::enable_if_t<std::is_copy_constructible_v<Q> && std::is_copy_assignable_v<Q>, int>>
    std::vector<Future<T>> Promise<T>::get_multiple_future(const size_t amount) {
        std::vector<Future<T>> m_futures(amount);
        std::vector<std::shared_ptr<pipe<T>>> m_futures_pipes;
        for(auto& i : m_futures) m_futures_pipes.push_back(i.m_pipe);

        m_fut.m_pipe->make_callback([next_refs = m_futures_pipes](StoreType<T> val) {
            for(auto& next_ref : next_refs) {
                StoreType<T> copy = (const StoreType<T>&)val;
                next_ref->set(std::move(copy));
            }
        });
        return m_futures;
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