#pragma once

#include <cmath>

#include <soci/rowset.h>
#include <soci/session.h>

#include "page.hpp"
#include "query/query-builder.hpp"
#include "query/where-builder.hpp"
#include "result.hpp"

namespace kouta::db::utils
{
    /// @brief Obtain a paginated list of items from a table.
    ///
    /// @details
    /// This function assumes that a `soci::type_conversion` is specified for the specified model type `TModel`.
    ///
    /// First, a count of items is extracted, and then pagination is applied using OFFSET and LIMIT.
    ///
    /// @tparam TModel                  Type of the models being retrieved.
    ///
    /// @param[in,out] session          Database session.
    /// @param[in] table                Name of the table from which to extract information.
    /// @param[in] page                 Page to retrieve.
    /// @param[in] page_size            Items to retrieve per page. Set to 0 to fetch all items.
    /// @param[in] where_clause         WHERE clause to apply to the COUNT and SELECT queries.
    ///
    /// @return Result indicating the ID of the inserted row.
    template<class TModel>
    Result<Page<TModel>> get_list(
        soci::session& session,
        const std::string_view& table,
        std::size_t page,
        std::size_t page_size,
        const query::WhereBuilder& where_clause)
    {
        query::QueryBuilder builder{};
        Page<TModel> page_result{};

        // Find number of elements
        int count{};

        builder.select(table, {"COUNT(1)"});

        if (where_clause)
        {
            builder.where(
                [&where_clause](query::WhereBuilder& b)
                {
                    b = where_clause;
                });
        }

        soci::statement st_count{session};
        st_count.exchange(soci::into(count));
        st_count.alloc();
        st_count.prepare(builder.query());
        st_count.define_and_bind();
        st_count.execute(true);

        if (count == 0)
        {
            return {ResultCode::Ok, 0, std::move(page_result)};
        }

        if (page_size == 0)
        {
            // Not paginated
            page_result.set_page(page);
            page_result.set_pages(0);
            page_result.set_total_items(count);
        }
        else
        {
            // Paginated
            page_result.set_page(page);
            page_result.set_pages((count > page_size) ? std::lround(count / page_size) : 1);
            page_result.set_total_items(count);
        }

        // Retrieve items
        TModel row{};

        builder.select(table, {"COUNT(*)"});

        if (where_clause)
        {
            builder.where(
                [&where_clause](query::WhereBuilder& b)
                {
                    b = where_clause;
                });
        }

        soci::statement st_rows(session);
        st_rows.alloc();
        st_rows.prepare(builder.query());
        st_rows.define_and_bind();
        st_rows.exchange_for_rowset(soci::into(row));
        st_rows.execute(false);

        // This acts as begin/end for the assignation
        page_result.items.assign(soci::rowset_iterator<TModel>(st_rows, row), soci::rowset_iterator<TModel>{});

        return {ResultCode::Ok, 0, std::move(page_result)};
    }
}  // namespace kouta::db::utils
