#pragma once

#include "abstract-callback.hpp"

namespace kouta::callback
{
    /// @brief Null callback.
    ///
    /// @details
    /// A Callback that does nothing when called. As opposed to @ref AbstractCallback, which throws an exception when
    /// called and no callable has been set.
    ///
    /// Example usage:
    ///
    /// @code
    /// NullCallback<int, std:string cb{};
    ///
    /// // This does nothing, instead of throwing an exception
    /// cb(42, "test");
    /// @endcode
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class NullCallback : public AbstractCallback<TArgs...>
    {
    public:
        /// @brief The type of the callable the Callback points to.
        using Callable = typename AbstractCallback<TArgs...>::Callable;

        NullCallback()
            : AbstractCallback<TArgs...>{}
        {
            this->set_callable(
                [](TArgs...)
                {
                    // Does nothing
                });
        }

        // Copyable
        NullCallback(const NullCallback&) = default;
        NullCallback& operator=(const NullCallback&) = default;

        // Movable
        NullCallback(NullCallback&&) = default;
        NullCallback& operator=(NullCallback&&) = default;

        ~NullCallback() override = default;
    };
}  // namespace kouta::callback
