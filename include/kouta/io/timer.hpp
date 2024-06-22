#pragma once

#include <chrono>
#include <functional>

#include <boost/asio/steady_timer.hpp>

#include <kouta/base.hpp>

namespace kouta::io
{
    class Timer : public base::Component
    {
    public:
        /// Signature of the function to be called when the timer expires or is cancelled.
        using OnExpired = std::function<void(Timer&)>;

        // Not default-constructible.
        Timer() = delete;

        /// @brief Constructor.
        ///
        /// @param[in] parent           Parent component granting access to the event loop.
        /// @param[in] duration         Duration of the timer.
        /// @param[in] on_expired       Function to call when the timer expires.
        Timer(Component* parent, std::chrono::milliseconds duration, const OnExpired& on_expired)
            : Component{parent}
            , m_timer{context()}
            , m_duration{duration}
            , m_on_expired(on_expired)
        {
        }

        // Not copyable
        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;

        // Not movable
        Timer(Timer&&) = delete;
        Timer& operator=(Timer&&) = delete;

        ~Timer() override = default;

        /// @brief Start the timer and wait for it to complete asynchronously.
        ///
        /// @details
        /// The timer will be automatically stopped if it was already running (which could be seen as restarting the
        /// timer). The duration used for awaiting the timer is the one already set inside this object.
        ///
        /// @note This is a one-shot waiting operation.
        void start()
        {
            // Timer is stopped in case it was already running
            stop();

            m_timer.expires_after(m_duration);
            m_timer.async_wait(std::bind_front(&Timer::handle_expiration, this));
        }

        /// @brief Stop the timer if it was running/being waited for.
        void stop()
        {
            m_timer.cancel();
        }

        /// @brief Set the duration of the timer in future waiting operations.
        ///
        /// @details
        /// The purpose of this method is to allow setting the timer duration without actually starting it, as opposed
        /// to @ref start(). However, this method will **not** affect any running timer awaits, meaning that @ref stop()
        /// must be called explicitly if such abehaviour is required.
        ///
        /// @param[in] duration         New duration for the timer.
        void set_duration(std::chrono::milliseconds duration)
        {
            m_duration = duration;
        }

    private:
        /// @brief Handle the expiration of the internal timer.
        ///
        /// @details
        /// If the timer expired normally, the internal callback will be executed to notify the external world. Note
        /// that said callback will be executed within the context of the event loop as a direct invocation.
        ///
        /// @param[in] ec       Error code of the asynchronous wait operation.
        void handle_expiration(const boost::system::error_code& ec)
        {
            if (ec != boost::asio::error::operation_aborted)
            {
                m_on_expired(*this);
            }
        }

        boost::asio::steady_timer m_timer;
        std::chrono::milliseconds m_duration;
        OnExpired m_on_expired;
    };
}  // namespace kouta::io
