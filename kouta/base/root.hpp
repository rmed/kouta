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
        Root();

        // Not copyable
        Root(const Root&) = delete;
        Root& operator=(const Root&) = delete;

        // Not movable
        Root(Root&&) = delete;
        Root& operator=(Root&&) = delete;

        virtual ~Root() = default;

        /// @brief Provide the I/O context owned by the Root.
        boost::asio::io_context& context() override;

        /// @brief Run the event loop.
        ///
        /// @note This method blocks until the event loop is terminated.
        virtual void run();

        /// @brief Stop the event loop and exit.
        ///
        /// @note Under normal circumstances, this would only be called when terminating the application.
        virtual void stop();

    private:
        boost::asio::io_context m_context;
    };
}  // namespace kouta::base
