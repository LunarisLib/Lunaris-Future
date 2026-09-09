namespace Lunaris {
namespace Future {

    template<typename T>
    inline pipe<T>::pipe() 
        : m_redirect([this](BaseType&& assign) { m_value.emplace(std::move(assign)); })
    {}

    template<typename T>
    template<typename Function>
    inline void pipe<T>::make_callback(Function&& callback) {
        {
            std::unique_lock<std::mutex> l(m_mtx);
            m_redirect = std::move(callback);
            m_redirected = true;

            if (m_value.has_value())
                m_redirect(std::move(*std::move(m_value)));
        }

        m_cond.notify_all();
    }

    template<typename T>
    inline void pipe<T>::set(BaseType&& move) {
        {
            std::unique_lock<std::mutex> l(m_mtx);
            m_redirect(std::move(move));
        }
        m_cond.notify_all();
    }

    template<typename T>
    inline bool pipe<T>::can_get() {
        return m_value.has_value();
    }
    
    template<typename T>
    e_wait_status pipe<T>::wait() {
        std::unique_lock<std::mutex> l(m_mtx);

        const bool last_ret = m_cond.wait(l,
            [this]{ return m_redirected || this->can_get(); }
        );
        
        if (m_redirected)    return e_wait_status::VALUE_TUNNELED;
        if (this->can_get()) return e_wait_status::VALUE_SET;
        return e_wait_status::VALUE_UNSET_TIMEOUT;
    }

    template<typename T>
    template<typename Rep, typename Period>
    e_wait_status pipe<T>::wait(const std::chrono::duration<Rep, Period>& wait_for) {
        std::unique_lock<std::mutex> l(m_mtx);

        const bool last_ret = m_cond.wait_for(l,
            wait_for,
            [this]{ return m_redirected || this->can_get(); }
        );

        if (m_redirected)    return e_wait_status::VALUE_TUNNELED;
        if (this->can_get()) return e_wait_status::VALUE_SET;
        return e_wait_status::VALUE_UNSET_TIMEOUT;
    }

    template<typename T>
    inline pipe<T>::BaseType pipe<T>::get() {
        std::unique_lock<std::mutex> l(m_mtx);

        m_cond.wait(l, [this]{ return m_redirected || this->can_get(); });

        if (m_redirected || !this->can_get()) 
            throw FutureException("Object has used redirect or is not valid!");

        BaseType taken = std::move(*std::move(m_value));
        m_value.reset();
        return taken;
    }

} // namespace Future
} // namespace Lunaris