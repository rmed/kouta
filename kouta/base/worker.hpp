#pragma once

#include <thread>

#include <kouta/base/component.hpp>
#include <kouta/base/root.hpp>

namespace kouta::base
{
    /// @brief Background component executor.
    ///
    /// @details
    /// A Worker wraps a @ref Component based object so that the event loop it is attached to is executed in a
    /// separate thread. This implies that any children that have the wrapped Component as a parent will also run in
    /// the worker thread.
    ///
    /// By default, the Worker does nothing, and its event loop must be explicitly started by calling the @ref run()
    /// method. As opposed to the original method defined in @ref Root, the one specified here will launch the thread
    /// (and the event loop) and return immediately.
    ///
    /// @tparam TWrapped            Wrapped @ref Component type.
    template<class TWrapped>
        requires std::is_base_of_v<Component, TWrapped>
    class Worker : public Root
    {
    public:
        using WrappedComponent = TWrapped;

        // Not default-constructible.
        Worker() = delete;

        /// @brief Worker component constructor.
        ///
        /// @tparam TArgs           Types of the arguments to provide the wrapped component.
        ///
        /// @param[in] args         Arguments to provide the wrapped component.
        template<class... TArgs>
        Worker(TArgs... args)
            : Root{}
            , m_worker{}
            , m_component{args...}
        {
        }

        // Not copyable
        Worker(const Worker&) = delete;
        Worker& operator=(const Worker&) = delete;

        // Not movable
        Worker(Root&&) = delete;
        Worker& operator=(Worker&&) = delete;

        /// @brief Worker destructor.
        ///
        /// @details
        /// The destructor takes care of the cleanup of the worker thread, by stopping its event loop and waiting
        /// for the thread to terminate before joining it.
        virtual ~Worker()
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
                m_worker = std::thread{&Worker<WrappedComponent>::run_worker, this};
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

        /// @brief Inherit @ref Root::post() to allow posting events to the worker itself.
        ///
        /// @note This may be used to post a call to the @ref stop() method.
        using Root::post;

    private:
        /// @brief Run the event loop.
        ///
        /// @note This method blocks until the event loop is terminated.
        void run_worker()
        {
            auto work_guard{boost::asio::make_work_guard(context())};
            context().run();
        }

        std::thread m_worker;
        WrappedComponent m_component;
    };
}  // namespace kouta::base
