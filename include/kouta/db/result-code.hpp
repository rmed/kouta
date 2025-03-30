#pragma once

#include <cstdint>

namespace kouta::db
{
    enum class ResultCode : std::uint8_t
    {
        /// @brief No error.
        Ok,

        /// @brief No data was found for the executed query.
        NotFound,

        /// @brief Database backend error.
        ///
        /// @details
        /// The additional `error_detail` attribute of the @ref Result should be set to the appropriate error code
        /// depending on the backend.
        DatabaseBackendError,

        /// @brief Database adapter error.
        ///
        /// @details
        /// The additional `error_detail` attribute of the @ref Result should be set by the adapter to to the
        /// appropriate error code. Adapter error codes should be properly documented, if used.
        AdapterError,

        /// @brief Unknown error encountered whilst running the query.
        UnknownError
    };
}
