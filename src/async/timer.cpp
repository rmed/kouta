#include "kouta/async/timer.hpp"

#include <asio/any_io_executor.hpp>
#include <asio/steady_timer.hpp>

#include "detail/context_p.hpp"

namespace kouta::async
{
    // Private implementation
    struct Timer::Impl
    {
        /// @brief Custom constructor to handle the explicit constructor of @ref asio::steady_timer
        Impl(const asio::any_io_executor& executor)
            : asio_timer(executor)
        {
        }

        asio::steady_timer asio_timer;
    };

    Timer::Timer(Component* parent, std::chrono::milliseconds duration, OnExpired&& on_expired)
        : Component{parent}
        , m_impl{std::make_unique<Timer::Impl>(context().m_impl->asio_context.get_executor())}
        , m_duration{duration}
        , m_on_expired(on_expired)
    {
    }

    // Destructor must be specified here due to the incomplete type of the Impl struct.
    Timer::~Timer() = default;

    void Timer::start()
    {
        // Timer is stopped in case it was already running
        stop();

        m_impl->asio_timer.expires_after(m_duration);
        m_impl->asio_timer.async_wait(
            [this](const asio::error_code& ec)
            {
                // Only invoke callback if the timer expired normally
                if (ec != asio::error::operation_aborted)
                {
                    m_on_expired(*this);
                }
            });
    }

    void Timer::stop()
    {
        m_impl->asio_timer.cancel();
    }

    void Timer::set_duration(const std::chrono::milliseconds& duration)
    {
        m_duration = duration;
    }
}  // namespace kouta::async
