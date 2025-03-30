#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace kouta::db::params
{
    /// @brief Set of optional parameters to pass to the SQLite3 connection string.
    ///
    /// @note These are supported by SOCI.
    struct SqliteParams
    {
        /// @brief SQLite busy timeout (in seconds).
        std::optional<int> timeout;

        /// @brief Whether to open the database in read-only mode (instead of read-write).
        ///
        /// @note Database file must already exist if read-only mode is used.
        std::optional<bool> readonly;

        /// @brief Set the pragma synchronous flag.
        ///
        /// @details
        /// Acceptable values:
        ///
        /// - "OFF"
        /// - "NORMAL"
        /// - "FULL"
        /// - "EXTRA"
        std::optional<std::string> synchronous;

        /// @brief Set VFS used as OS interface.
        ///
        /// @note The VFS should be registered before opening the connection.
        std::optional<std::string> vfs;
    };
}  // namespace kouta::db::params
