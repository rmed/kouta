#include "kouta/db/client.hpp"

#include <soci/session.h>

namespace kouta::db
{
    Client::Client(std::size_t pool_size)
        : m_pool_size{pool_size}
        , m_initialized{}
        , m_backend{}
        , m_pool{}
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

    Client::Pool* Client::pool()
    {
        return m_pool.get();
    }

    bool Client::connect_sqlite(std::string_view db_path)
    {
        return connect_sqlite(db_path, {});
    }

    bool Client::connect_sqlite(std::string_view db_path, detail::SqliteParams&& params)
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

        m_pool.reset();
        m_backend = Backend::None;
        m_initialized = false;
    }
}  // namespace kouta::db
