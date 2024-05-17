#include <kouta/io/timer.hpp>

namespace kouta::io
{
    Timer::Timer(Component* parent, std::chrono::milliseconds duration, const OnExpired& on_expired)
        : Component{parent}
        , m_timer{context()}
        , m_duration{duration}
        , m_on_expired(on_expired)
    {
    }

    void Timer::start()
    {
        // Timer is stopped in case it was already running
        stop();

        m_timer.expires_after(m_duration);
        m_timer.async_wait(std::bind_front(&Timer::handle_expiration, this));
    }

    void Timer::stop()
    {
        m_timer.cancel();
    }

    void Timer::set_duration(std::chrono::milliseconds duration)
    {
        m_duration = duration;
    }

    void Timer::handle_expiration(const boost::system::error_code& ec)
    {
        if (ec != boost::asio::error::operation_aborted)
        {
            m_on_expired(*this);
        }
    }
}  // namespace kouta::io
