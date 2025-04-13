#include "kouta/db/abstract-adapter.hpp"

namespace kouta::db
{
    AbstractAdapter::AbstractAdapter()
        : m_pool{nullptr}
        , m_backend_error_handler{std::bind_front(&AbstractAdapter::default_backend_error_handler, this)}
    {
    }

    void AbstractAdapter::set_pool(Pool* pool)
    {
        m_pool = pool;
    }

    void AbstractAdapter::set_backend_error_handler(BackendErrorHandler&& handler)
    {
        if (handler)
        {
            m_backend_error_handler = handler;
        }
        else
        {
            // Use default function
            m_backend_error_handler = std::bind_front(&AbstractAdapter::default_backend_error_handler, this);
        }
    }

    AbstractAdapter::Pool* AbstractAdapter::pool()
    {
        return m_pool;
    }

    std::pair<ResultCode, int> AbstractAdapter::default_backend_error_handler(const soci::soci_error* /*e*/)
    {
        // Defaults to unknown error
        return std::make_pair(ResultCode::UnknownError, 0);
    }
}  // namespace kouta::db
