#pragma once

#include <kouta/base/callback/base-callback.hpp>
#include <kouta/base/callback/direct.hpp>
#include <kouta/base/callback/deferred.hpp>
#include <kouta/base/callback/list.hpp>

namespace kouta::base
{
    /// @brief Make @ref callback::BaseCallback the default Callback type.
    ///
    /// @details
    /// The idea is to be able to have components use base Callbacks in order for them to be completely independent of
    /// implementation details.
    template<class... TArgs>
    using Callback = callback::BaseCallback<TArgs...>;
}  // namespace kouta::base
