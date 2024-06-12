#pragma once

#include <functional>
#include <list>
#include <optional>

#include <kouta/http/server/request.hpp>
#include <kouta/http/server/response.hpp>

namespace kouta::http::server
{
    namespace handler_detail
    {
        /// @brief Pre-request middleware function type.
        ///
        /// @details
        /// These functions are intended to perform some pre-processing on the request, such as initializing a
        /// database connection or extracting certain information to use in the handlers. The @p ctx parameter can be
        /// modified to provide this information further down the processing chain.
        ///
        /// Returning a @ref Response from these functions effectively interrupts the chain, which can be useful in case
        /// a specific pre-condition has not been met.
        ///
        /// @tparam TContext        Type of the context to provide to the request processing chain.
        ///
        /// @param[in] request      Received request.
        /// @param[in,out] ctx      Context built during the request handling flow.
        ///
        /// @returns std::nullopt to continue with the processing chain, otherwise a @ref Response object to interrupt
        /// it.
        template<class TContext>
        using PreMiddlewareFunc = std::function<std::optional<Response>(const Request& request, TContext& ctx)>;

        /// @brief Post-request middleware function type.
        ///
        /// @details
        /// These functions are intended to perform some post-processing on the response, such as setting a content type
        /// or dropping a database connection. Both the @p response and @p ctx parameters can be modified to provide
        /// information further down the processing chain.
        ///
        /// Returning a @ref Response from these functions effectively interrupts the chain, which can be useful in case
        /// a specific post-condition has not been met.
        ///
        /// @tparam TContext        Type of the context to provide to the request processing chain.
        ///
        /// @param[in] response     Response returned by the handler.
        /// @param[in,out] ctx      Context built during the request handling flow.
        ///
        /// @returns std::nullopt to continue with the processing chain, otherwise a @ref Response object to interrupt
        /// it.
        template<class TContext>
        using PostMiddlewareFunc = std::function<std::optional<Response>(Response& response, TContext& ctx)>;

        /// @brief Request handler type.
        ///
        /// @tparam TContext        Type of the context to provide to the request processing chain.
        ///
        /// @param[in] request      Received request.
        /// @param[in,out] ctx      Context built during the request handling flow.
        ///
        /// @returns response to send, which may be processed further by any post-request middleware.
        template<class TContext>
        using HandlerFunc = std::function<Response(const Request& request, TContext& ctx)>;
    }  // namespace handler_detail

    /// @brief Complete flow used to handle a request.
    ///
    /// @tparam TContext                Type of the context to provide to the request processing chain.
    template<class TContext>
    struct Handler
    {
        /// Request handler.
        handler_detail::HandlerFunc<TContext> handler;
        /// Functions to call before the handler.
        std::list<handler_detail::PreMiddlewareFunc<TContext>> pre_request;
        /// Functions to call after the handler.
        std::list<handler_detail::PostMiddlewareFunc<TContext>> post_request;
    };
}  // namespace kouta::http::server
