#pragma once

#include <boost/asio.hpp>

#include <kouta/base/callback.hpp>

namespace kouta::base
{
    /// @brief Base class for asynchronous components.
    ///
    /// @details
    /// A component provies access to the underlying event loop which, by default, belongs to the parent component.
    class Component
    {
    public:
        // Not default-constructible.
        Component() = delete;

        /// @brief Constructor.
        ///
        /// @param[in] parent           Parent component. Note that the parent provides access to the event loop,
        ///                             hence its lifetime must, at least, surpass that of the child.
        explicit Component(Component* parent = nullptr)
            : m_parent{parent}
        {}

        // Not copyable
        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        // Not movable
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;

        virtual ~Component() = default;

        /// @brief Obtain a reference to the underlying I/O context.
        ///
        /// @note By default, this I/O context comes from the parent component.
        virtual boost::asio::io_context& context()
        {
            return m_parent->context();
        }

        /// @brief Post a method call to the event loop for deferred execution.
        ///
        /// @details
        /// This allows other components, even those residing in another thread/event loop, to post a call to this
        /// specific component.
        ///
        /// @warning Arguments are **copied** before being passed to the event loop.
        ///
        /// @tparam TClass              Child class whose method is going to be executed.
        /// @tparam TMethodArgs         Types of the arguments that the method accepts.
        /// @tparam TArgs               Types of the arguments provided to the invocation.
        ///
        /// @param[in] method           Method to execute. Its signature must match `void(TArgs...)`
        /// @param[in] args             Arguments to execute the method with.
        template<class TClass, class... TMethodArgs, class... TArgs>
        void post(void (TClass::*method)(TMethodArgs...), TArgs... args)
        {
            boost::asio::post(
                context().get_executor(),
                [this, method, args...]()
                {
                    (static_cast<TClass*>(this)->*method)(std::move(args)...);
                });
        }

    private:
        Component* m_parent;
    };
}  // namespace kouta::base
