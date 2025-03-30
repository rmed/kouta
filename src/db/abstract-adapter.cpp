#include "kouta/db/abstract-adapter.hpp"

namespace kouta::db
{
    void AbstractAdapter::set_pool(Pool* pool)
    {
        m_pool = pool;
    }

    AbstractAdapter::Pool* AbstractAdapter::pool()
    {
        return m_pool;
    }
}  // namespace kouta::db
