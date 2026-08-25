#include "kouta/db/client.hpp"

#include <format>
#include <mutex>

namespace kouta::db
{

    Client::Client()
        : Client{1}
    {
    }

    Client::Client(std::size_t pool_size)
        : kouta::utils::LoggerAware{}
        , m_pool_size{pool_size > 0 ? pool_size : 1}
        , m_initialized{}
        , m_backend{}
        , m_pool{}
        , m_thread_session_mutex{}
        , m_thread_leases{}
    {
    }

    bool Client::initialized() const
    {
        return m_initialized;
    }

    Client::Backend Client::backend() const
    {
        return m_backend;
    }

    const Client::LeaseMap& Client::thread_leases() const
    {
        return m_thread_leases;
    }

    Client::Pool* Client::pool()
    {
        return m_pool.get();
    }

    void Client::clean_leases()
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

    bool Client::connect_sqlite(std::string_view db_path)
    {
        return connect_sqlite(db_path, {});
    }

    bool Client::connect_sqlite(std::string_view db_path, const params::SqliteParams& params)
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

    void Client::disconnect()
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

    Client::SessionLease Client::lease_session()
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

    void Client::set_connection_pool_size(std::size_t pool_size)
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
}  // namespace kouta::db
