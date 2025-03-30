#pragma once

#include <atomic>
#include <format>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <thread>
#include <unordered_map>

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
    ///
    /// The size of the connection pool may be set when constructing the object, or via the @ref
    /// set_connection_pool_size() method (as long as the connection has not been established).
    class Client : public kouta::utils::LoggerAware
    {
    public:
        /// @brief Database connection pool type.
        using Pool = soci::connection_pool;

        /// @brief Database backend.
        using Backend = client_detail::Backend;

        /// @brief Map of database session leases per thread.
        using LeaseMap = std::unordered_map<std::thread::id, std::weak_ptr<soci::session>>;

        /// @brief Database session lease.
        using SessionLease = std::shared_ptr<soci::session>;

        /// @brief Default constructor.
        ///
        /// @details
        /// The internal connection pool size will be set to 1.
        Client()
            : Client{1}
        {
        }

        /// @brief Constructor.
        ///
        /// @param[in] pool_size            Size of the internal connection pool. Will be set to 1 if value 0 is
        ///                                 specified.
        explicit Client(std::size_t pool_size)
            : kouta::utils::LoggerAware{}
            , m_pool_size{pool_size > 0 ? pool_size : 1}
            , m_initialized{}
            , m_backend{}
            , m_pool{}
            , m_thread_session_mutex{}
            , m_thread_leases{}
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

        /// @brief Obtain the map of leased sessions per thread.
        const LeaseMap& thread_leases() const
        {
            return m_thread_leases;
        }

        /// @brief Obtain a pointer to the internal pool.
        ///
        /// @details
        /// This pointer may be used to retrieve sessions via the lease mechanism implemented by SOCI. However, for most
        /// cases, and in order to allow efficient reuse of sessions within an adapter, it is recommended to obtained a
        /// leased session via @ref lease_session().
        ///
        /// @warning The pool must have been initialized beforehand via one of the connection methods.
        ///
        /// @returns Pointer to the connection pool if initialized, `nullptr` otherwise.
        Pool* pool()
        {
            return m_pool.get();
        }

        /// @brief Clean thread session leases which are no longer valid.
        ///
        /// @details
        /// This method will check the internal map of leases and remove those which are no longer valid (e.g. no shared
        /// pointer is currently active).
        ///
        /// This operation should be performed periodically at the developer's discretion if the application is expected
        /// to have an arbitrary number of threads. In cases where the number of threads is finite and controlled, it
        /// may not be necessary to do so, as the internal map will not grow out of control.
        ///
        /// @note This operation is thread-safe.
        ///
        /// @warning
        /// Calling this method may incur in a performance overhead as it is not possible to lease any sessions while
        /// the cleaning is in progress.
        void clean_leases()
        {
            std::unique_lock<std::shared_mutex> lock{m_thread_session_mutex};

            auto it = m_thread_leases.begin();

            while (it != m_thread_leases.end())
            {
                if (it->second.expired())
                {
                    m_thread_leases.erase(it++);
                }
                else
                {
                    ++it;
                }
            }
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

            // Close sessions
            log_debug("Closing sessions in the connection pool...");

            for (std::size_t i{}; i < m_pool_size; i++)
            {
                try
                {
                    soci::session& sql{m_pool->at(i)};
                    sql.close();
                }
                catch (...)
                {
                    log_error(std::format("Failed to close database session {}", i));
                }
            }

            m_pool.reset();
            m_backend = Backend::None;
            m_initialized = false;

            log_debug("Disconnected");
        }

        /// @brief Obtain a session from the session pool.
        ///
        /// @details
        /// This method is especially intended for multi-threaded applications in which a thread may perform several
        /// operations on the database as part of a larger parent operation. Obtaining a lease this way instead of
        /// manually via the connection pool will guarantee that subsequent lease requests from the same thread will
        /// always return the same session until the first (parent) lease is finished and the session returns to the
        /// pool.
        ///
        /// A @ref SessionLease is a shared pointer which is tracked by a weak pointer assigned to the ID of the caller
        /// thread. The client will check whether a lease already exists for the current thread and return a new shared
        /// pointer to that specific session. In practice, this mechaism allows for re-entrant calls in @ref
        /// kouta::db::AbstractAdapter::session_run(). Once the last @ref SessionLease is destroyed (usually via RAII),
        /// the inner weak pointer will know that no further leases are active for the thread.
        ///
        /// @note
        /// In cases were an arbitrary number of threads (or dynamic threads) is expected, the @ref clean_leases()
        /// method should be called periodically, as the client has no way of knowing when a lease has ended unless it
        /// specifically checks the entry for a given thread ID.
        ///
        /// @warning
        /// This method may block until a session is available in the pool if the thread does not already have a valid
        /// lease.
        ///
        /// @returns Shared pointer to the session, or an empty pointer if the pool has not been initialized.
        SessionLease lease_session()
        {
            if (!m_pool)
            {
                log_warning("Attempted to lease a session even though a connection pool is not configured");
                return {};
            }

            // Check if current thread has a lease already
            {
                std::shared_lock<std::shared_mutex> lock{m_thread_session_mutex};

                auto it = m_thread_leases.find(std::this_thread::get_id());

                if (it != m_thread_leases.cend() && !it->second.expired())
                {
                    // Session already exists
                    return it->second.lock();
                }
            }

            // Need to create a new session (this may block)
            SessionLease lease{std::make_shared<soci::session>(*m_pool)};

            {
                std::unique_lock<std::shared_mutex> lock{m_thread_session_mutex};

                m_thread_leases.insert_or_assign(std::this_thread::get_id(), std::weak_ptr<soci::session>{lease});
            }

            return lease;
        }

        /// @brief Set the size of the connection pool.
        ///
        /// @note This can only be done if the connection has not been established.
        ///
        /// @param[in] pool_size        New size for the pool. If set to 0, will be automatically be se to 1.
        void set_connection_pool_size(std::size_t pool_size)
        {
            if (m_initialized || m_backend != Backend::None)
            {
                // Already initialized
                log_warning("Cannot change the size of the connection pool after the client has been initialized");
                return;
            }

            m_pool_size = pool_size > 0 ? pool_size : 1;

            log_debug(std::format("Connection pool size set to {}", m_pool_size));
        }

    private:
        std::size_t m_pool_size;
        std::atomic_bool m_initialized;
        Backend m_backend;
        std::unique_ptr<Pool> m_pool;

        std::shared_mutex m_thread_session_mutex;
        LeaseMap m_thread_leases;
    };
}  // namespace kouta::db
