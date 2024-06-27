#pragma once

#include <kouta/base/component.hpp>

namespace kouta::base
{
    /// @brief Root component.
    ///
    /// @details
    /// As opposed to a regular @ref Component, the Root does own the event loop and is in charge of
    /// running it and acting as the entry-point to the rest of the application.
    class Root : public Component
    {
    public:
        Root()
            : Component{nullptr}
            , m_context{}
        {
        }

        // Not copyable
        Root(const Root&) = delete;
        Root& operator=(const Root&) = delete;

        // Not movable
        Root(Root&&) = delete;
        Root& operator=(Root&&) = delete;

        virtual ~Root() = default;

        /// @brief Obtain a reference to the underlying I/O context.
        ///
        /// @note The I/O context is owned by the root.
        boost::asio::io_context& context() override
        {
            return m_context;
        }

        /// @brief Run the event loop.
        ///
        /// @note This method blocks until the event loop is terminated.
        virtual void run()
        {
            // Have the event loop run forever
            auto work_guard{boost::asio::make_work_guard(m_context)};
            m_context.run();
        }

        /// @brief Stop the event loop and exit.
        ///
        /// @note Under normal circumstances, this would only be called when terminating the application.
        virtual void stop()
        {
            m_context.stop();
        }

    private:
        boost::asio::io_context m_context;
    };
}  // namespace kouta::base
