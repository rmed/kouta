#pragma once

#include <functional>
#include <memory>

namespace kouta::async
{
    /// @brief Wrapper for the I/O context.
    ///
    /// @details
    /// This class wraps the asio I/O context so that the public API does not require asio to compile. In addition, it
    /// offers a series of methods for common asynchronous operations.
    ///
    /// Note, however, that the preferred way of interacting with the context is via a @ref Component subclass.
    class Context
    {
    public:
        /// @brief Type of function that can be posted to the context
        using PostableHandler = std::function<void()>;

        // Default constructor
        Context();

        // Not copyable
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        // Not movable
        Context(Context&&) = delete;
        Context& operator=(Context&&) = delete;

        /// @brief Destructor.
        ~Context();

        /// @brief Run the event loop.
        ///
        /// @details
        /// Begins operation of the internal event loop and should only be called from a top-level element of the
        /// architecture (see @ref Root, for instance).
        ///
        /// @note This method blocks until the event loop is terminated.
        void run();

        /// @brief Stop the event loop and exit.
        ///
        /// @details
        /// The internal event loop is notified and remaining work discarded. Under normal circumstances, this would
        /// only be called when terminating the application or a specific secondary context (see @ref Branch, for
        /// instance).
        void stop();

        /// @brief Post a callable handler to the event loop.
        ///
        /// @details
        /// The callable will be invoked once execution returns to the event loop.
        ///
        /// @warning
        /// Although posting is thread-safe, the invokation of @p handler will be made from the thread on which the
        /// context resides. It is up to the caller to make sure that no unsafe inter-thread data access occurs.
        ///
        /// @param[in] handler          Handler to schedule for invocation.
        /// @{
        void post(const PostableHandler& handler);
        void post(PostableHandler&& handler);
        /// @}

    private:
        /// @brief Private implementation of the context.
        struct Impl;

        std::unique_ptr<Impl> m_impl;

        // Friends allowed access the internal details
        friend class Timer;
    };
}  // namespace kouta::async
