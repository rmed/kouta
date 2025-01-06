#pragma once

#include <kouta/base/callback/base-callback.hpp>

namespace kouta::base::callback
{
    /// @brief Direct Callback implementation.
    ///
    /// @details
    /// A direct Callback wraps a callable that is invoked within the caller's thread, making it no different to a
    /// direct invokation of a specific function or method.
    ///
    /// The lifetime of the object the Callback points to must be guaranteed to surpass that of the Callback itself.
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class DirectCallback : public BaseCallback<TArgs...>
    {
    public:
        /// @brief The type of the callable the Callback points to.
        using Callable = BaseCallback<TArgs...>::Callable;

        // Copyable
        DirectCallback(const DirectCallback&) = default;
        DirectCallback& operator=(const DirectCallback&) = default;

        // Movable
        DirectCallback(DirectCallback&&) = default;
        DirectCallback& operator=(DirectCallback&&) = default;

        ~DirectCallback() override = default;

        /// @brief Callback constructor from a bound method.
        ///
        /// @details
        /// This constructor initializes the internal callable with a pointer to the specified bound @p method.
        /// The developer must guarantee the lifetime of the @p object to prevent invalid memory access.
        ///
        /// @tparam TClass              Object type.
        ///
        /// @param[in] object           Pointer to the object whose method is going to be called.
        /// @param[in] method           Pointer to the method that is going to be called. The arguments of the Callback
        ///                             must match those of this method.
        template<class TClass>
        DirectCallback(TClass* object, void (TClass::*method)(TArgs...))
            : BaseCallback<TArgs...>{}
        {
            this->set_callable(std::bind_front(method, object));
        }

        /// @brief Callback constructor from a callable.
        ///
        /// @details
        /// This constructor initializes the internal callable to the one specified in @p callable , so that it is
        /// invoked from within the current context (i.e. as a regular function).
        ///
        /// @tparam TClass              Object type.
        ///
        /// @param[in] object           Pointer to the object whose method is going to be called.
        /// @param[in] callable         Callable to store. For instance, this could be a lambda or anything convertible
        ///                             to `std::function`.
        /// @{
        explicit DirectCallback(const DirectCallback::Callable& callable)
            : BaseCallback<TArgs...>{}
        {
            this->set_callable(callable);
        }

        explicit DirectCallback(DirectCallback::Callable&& callable)
            : BaseCallback<TArgs...>{}
        {
            this->set_callable(callable);
        }
        /// @}
    };
}  // namespace kouta::base::callback
