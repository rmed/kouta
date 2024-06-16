#pragma once

#include <functional>
#include <vector>
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
        /// database connection or extracting certain information to use in the handlers. A context pointer can be
        /// obtained via the @ref context::get_context() function in order to pass information to latter steps.
        ///
        /// Returning a @ref Response from these functions effectively interrupts the chain, which can be useful in case
        /// a specific pre-condition has not been met.
        ///
        /// @param[in] request          Received request.
        /// @param[in,out] response     Response for the processing chain.
        ///
        /// @returns true to continue the processing chain, otherwise false to send the response as-is.
        using PreMiddlewareFunc = std::function<bool(const Request& request, Response& response)>;

        /// @brief Post-request middleware function type.
        ///
        /// @details
        /// These functions are intended to perform some post-processing on the response, such as setting a content type
        /// or dropping a database connection. A context pointer can be obtained via the @ref context::get_context()
        /// function, in order to pass information to latter steps. The @p response parameter can also be modified
        /// in-place.
        ///
        /// Returning a @ref Response from these functions effectively interrupts the chain, which can be useful in case
        /// a specific post-condition has not been met.
        ///
        /// @param[in] response     Response returned by the handler.
        /// @param[in,out] response     Response for the processing chain.
        ///
        /// @returns true to continue the processing chain, otherwise false to send the response as-is
        using PostMiddlewareFunc = std::function<bool(Response& response)>;

        /// @brief Request handler type.
        ///
        /// @details
        /// A context pointer can be obtained via the @ref context::get_context() function in order to receive
        /// information from previous steps or to pass information to latter steps.
        ///
        /// @param[in] request          Received request.
        /// @param[in,out] response     Response for the processing chain.
        ///
        /// @returns true to continue the processing chain, otherwise false to send the response as-is.
        using HandlerFunc = std::function<bool(const Request& request, Response& response)>;
    }  // namespace handler_detail

    /// @brief Complete flow used to handle a request.
    struct HandlerFlow
    {
        /// Request handler.
        handler_detail::HandlerFunc handler;
        /// Functions to call before the handler.
        std::vector<handler_detail::PreMiddlewareFunc> pre_request;
        /// Functions to call after the handler.
        std::vector<handler_detail::PostMiddlewareFunc> post_request;
    };
}  // namespace kouta::http::server
