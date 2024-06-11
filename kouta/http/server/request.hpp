#pragma once

#include <map>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/url.hpp>

namespace kouta::http::server
{
    /// @brief HTTP request wrapper.
    ///
    /// @details
    /// The request contains the underlying Beast request object, as well as some pre-parsed
    /// information, such as parameters and query arguments.
    ///
    /// @note Most pre-parsed information will be views pointing to the underlying request itself.
    struct Request
    {
        /// Raw HTTP request.
        boost::beast::http::request<boost::beast::http::string_body> req;

        /// Extracted URL.
        boost::urls::url_view url;

        /// Requested scheme.
        std::string_view scheme;

        /// Requested host.
        std::string_view host;

        /// Requested path.
        std::string_view path;

        /// Requested path segment.
        boost::urls::segments_view path_segments;

        /// Query arguments.
        boost::urls::params_view args;

        /// Route parameters.
        std::map<std::string, std::string> params;
    };
}  // namespace kouta::http::server
