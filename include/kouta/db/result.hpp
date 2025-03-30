#pragma once

#include <optional>

#include "result-code.hpp"

namespace kouta::db
{
    /// @brief POD for database results.
    ///
    /// @details
    /// This structure acts as a container for the result of the query and, optionally, any data that may be returned by
    /// the database.
    ///
    /// @tparam TData           Type of the data that may be contained in the result. This is set to `bool` by default
    ///                         in order to facilitate creating results where data type is not really relevant (e.g.
    ///                         only result code is important), or no data is returned.
    template<class TData = bool>
    struct Result
    {
        /// @brief Data type contained in the `data` attribute.
        using DataType = TData;

        /// @brief Database operation/query result code.
        ResultCode code;

        /// @brief Optional additional error/result code-
        ///
        /// @details
        /// The meaning of this attribute may depend on the value of @ref code or the database backend used, for
        /// instance.
        int error_detail;

        /// @brief Data that may be returned as a result.
        std::optional<TData> data;
    };
}  // namespace kouta::db
