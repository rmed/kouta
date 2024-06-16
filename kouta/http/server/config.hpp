#pragma once

#include <cstdint>
#include <chrono>

namespace kouta::http::server
{
    namespace config_detail
    {
        /// Maximum buffer size of 1 MB.
        constexpr std::size_t DEFAULT_MAX_BUFFER_SIZE{1 * 1024 * 1024};

        /// Maximum duration of the handling of 15 seconds.
        constexpr std::chrono::seconds DEFAULT_REQUEST_TIMEOUT{15};
    }

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
    };
}  // namespace kouta::http::server
