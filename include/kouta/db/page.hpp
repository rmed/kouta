#pragma once

#include <cstdint>
#include <vector>

namespace kouta::db
{
    /// @brief Database pagination result POD.
    ///
    /// @details
    /// This structure acts as a container for paginated results of queries by providing information about the current
    /// page, total number of pages, and number of elements per page that may be useful for the caller to request
    /// subsequent pages from the database.
    ///
    /// @tparam TData               Type of the data returned by the query.
    template<class TData>
    struct Page
    {
        /// @brief Data type contained in the `items` attribute.
        using DataType = TData;

        /// @brief Current page.
        std::size_t page;

        /// @brief Total number of pages.
        std::size_t pages;

        /// @brief Total number of items (in all pages).
        std::size_t total_items;

        /// @brief Items in the current page.
        std::vector<TData> items;
    };
}  // namespace kouta::db
