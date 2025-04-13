#pragma once

#include <tuple>

#include <soci/error.h>
#include <soci/sqlite3/soci-sqlite3.h>

#include <kouta/db/result-code.hpp>

namespace kouta::db::error::sqlite
{
    /// @brief Handle SQLite backend errors.
    ///
    /// @details
    /// This function extracts the SQLite-specific error code from the caught exception and returns it with the
    /// appropriate @ref ResultCode.
    ///
    /// The pair of values are intended to be used by an adapter when building a @ref Result object.
    ///
    /// @param[in] e            Caught exception.
    ///
    /// @returns Pair of result code and SQLite-specific result code as returned by SOCI.
    std::pair<ResultCode, int> handler(const soci::soci_error* e)
    {
        try
        {
            const soci::sqlite3_soci_error* specialized{dynamic_cast<const soci::sqlite3_soci_error*>(e)};

            return std::make_pair(ResultCode::DatabaseBackendError, specialized->result());
        }
        catch (...)
        {
            return std::make_pair(ResultCode::UnknownError, -1);
        }
    }
}  // namespace kouta::db::error::sqlite
