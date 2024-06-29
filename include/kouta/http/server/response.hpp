#pragma once

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

namespace kouta::http::server
{
    /// Raw HTTP response
    using Response = boost::beast::http::response<boost::beast::http::string_body>;
}  // namespace kouta::http::server
