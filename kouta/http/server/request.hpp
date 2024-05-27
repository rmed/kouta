#pragma once

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

namespace kouta::http::server
{
    /// @brief HTTP request wrapper.
    ///
    /// @details
    /// The request contains the underlying Beast request object, as well as some pre-parsed
    /// information, such as parameters and query arguments.
    struct Request
    {
        /// Raw HTTP request
        boost::beast::http::request<boost::beast::http::string_body> req;
    };
}  // namespace kouta::http::server
