#pragma once

#include "kouta/async/context.hpp"

#include <asio/io_context.hpp>

namespace kouta::async
{
    struct Context::Impl
    {
        /// @brief Private asio context.
        asio::io_context asio_context;
    };
}  // namespace kouta::async
