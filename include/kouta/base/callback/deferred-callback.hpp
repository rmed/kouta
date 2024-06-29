#pragma once

#include <kouta/base/callback/base-callback.hpp>

namespace kouta::base::callback
{
    /// @brief Deferred Callback implementation.
    ///
    /// @details
    /// A deferred Callback wraps a callable that is invoked within the destination object's thread, assuming that
    /// said object implements a `post()` method which allows the Callback to deferr the call to its event loop.
    /// This is the case for the @ref Component based architecture of this library.
    ///
    /// Because calling a deferred Callback posts an event to the destination's thread, **arguments passed to the
    /// callback must be copied during the call**, meaning that the following types of arguments should not be used
    /// (or used with much care to prevent invalid memory access or race conditions):
    ///
    /// - Raw pointers
    /// - Non-copyable objects
    /// - Objects referencing, but not owning, dynamic memory (e.g. @ref std::span)
    ///
    /// The lifetime of the object the Callback points to must be guaranteed to surpass that of the Callback itself.
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class DeferredCallback : public BaseCallback<TArgs...>
    {
    public:
        /// @brief The type of the callable the Callback points to.
        using Callable = BaseCallback<TArgs...>::Callable;

        // Copyable
        DeferredCallback(const DeferredCallback&) = default;
        DeferredCallback& operator=(const DeferredCallback&) = default;

        // Movable
        DeferredCallback(DeferredCallback&&) = default;
        DeferredCallback& operator=(DeferredCallback&&) = default;

        ~DeferredCallback() override = default;

        /// @brief Callback constructor from a bound method.
        ///
        /// @details
        /// This constructor initializes the internal callable with a pointer to the specified bound @p method.
        /// The developer must guarantee the lifetime of the @p object to prevent invalid memory access.
        ///
        /// @p object must implement a `post()` method to which the invokation can be passed to defer execution.
        ///
        /// @tparam TClass              Object type.
        ///
        /// @param[in] object           Pointer to the object whose method is going to be called.
        /// @param[in] method           Pointer to the method that is going to be called. The arguments of the Callback
        ///                             must match those of this method.
        template<class TClass>
        DeferredCallback(TClass* object, void (TClass::*method)(TArgs...))
            : BaseCallback<TArgs...>{}
        {
            this->set_callable(
                [object, method](TArgs... args)
                {
                    object->template post<TClass, TArgs...>(method, args...);
                });
        }

        /// @brief Callback constructor from a callable.
        ///
        /// @details
        /// This constructor initializes the intenal callable to the one specified in @p callable , so that it is
        /// invoked from within the context of the provided @p object , which must implement a `post()` method to which
        /// the invokation can be passed to defer execution.
        ///
        /// The developer must guarantee the lifetime of the @p object to rpevent invalid memory access.
        ///
        /// @tparam TClass              Object type.
        ///
        /// @param[in] object           Pointer to the object whose method is going to be called.
        /// @param[in] callable         Callable to store. For instance, this could be a lambda or anything convertible
        ///                             to `std::function`.
        template<class TClass>
        DeferredCallback(TClass* object, const DeferredCallback::Callable& callable)
            : BaseCallback<TArgs...>{}
        {
            this->set_callable(
                [object, callable](TArgs... args)
                {
                    object->template post<TArgs...>(callable, args...);
                });
        }
    };
}  // namespace kouta::base::callback
