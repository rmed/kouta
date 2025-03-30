#pragma once

#include <cstdint>

namespace kouta::db
{
    enum class ResultCode : std::uint8_t
    {
        /// @brief No error.
        Ok,

        /// @brief User/Connection has no permission to do the required operation.
        PermissionError,

        /// @brief Database constraint violation.
        ///
        /// @details
        /// Example of this include primary key constraints or unique constraints.
        ConstraintViolation,

        /// @brief Statement to execute was not valid.
        InvalidStatement,

        /// @brief No data was found for the query ran.
        NotFound,

        /// @brief Database adapter error.
        ///
        /// @details
        /// The additional `error_detail` attribute of the @ref Result should be set by the adapter to to the
        /// appropriate error code. Adapter error codes should be properly documented, if used.
        AdapterError,

        /// @brief Error in a transaction state.
        TransactionError,

        /// @brief Database connection error
        ///
        /// @details
        /// This could mean that the client/pool has not been initialized.
        ConnectionError,

        /// @brief Database backend error.
        ///
        /// @details
        /// The additional `error_detail` attribute of the @ref Result should be set to the appropriate error code
        /// depending on the backend.
        DatabaseBackendError,

        /// @brief System error.
        SystemError,

        /// @brief Unknown error encountered whilst running the query.
        UnknownError
    };
}
