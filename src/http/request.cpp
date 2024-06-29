#include <kouta/http/server/request.hpp>

namespace kouta::http::server
{
    const Request::PathParams& Request::path_params() const
    {
        return m_path_params;
    }

    void Request::set_path_params(const PathParams& params)
    {
        m_path_params = params;
    }
}  // namespace kouta::http::server
