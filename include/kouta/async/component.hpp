#pragma once

#include <set>

#include "context.hpp"

namespace kouta::async
{
    /// @brief Base class for asynchronous components.
    ///
    /// @details
    /// A component provides access to the underlying event loop which, by default, belongs to the parent component.
    /// Moreover, specifying a parent will add the component to its children list and make sure that the component is
    /// deleted when the parent is destroyed (if the component was allocated on the heap).
    class Component
    {
    public:
        // Not default-constructible.
        Component() = delete;

        /// @brief Constructor.
        ///
        /// @param[in] parent           Parent component. The lifetime of the parent must surpass that of the child.
        explicit Component(Component* parent);

        // Not copyable
        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        // Not movable
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;

        /// @brief Component destructor.
        ///
        /// @details
        /// The component will go through its list of children and delete them one at a time. This is only useful if the
        /// components were allocated in the heap, as stack-allocated ones will probably have been deleted automatically
        /// prior to calling this destructor.
        ///
        /// In addition, once a component has deleted its children, it will remove itself from its parent.
        ///
        /// @note Child deletion happens in reverse order.
        virtual ~Component();

        /// @brief Obtain a reference to the underlying I/O context.
        ///
        /// @note By default, this I/O context comes from the parent component.
        virtual Context& context();

        /// @brief Post a method call to the event loop for deferred execution.
        ///
        /// @details
        /// This allows other components, even those residing in another thread/event loop, to post a method call to
        /// this specific component.
        ///
        /// @warning Arguments are **copied** before being passed to the event loop.
        ///
        /// @tparam TClass              Child class whose method is going to be invoked.
        /// @tparam TMethodArgs         Types of the arguments that the method accepts.
        /// @tparam TArgs               Types of the arguments provided to the invocation.
        ///
        /// @param[in] method           Method to invoke. Its signature must match `void(TArgs...)`
        /// @param[in] args             Arguments to invoke the method with.
        template<class TClass, class... TMethodArgs, class... TArgs>
        void post(void (TClass::*method)(TMethodArgs...), TArgs... args)
        {
            context().post(
                [this, method, args...]()
                {
                    (static_cast<TClass*>(this)->*method)(std::move(args)...);
                });
        }

        /// @brief Post a function call to the event loop for deferred execution.
        ///
        /// @details
        /// This allows other components, even those residing in another thread/event loop, to post a function call to
        /// this specific component.
        ///
        /// @warning Arguments are **copied** before being passed to the event loop.
        ///
        /// @tparam TFuncArgs           Types of the arguments that the function accepts.
        /// @tparam TArgs               Types of the arguments provided to the invocation.
        ///
        /// @param[in] callable         Callable to invoke. Its signature must match `void(TArgs...)`
        /// @param[in] args             Arguments to pass to the callable.
        /// @{
        template<class... TFuncArgs, class... TArgs>
        void post(const std::function<void(TFuncArgs...)>& callable, TArgs... args)
        {
            context().post(
                [callable = std::move(callable), args...]()
                {
                    callable(std::move(args)...);
                });
        }

        template<class... TFuncArgs, class... TArgs>
        void post(std::function<void(TFuncArgs...)>&& callable, TArgs... args)
        {
            context().post(
                [callable = std::move(callable), args...]()
                {
                    callable(std::move(args)...);
                });
        }
        /// @}

        /// @brief Post a handler invocation to the event loop for deferred execution.
        ///
        /// @see Context::post().
        ///
        /// @param[in] handler          Handler to schedule for invocation.
        void post(Context::PostableHandler&& handler);

    private:
        /// @brief Add a child component to the list.
        ///
        /// @details
        /// This is used to keep track of objects to delete when the component has been allocated in the heap.
        ///
        /// @note Normally, this will only be called from the Constructor of the component.
        ///
        /// @param[in] component            Pointer to the component to add.
        void add_child(Component* component);

        /// @brief Remove a child component from the list.
        ///
        /// @details
        /// When a child is removed, the parent component will not attempt to delete it itself. Note that when a
        /// component allocated in the stack is destroyed, it will remove itself from the list and prevent
        /// double-free issues.
        void remove_child(Component* component);

        Component* m_parent;
        std::set<Component*> m_children;
    };
}  // namespace kouta::async
