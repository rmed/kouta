#pragma once

#include <functional>

namespace kouta::callback
{
    /// @brief Safe function pointer wrapper.
    ///
    /// @details
    /// A Callback can be used to store pointers to callable such as free functions, lambdas, or member methods.
    ///
    /// This class serves as a base for custom Callback implementations that may need to take into account details such
    /// as event loops or scheduling, depending on the target of the Callback. In addition, this type may be used to
    /// specify generic callbacks, making components independent of where the callback points to.
    ///
    /// @tparam TArgs                   Callable arguments.
    template<class... TArgs>
    class AbstractCallback
    {
    public:
        /// @brief The type of the callable the Callback points to.
        using Callable = std::function<void(TArgs...)>;

        /// @brief Default constructor.
        ///
        /// @details
        /// Callbacks may be default-constructed, allowing for "optional callbacks" that may or may not be specified
        /// at runtime. Do note that calling a Callback that has not been initialized via @ref set_callable() will throw
        /// an exception.
        AbstractCallback() = default;

        // Copyable
        AbstractCallback(const AbstractCallback&) = default;
        AbstractCallback& operator=(const AbstractCallback&) = default;

        // Movable
        AbstractCallback(AbstractCallback&&) = default;
        AbstractCallback& operator=(AbstractCallback&&) = default;

        virtual ~AbstractCallback() = default;

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
        /// This method exists to allow inheriting from the AbstractCallback to add custom constructors that may, for
        /// instance, interface with event loops by posting events.
        ///
        /// @param[in] callable          Callable to use internally.
        /// @{
        void set_callable(const Callable& callable)
        {
            m_callable = callable;
        }

        void set_callable(Callable&& callable)
        {
            m_callable = callable;
        }
        /// @}

    private:
        Callable m_callable;
    };
}  // namespace kouta::callback
