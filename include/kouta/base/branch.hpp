#pragma once

#include <thread>

#include <kouta/base/component.hpp>
#include <kouta/base/root.hpp>

namespace kouta::base
{
    /// @brief Background component executor.
    ///
    /// @details
    /// A Branch wraps a @ref Component based object so that the event loop it is attached to is executed in a
    /// separate thread. This implies that any children that have the wrapped Component as a parent will also run in
    /// the worker thread.
    ///
    /// By default, the Branch does nothing, and its event loop must be explicitly started by calling the @ref run()
    /// method. As opposed to the original method defined in @ref Root, the one specified here will launch the thread
    /// (and the event loop) and return immediately.
    ///
    /// @tparam TWrapped            Wrapped @ref Component type. IT is assumed that the first argument of the component
    ///                             will be a pointer to a parent component (which will be set to this Branch).
    template<class TWrapped>
        requires std::is_base_of_v<Component, TWrapped>
    class Branch : public Root
    {
    public:
        using WrappedComponent = TWrapped;

        // Not default-constructible.
        Branch() = delete;

        /// @brief Branch component constructor.
        ///
        /// @details
        /// This constructor will register the Branch object with the parent **only to manage the memory deallocation**
        /// in case the object was allocated on the heap. Regardless of having a parent, the Branch owns its event loop.
        ///
        /// @tparam TArgs               Types of the arguments to provide the wrapped component.
        ///
        /// @param[in] parent           Parent component. The lifetime of the parent must surpass that of the
        ///                             child.
        /// @param[in] args             Arguments to provide the wrapped component.
        template<class... TArgs>
        Branch(Component* parent, TArgs... args)
            : Root{parent}
            , m_worker{}
            , m_component{this, args...}  // Assuming first argument is the parent component
        {
        }

        // Not copyable
        Branch(const Branch&) = delete;
        Branch& operator=(const Branch&) = delete;

        // Not movable
        Branch(Root&&) = delete;
        Branch& operator=(Branch&&) = delete;

        /// @brief Branch destructor.
        ///
        /// @details
        /// The destructor takes care of the cleanup of the worker thread, by stopping its event loop and waiting
        /// for the thread to terminate before joining it.
        virtual ~Branch()
        {
            if (m_worker.joinable())
            {
                stop();

                m_worker.join();
            }
        }

        /// @brief Obtain a constant reference to the wrapped component.
        const WrappedComponent& component() const
        {
            return m_component;
        }

        /// @brief Obtain a mutable reference to the wrapped component.
        WrappedComponent& component()
        {
            return m_component;
        }

        /// @brief Run the event loop in the worker thread.
        ///
        /// @note Calling this method several times has no effect.
        /// @note The worker thread blocks until the event loop is terminated.
        void run() override
        {
            // Can only run the thread once
            if (!m_worker.joinable())
            {
                m_worker = std::thread{&Branch<WrappedComponent>::run_worker, this};
            }
        }

        /// @brief Post a wrapped component method call to the event loop for deferred execution.
        ///
        /// @details
        /// This is a pass-through to the @ref Component::post() method of the wrapped component.
        ///
        /// @warning Arguments are **copied** before being passed to the event loop.
        ///
        /// @tparam TArgs               Types of the arguments that the method accepts.
        /// @tparam TMethodArgs         Types of the arguments that the method accepts.
        /// @tparam TArgs               Types of the arguments provided to the invocation.
        ///
        /// @param[in] method           Method to execute. Its signature must match `void(TArgs...)`
        /// @param[in] args             Arguments to execute the method with.
        template<class... TMethodArgs, class... TArgs>
        void post(void (WrappedComponent::*method)(TMethodArgs...), TArgs... args)
        {
            m_component.post(method, args...);
        }

        /// @brief Inherit @ref Root::post() to allow posting events to the branch itself.
        ///
        /// @note This may be used to post a call to the @ref stop() method.
        using Root::post;

    private:
        /// @brief Run the event loop.
        ///
        /// @note This method blocks until the event loop is terminated.
        void run_worker()
        {
            auto work_guard{asio::make_work_guard(context())};
            context().run();
        }

        std::thread m_worker;
        WrappedComponent m_component;
    };
}  // namespace kouta::base
