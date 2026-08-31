#pragma once

#include <chrono>
#include <functional>

#include "kouta/async/component.hpp"

namespace kouta::async
{
    /// @brief Timer implementation that can be awaited asynchronously.
    ///
    /// @details
    /// Once the timer expires, it will invoke the provided callback. Note, however, that this is a oneshot timer, so it
    /// **must be manually rearmed** for periodic executions.
    ///
    /// The duration of the timer is specified relative to the current system time (e.g. "100ms from now").
    class Timer : public Component
    {
    public:
        /// @brief Signature of the function to be invoked when the timer expires or is cancelled.
        using OnExpired = std::function<void(Timer&)>;

        // Not default-constructible.
        Timer() = delete;

        /// @brief Constructor.
        ///
        /// @param[in] parent           Parent component granting access to the event loop.
        /// @param[in] duration         Duration of the timer.
        /// @param[in] on_expired       Function to call when the timer expires.
        Timer(Component* parent, std::chrono::milliseconds duration, OnExpired&& on_expired);

        // Not copyable
        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;

        // Not movable
        Timer(Timer&&) = delete;
        Timer& operator=(Timer&&) = delete;

        ~Timer() override;

        /// @brief Start the timer and wait for it to complete asynchronously.
        ///
        /// @details
        /// The timer will be automatically stopped if it was already running (which could be seen as restarting the
        /// timer). The duration used for awaiting the timer is the one already set inside this object.
        ///
        /// If the timer expired normally, the internal callback will be invoked to notify the creator of the Timer.
        /// Note that said callback will be executed within event loop in which the Timer resides (as a direct
        /// invocation).
        ///
        /// @note This is a one-shot waiting operation.
        void start();

        /// @brief Stop the timer if it was running/being waited for.
        void stop();

        /// @brief Set the duration of the timer in future waiting operations.
        ///
        /// @details
        /// The purpose of this method is to allow setting the timer duration without actually starting it, as opposed
        /// to @ref start(). However, this method will **not** affect any running timer awaits, meaning that @ref stop()
        /// must be called explicitly if such behaviour is required.
        ///
        /// @param[in] duration         New duration for the timer.
        void set_duration(const std::chrono::milliseconds& duration);

    private:
        /// @brief Private implementation of the timer.
        struct Impl;

        std::unique_ptr<Impl> m_impl;
        std::chrono::milliseconds m_duration;
        OnExpired m_on_expired;
    };
}  // namespace kouta::async
