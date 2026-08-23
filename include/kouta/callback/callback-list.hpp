#pragma once

#include <initializer_list>
#include <vector>

#include "abstract-callback.hpp"

namespace kouta::callback
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
    /// Cls obj{};
    ///
    /// // Store multiple callbacks
    /// CallbackList cb_list{
    ///     DirectCallback<int, const std::string& cb{free_func},
    ///     DirectCallback<int, const std::string& cb{&obj, &Cls::bound_method},
    ///     NullCallback<int, const std::string&>{}
    /// };
    ///
    /// cb_list(42, "test");
    /// @endcode
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class CallbackList : public AbstractCallback<TArgs...>
    {
    public:
        /// @brief Default constructor.
        ///
        /// @details
        /// The internal list of callbacks to invoke will be empty and can be updated via the @ref set_callbacks()
        /// method.
        CallbackList()
            : AbstractCallback<TArgs...>{}
            , m_callbacks{}
        {
            this->set_callable(std::bind_front(&CallbackList::invoke_callbacks, this));
        }

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
        CallbackList(std::initializer_list<AbstractCallback<TArgs...>> callbacks)
            : CallbackList<TArgs...>{}
            , m_callbacks{callbacks}
        {
        }

        /// @brief Update the internal list of callbacks to invoke.
        ///
        /// @param[in] callbacks        Set of Callbacks to store.
        void set_callbacks(std::initializer_list<AbstractCallback<TArgs...>> callbacks)
        {
            m_callbacks.assign(callbacks);
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

        std::vector<AbstractCallback<TArgs...>> m_callbacks;
    };
}  // namespace kouta::callback
