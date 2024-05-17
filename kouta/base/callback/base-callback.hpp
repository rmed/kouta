#pragma once

#include <functional>

namespace kouta::base::callback
{
    /// @brief Safe function pointer wrapper.
    ///
    /// @details
    /// A Callback can be used to store pointers to callable such as free functions, lambdas, or member methods.
    ///
    /// This class serves as a base for custom Callback implementations that may need to take into account details such
    /// as event loops or scheduling, depending on the target of the Callback.
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class BaseCallback
    {
    public:
        using Callable = std::function<void(TArgs...)>;

        /// @brief Default constructor.
        ///
        /// @details
        /// Callbacks may be default-constructed, allowing for "optional callbacks" that may or may not be specified
        /// at runtime. Do note that calling a callback that has not been initialized will throw an exception (which is
        /// intended behaviour).
        BaseCallback() = default;

        // Copyable
        BaseCallback(const BaseCallback&) = default;
        BaseCallback& operator=(const BaseCallback&) = default;

        // Movable
        BaseCallback(BaseCallback&&) = default;
        BaseCallback& operator=(BaseCallback&&) = default;

        virtual ~BaseCallback() = default;

        /// @brief Invoke the underlying callable.
        ///
        /// @param[in] args             Arguments to provide to the invokation.
        void operator()(TArgs... args) const
        {
            m_callable(args...);
        }

    protected:
        /// @brief Set the callable.
        ///
        /// @details
        /// This method exists to allow inheriting from the Callback to add custom constructors that may, for instance,
        /// interface with event loops by posting events
        ///
        /// @param[in] callable          Callable to use internally.
        void set_callable(const Callable& callable)
        {
            m_callable = callable;
        }

    private:
        Callable m_callable;
    };
}  // namespace kouta::base::callback
