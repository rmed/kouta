#pragma once

#include <array>
#include <chrono>
#include <map>

#include <boost/beast/http/field.hpp>

namespace kouta::http::server
{
    namespace config_detail
    {
        /// Maximum buffer size of 1 MB.
        constexpr std::size_t DEFAULT_MAX_BUFFER_SIZE{1 * 1024 * 1024};

        /// Maximum duration of the handling of 15 seconds.
        constexpr std::chrono::seconds DEFAULT_REQUEST_TIMEOUT{15};

        /// Specify response fields.
        constexpr std::array<std::pair<boost::beast::http::field, std::string_view>, 1> DEFAULT_RESPONSE_FIELDS{
            std::make_pair(boost::beast::http::field::server, "Kouta")};
    }  // namespace config_detail

    /// @brief Server configuration structure.
    ///
    /// @details
    /// This structure provides sensible defaults for the configuration of the server and the request processing.
    struct Config
    {
        /// @brief Maximum buffer size for request handling in bytes.
        std::size_t max_buffer_size = config_detail::DEFAULT_MAX_BUFFER_SIZE;

        /// @brief Duration after which a the processing of a request will be aborted.
        std::chrono::seconds request_timeout = config_detail::DEFAULT_REQUEST_TIMEOUT;

        /// @brief Fields to add to a response upon creation before being passed to the handlers.
        std::map<boost::beast::http::field, std::string> base_response_fields = {
            config_detail::DEFAULT_RESPONSE_FIELDS.cbegin(), config_detail::DEFAULT_RESPONSE_FIELDS.cend()};
    };
}  // namespace kouta::http::server
