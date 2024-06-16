#pragma once

#include <boost/beast/http/verb.hpp>

namespace kouta::http
{
    /// @brief Alias for HTTP method types.
    using Method = boost::beast::http::verb;
}  // namespace kouta::http
