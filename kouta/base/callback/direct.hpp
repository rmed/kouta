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
    class Direct : public BaseCallback<TArgs...>
    {
    public:
        // Copyable
        Direct(const Direct&) = default;
        Direct& operator=(const Direct&) = default;

        // Movable
        Direct(Direct&&) = default;
        Direct& operator=(Direct&&) = default;

        ~Direct() override = default;

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
        Direct(TClass* object, void (TClass::*method)(TArgs...))
            : BaseCallback<TArgs...>{}
        {
            this->set_callable(std::bind_front(method, object));
        }
    };
}  // namespace kouta::base::callback
