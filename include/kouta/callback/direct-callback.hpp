#pragma once

#include "abstract-callback.hpp"

namespace kouta::callback
{
    /// @brief Direct callback implementation.
    ///
    /// @details
    /// A direct Callback wraps a callable that is invoked as if it were a direct function call.
    ///
    /// Example usage:
    ///
    /// @code
    /// void free_func(int a, const std::string& b)
    /// {
    ///     std::cout << "Function: " << a << " " << b << std::endl;
    /// }
    ///
    /// class Cls
    /// {
    /// public:
    ///     void bound_method(int a, const std::string& b)
    ///     {
    ///         std::cout << "Method: " << a << " " << b << std::endl;
    ///     }
    /// };
    ///
    /// // Callback pointing to a free function
    /// DirectCallback<int, const std::string& cb{free_func};
    /// cb(42, "test");
    ///
    /// // Callback pointing to a bound method
    /// Cls obj{};
    ///
    /// DirectCallback<int, const std::string& cb2{&obj, &Cls::bound_method};
    /// cb2(42, "test");
    ///
    /// // Callback pointing to a lambda
    /// DirectCallback<int, const std::string& cb3{[](int a, const std::string& b) {
    ///     std::cout << "Lambda: " << a << " " << b << std::endl;
    /// }};
    /// cb3(42, "test");
    /// @endcode
    ///
    /// @warning
    /// When pointing to object methods, the lifetime of the object the Callback points to must be guaranteed to surpass
    /// that of the Callback itself.
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class DirectCallback : public AbstractCallback<TArgs...>
    {
    public:
        /// @brief The type of the callable the Callback points to.
        using Callable = typename AbstractCallback<TArgs...>::Callable;

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
            : AbstractCallback<TArgs...>{}
        {
            this->set_callable(std::bind_front(method, object));
        }

        /// @brief Callback constructor from a callable.
        ///
        /// @details
        /// This constructor initializes the internal callable to the one specified in @p callable , so that it is
        /// invoked from within the current context (i.e. as a regular function).
        ///
        /// @param[in] callable         Callable to store. For instance, this could be a lambda or anything convertible
        ///                             to `std::function`.
        /// @{
        explicit DirectCallback(const DirectCallback::Callable& callable)
            : AbstractCallback<TArgs...>{}
        {
            this->set_callable(callable);
        }

        explicit DirectCallback(DirectCallback::Callable&& callable)
            : AbstractCallback<TArgs...>{}
        {
            this->set_callable(callable);
        }
        /// @}
    };
}  // namespace kouta::callback
