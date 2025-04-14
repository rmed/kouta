#pragma once

#include <atomic>
#include <format>
#include <memory>
#include <string_view>

#include <soci/connection-pool.h>
#include <soci/session.h>

#include <kouta/utils/logger-aware.hpp>

#include "params/sqlite-params.hpp"

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
    class Client : public kouta::utils::LoggerAware
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
        explicit Client(std::size_t pool_size)
            : kouta::utils::LoggerAware{}
            , m_pool_size{pool_size}
            , m_initialized{}
            , m_backend{}
            , m_pool{}
        {
        }

        // Not copyable
        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;

        // Not movable
        Client(Client&&) = delete;
        Client& operator=(Client&&) = delete;

        virtual ~Client() = default;

        /// @brief Determine whether the client has been initialized.
        bool initialized() const
        {
            return m_initialized;
        }

        /// @brief Determine the backend used by the client.
        Backend backend() const
        {
            return m_backend;
        }

        /// @brief Obtain a pointer to the internal pool.
        ///
        /// @warning The pool must have been initialized beforehand via one of the connection methods.
        ///
        /// @returns Pointer to the connection pool if initialized, `nullptr` otherwise.
        Pool* pool()
        {
            return m_pool.get();
        }

        /// @brief Connect to a SQLite3 database.
        ///
        /// @param[in] db_path              Path to the database file to connect to.
        ///
        /// @returns Whether connection succeeded.
        bool connect_sqlite(std::string_view db_path)
        {
            return connect_sqlite(db_path, {});
        }

        /// @brief Connect to a SQLite3 database.
        ///
        /// @param[in] db_path              Path to the database file to connect to.
        /// @param[in] params               Optional parameters for the connection.
        ///
        /// @returns Whether connection succeeded.
        bool connect_sqlite(std::string_view db_path, const params::SqliteParams& params)
        {
            if (m_initialized || m_backend != Backend::None)
            {
                // Already initialized
                return false;
            }

            // Parse parameters
            std::ostringstream conn_stream{};

            conn_stream << "db=" << db_path << " shared_cache=true";

            if (params.timeout.has_value())
            {
                conn_stream << " timeout=" << std::to_string(params.timeout.value());
            }

            if (params.readonly.has_value())
            {
                conn_stream << " readonly=" << (params.readonly.value() ? "1" : "0");
            }

            if (params.synchronous.has_value())
            {
                conn_stream << " synchronous=" << params.synchronous.value();
            }

            if (params.vfs.has_value())
            {
                conn_stream << " vfs=" << params.vfs.value();
            }

            std::string conn_string{conn_stream.str()};

            log_debug(std::format("Connecting to SQLite database with connection string: {}", conn_string));

            m_pool = std::make_unique<Pool>(m_pool_size);

            try
            {
                // Initialize sessions
                for (std::size_t i{}; i < m_pool_size; i++)
                {
                    soci::session& sql{m_pool->at(i)};
                    sql.open("sqlite3", conn_string);
                }

                m_backend = Backend::Sqlite;
                m_initialized = true;

                return true;
            }
            catch (...)
            {
                m_pool.reset();
                m_backend = Backend::None;
                m_initialized = false;

                log_error(std::format("Failed to connect to SQLite3 database at {}", db_path));

                return false;
            }
        }

        /// @brief Disconnect the client and release the pool.
        ///
        /// @details
        /// This method may be used to disconnect from a database backend in order to reuse the same client for
        /// subsequent connections.
        ///
        /// @warning The client does not verify whether sessions in the pool are currently being used.
        void disconnect()
        {
            if (!m_initialized || m_backend == Backend::None)
            {
                // Nothing to do
                return;
            }

            log_debug("Disconnected");

            m_pool.reset();
            m_backend = Backend::None;
            m_initialized = false;
        }

    private:
        std::size_t m_pool_size;
        std::atomic_bool m_initialized;
        Backend m_backend;
        std::unique_ptr<Pool> m_pool;
    };
}  // namespace kouta::db
