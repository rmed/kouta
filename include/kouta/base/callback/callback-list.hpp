#pragma once

#include <initializer_list>
#include <vector>

#include <kouta/base/callback/base-callback.hpp>

namespace kouta::base::callback
{
    /// @brief Callback list.
    ///
    /// @details
    /// The Callback list exposes the same API as a regular Callback, but is initialized by providing a set of any kind
    /// of Callback. When the callback list is called, it will in turn invoke every Callback contained within.
    ///
    /// The lifetime of the contained Callbacks and the objects they may point to must be guaranteed to surpass
    /// that of the Callback list itself.
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class CallbackList : public BaseCallback<TArgs...>
    {
    public:
        // Copyable
        CallbackList(const CallbackList&) = default;
        CallbackList& operator=(const CallbackList&) = default;

        // Movable
        CallbackList(CallbackList&&) = default;
        CallbackList& operator=(CallbackList&&) = default;

        ~CallbackList() override = default;

        /// @brief Callback constructor from a set of callbacks
        ///
        /// @details
        /// This constructor initializes the internal callable to point to an internal function that calls all the
        /// stored Callbacks (in order).
        ///
        /// @param[in] callbacks        Set of Callbacks to store.
        CallbackList(std::initializer_list<BaseCallback<TArgs...>> callbacks)
            : BaseCallback<TArgs...>{}
            , m_callbacks{callbacks}
        {
            this->set_callable(std::bind_front(&CallbackList::invoke_callbacks, this));
        }

    private:
        /// @brief Invoke all stored callbacks in order.
        void invoke_callbacks(TArgs... args)
        {
            for (const auto& cb : m_callbacks)
            {
                cb(args...);
            }
        }

        std::vector<BaseCallback<TArgs...>> m_callbacks;
    };
}  // namespace kouta::base::callback
