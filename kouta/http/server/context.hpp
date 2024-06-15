#pragma once

#include <any>

namespace kouta::http::server::context
{
    namespace detail
    {
        /// @brief Request context container.
        ///
        /// @details
        /// The context can be accessed by any handler or middleware **within the same thread** when processing a
        /// request in order to pass information to other steps of the processing chain. It is re-created before the
        /// processing starts.
        ///
        /// This variable should not be accessed directly. Instead, the @ref set_context() and @ref get_context()
        /// methods should be used to perform the appropriate type conversions.
        thread_local std::any t_context;
    }  // namespace detail

    /// @brief Set the context variable.
    ///
    /// @brief
    /// This function is called by the server worker in order to bind a new context to the current thread, making it
    /// available to middleware and handlers in the processing chain.
    ///
    /// @tparam TContext            Context type to set.
    ///
    /// @param[in] context          Context to set. Ideally moves the created object to the thread-local variable.
    template<class TContext>
    void set_context(TContext&& context)
    {
        detail::t_context.reset();
        detail::t_context = std::move(context);
    }

    /// @brief Obtain a pointer to the request context.
    ///
    /// @details
    /// Handlers and middleware in the request processing chain can obtain a pointer to the application-specific context
    /// in order to retrieve or store information.
    ///
    /// @tparam TContext            Context type to retrieve. This is application-dependent and is up to the developer
    ///                             to specify the correct type that was registered in the server.
    ///
    /// @returns Pointer to the context variable, or nullptr if type was not valid or variable is not set.
    template<class TContext>
    TContext* get_context()
    {
        return std::any_cast<TContext>(&detail::t_context);
    }
}  // namespace kouta::http::server::context
