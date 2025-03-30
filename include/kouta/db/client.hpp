#pragma once

#include <atomic>
#include <memory>
#include <string_view>

#include <soci/connection-pool.h>

#include <kouta/utils/logger-aware.hpp>

#include "detail/sqlite-params.hpp"

namespace kouta::db
{
    namespace client_detail
    {
        /// @brief Client database backend used.
        enum class Backend : std::uint8_t
        {
            /// Used when not initialized.
            None,
            Sqlite,
            MySql,
            Postgres,
            Oracle
        };
    }  // namespace client_detail

    /// @brief Database client.
    ///
    /// @details
    /// The client acts as container for the database connection pool and is agnostic of the database backend used. To
    /// that end, it exposes a series of connection methods that will attempt to load the approprate database backend
    /// library during runtime.
    ///
    /// This class is intended to be *integrated* into application-specific database code and will not perform any logic
    /// outside of the scope of connecting to the backend and managing the connection pool.
    class Client : kouta::utils::LoggerAware
    {
    public:
        /// @brief Database connection pool type.
        using Pool = soci::connection_pool;

        /// @brief Database backend.
        using Backend = client_detail::Backend;

        /// @brief Default constructor.
        Client() = delete;

        /// @brief Constructor.
        ///
        /// @param[in] pool_size            Size of the internal connection pool.
        explicit Client(std::size_t pool_size);

        // Not copyable
        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;

        // Not movable
        Client(Client&&) = delete;
        Client& operator=(Client&&) = delete;

        virtual ~Client() = default;

        /// @brief Determine whether the client has been initialized.
        bool initialized() const;

        /// @brief Determine the backend used by the client.
        Backend backend() const;

        /// @brief Obtain a pointer to the internal pool.
        ///
        /// @warning The pool must have been initialized beforehand via one of the connection methods.
        ///
        /// @returns Pointer to the connection pool if initialized, `nullptr` otherwise.
        Pool* pool();

        /// @brief Connect to a SQLite3 database.
        ///
        /// @param[in] db_path              Path to the database file to connect to.
        ///
        /// @returns Whether connection succeeded.
        bool connect_sqlite(std::string_view db_path);

        /// @brief Connect to a SQLite3 database.
        ///
        /// @param[in] db_path              Path to the database file to connect to.
        /// @param[in] params               Optional parameters for the connection.
        ///
        /// @returns Whether connection succeeded.
        bool connect_sqlite(std::string_view db_path, detail::SqliteParams&& params);

        /// @brief Disconnect the client and release the pool.
        ///
        /// @details
        /// This method may be used to disconnect from a database backend in order to reuse the same client for
        /// subsequent connections.
        ///
        /// @warning The client does not verify whether sessions in the pool are currently being used.
        void disconnect();

    private:
        std::size_t m_pool_size;
        std::atomic_bool m_initialized;
        Backend m_backend;
        std::unique_ptr<Pool> m_pool;
    };
}  // namespace kouta::db
