#pragma once

#include "component.hpp"

namespace kouta::async
{
    /// @brief Root component.
    ///
    /// @details
    /// As opposed to a regular @ref Component, the Root does own the I/O context and is in charge of running the event
    /// loop and acting as the entry-point to the rest of the application.
    ///
    /// @note
    /// There is usually a single Root per application, as running it blocks the thread. For multi-threaded/context
    /// use-cases, @see Branch.
    class Root : public Component
    {
    public:
        /// @brief Default constructor.
        ///
        /// @details
        /// This constructor assumes that the Root object will not have a parent (e.g. it is the main object of the
        /// tree), meaning that it will not attempt to register itself with the parent, nor remove itself from its list
        /// of children when being destroyed.
        Root();

        /// @brief Construct from a parent.
        ///
        /// @details
        /// This constructor will register the Root object with the parent **only to manage the memory deallocation** in
        /// case the object was allocated on the heap. Regardless of having a parent, the Root owns its event loop.
        explicit Root(Component* parent);

        // Not copyable
        Root(const Root&) = delete;
        Root& operator=(const Root&) = delete;

        // Not movable
        Root(Root&&) = delete;
        Root& operator=(Root&&) = delete;

        ~Root() override = default;

        /// @brief Obtain a reference to the underlying I/O context.
        ///
        /// @note The I/O context is owned by the root.
        Context& context() override;

        /// @brief Run the event loop.
        ///
        /// @note Internally, this calls @ref Context::run();
        virtual void run();

        /// @brief Stop the event loop and exit.
        ///
        /// @note Internally, this calls @ref Context::stop();
        virtual void stop();

    private:
        Context m_context;
    };
}  // namespace kouta::async
