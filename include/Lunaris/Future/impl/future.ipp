namespace Lunaris {
namespace Future {

    template<typename T>
    inline Future<T>::Future()
        : m_pipe(std::make_shared<pipe<T>>())
    {}

    template<typename T>
    inline Future<T>::Future(Future&& oth) noexcept
        : m_pipe(std::move(oth.m_pipe))
    {}

    template<typename T>
    inline void Future<T>::operator=(Future&& oth) noexcept {
        m_pipe = std::move(oth.m_pipe);
    }

    template<typename T>
    template<typename Q, std::enable_if_t<std::is_void_v<Q>, int>>
    inline void Future<T>::get() {
        if (m_pipe->get() != true) throw 1;
    }

    template<typename T>
    template<typename Q, std::enable_if_t<!std::is_void_v<Q>, int>>
    inline T Future<T>::get() {
        return m_pipe->get();
    }

    template<typename T>
    e_wait_status Future<T>::wait() {
        return m_pipe->wait();
    }

    template<typename T>
    template<typename Rep, typename Period>
    e_wait_status Future<T>::wait(const std::chrono::duration<Rep, Period>& wait_for) {
        return m_pipe->wait(wait_for);
    }

    template<typename T>
    template<typename Function>
    inline auto Future<T>::then(Function&& callback) {
        using ReturnType = std::invoke_result_t<Function, T>;
        Future<ReturnType> next;

        // make this m_pipe redirect result to next
        m_pipe->make_callback([next_ref = next.m_pipe, cb = std::move(callback)](StoreType<T>&& val){
            if constexpr (std::is_void_v<ReturnType>) {
                cb(std::move(val));
                next_ref->set(true);
            }
            else {
                next_ref->set(cb(std::move(val)));
            }
        });

        return next;
    }

} // namespace Future
} // namespace Lunaris