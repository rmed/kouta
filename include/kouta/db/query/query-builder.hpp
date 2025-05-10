#pragma once

#include <functional>
#include <initializer_list>
#include <sstream>
#include <string_view>
#include <vector>

#include "where-builder.hpp"

namespace kouta::db::query
{
    namespace query_builder_detail
    {
        /// @brief Sorting order.
        enum class Order : std::uint8_t
        {
            Ascending,
            Descending
        };
    }  // namespace query_builder_detail

    class QueryBuilder
    {
    public:
        /// @brief Type used to represent a pair with column and placeholder.
        using ColumnPair = std::pair<std::string_view, std::string_view>;

        /// @brief Sorting order.
        using Order = query_builder_detail::Order;

        // Default constructor
        QueryBuilder() = default;

        // Copyable
        QueryBuilder(const QueryBuilder& other)
        {
            m_query.clear();
            m_query << other.m_query.rdbuf();
        }

        QueryBuilder& operator=(const QueryBuilder& other)
        {
            m_query.clear();
            m_query << other.m_query.rdbuf();

            return *this;
        }

        // Movable
        QueryBuilder(QueryBuilder&&) = default;
        QueryBuilder& operator=(QueryBuilder&&) = default;

        virtual ~QueryBuilder() = default;

        /// @brief Retrieve the SQL query string
        std::string query() const
        {
            std::string query{m_query.str()};

            // Trim trailing whitespaces
            query.erase(
                std::find_if(
                    query.rbegin(),
                    query.rend(),
                    [](unsigned char c)
                    {
                        return !std::isspace(c);
                    })
                    .base(),
                query.end());

            return query;
        }

        /// @brief Start building a SELECT query.
        ///
        /// @note This will discard any previous query.
        ///
        /// @param[in] table                Table from which to extract data.
        /// @param[in] columns              Set of columns to select. If empty, selects all columns.
        /// @{
        virtual QueryBuilder& select(std::string_view table, std::initializer_list<std::string_view> columns)
        {
            return QueryBuilder::select(table, std::vector<std::string_view>{columns});
        }

        virtual QueryBuilder& select(std::string_view table, const std::vector<std::string_view>& columns)
        {
            m_query = std::ostringstream{};

            m_query << "SELECT ";

            if (columns.empty())
            {
                m_query << "*";
            }
            else
            {
                std::size_t last_index{columns.size() - 1};

                for (std::size_t i{}; i < columns.size(); ++i)
                {
                    m_query << columns[i];

                    // Add comma to all except last
                    if (i < last_index)
                    {
                        m_query << ", ";
                    }
                }
            }

            m_query << " FROM " << table;

            return *this;
        }
        /// @}

        /// @brief Start building an INSERT query.
        ///
        /// @details
        /// If the placeholder of a pair in @p columns is omitted/empty, the name of the column will be used by
        /// prepending `:`.
        ///
        /// @note This will discard any previous query.
        ///
        /// @param[in] table                Table in which to insert data.
        /// @param[in] columns              Set of columns to insert.
        /// @{
        virtual QueryBuilder& insert(std::string_view table, std::initializer_list<ColumnPair> columns)
        {
            return insert(table, std::vector<ColumnPair>{columns});
        }

        virtual QueryBuilder& insert(std::string_view table, const std::vector<ColumnPair>& columns)
        {
            m_query = std::ostringstream{};

            m_query << "INSERT INTO " << table << " (";

            // Columns
            std::size_t last_index{columns.size() - 1};

            for (std::size_t i{}; i < columns.size(); ++i)
            {
                m_query << columns[i].first;

                // Add comma to all except last
                if (i < last_index)
                {
                    m_query << ", ";
                }
            }

            m_query << ") VALUES (";

            // Placeholders
            for (std::size_t i{}; i < columns.size(); ++i)
            {
                if (columns[i].second.empty())
                {
                    // Use column name for placeholder
                    m_query << ":" << columns[i].first;
                }
                else if (!columns[i].second.starts_with(":"))
                {
                    // User forgot placeholder mark
                    m_query << ":" << columns[i].second;
                }
                else
                {
                    m_query << columns[i].second;
                }

                // Add comma to all except last
                if (i < last_index)
                {
                    m_query << ", ";
                }
            }

            m_query << ") ";

            return *this;
        }
        /// @}

        /// @brief Start building an UPDATE query.
        ///
        /// @details
        /// If the placeholder of a pair in @p columns is omitted/empty, the name of the column will be used by
        /// prepending `:`.
        ///
        /// @note This will discard any previous query.
        ///
        /// @param[in] table                Table in which to update data.
        /// @param[in] columns              Set of columns to update.
        /// @{
        virtual QueryBuilder& update(std::string_view table, std::initializer_list<ColumnPair> columns)
        {
            return update(table, std::vector<ColumnPair>{columns});
        }

        virtual QueryBuilder& update(std::string_view table, const std::vector<ColumnPair>& columns)
        {
            m_query = std::ostringstream{};

            m_query << "UPDATE " << table << " SET ";

            std::size_t last_index{columns.size() - 1};

            for (std::size_t i{}; i < columns.size(); ++i)
            {
                m_query << columns[i].first << " = ";

                if (columns[i].second.empty())
                {
                    // Use column name for placeholder
                    m_query << ":" << columns[i].first;
                }
                else if (!columns[i].second.starts_with(":"))
                {
                    // User forgot placeholder mark
                    m_query << ":" << columns[i].second;
                }
                else
                {
                    m_query << columns[i].second;
                }

                // Add comma to all except last
                if (i < last_index)
                {
                    m_query << ", ";
                }
            }

            return *this;
        }
        /// @}

        /// @brief Start building a DELETE query.
        ///
        /// @note This will discard any previous query.
        ///
        /// @param[in] table                Table in which to delete data.
        /// @{
        virtual QueryBuilder& erase(std::string_view table)
        {
            m_query = std::ostringstream{};
            m_query << "DELETE FROM " << table;
            return *this;
        }
        /// @}

        /// @brief Add a WHERE clause.
        ///
        /// @details
        /// This method expects a @p builder function that will receive a @ref WhereBuilder object that, after the
        /// builder function has been called, will result in the conditional clauses.
        ///
        /// @param[in] builder              Function in which the clause will be built. Receives a @ref WhereBuilder.
        QueryBuilder& where(std::function<void(WhereBuilder&)>&& builder)
        {
            WhereBuilder whereBuilder{};
            builder(whereBuilder);

            m_query << " WHERE " << whereBuilder.build();
            return *this;
        }

        /// @brief Add a simple JOIN clause.
        ///
        /// @param[in] table            Name of the table to join.
        /// @param[in] condition        Joining condition. Set to empty string to have the database figure it out.
        virtual QueryBuilder& join(std::string_view table, std::string_view condition)
        {
            m_query << " JOIN " << table;

            if (!condition.empty())
            {
                m_query << " ON " << condition;
            }

            return *this;
        }

        /// @brief Add an ORDER BY clause.
        ///
        /// @param[in] columns              Set of columns to order by.
        /// @param[in] order                Ordering to use.
        /// @{
        virtual QueryBuilder& order_by(std::initializer_list<std::string_view> columns, Order order)
        {
            return order_by(std::vector<std::string_view>{columns}, order);
        }

        virtual QueryBuilder& order_by(const std::vector<std::string_view>& columns, Order order)
        {
            m_query << " ORDER BY ";

            std::size_t last_index{columns.size() - 1};

            for (std::size_t i{}; i < columns.size(); ++i)
            {
                m_query << columns[i];

                // Add comma to all except last
                if (i < last_index)
                {
                    m_query << ", ";
                }
            }

            m_query << " " << ((order == Order::Descending) ? "DESC" : "ASC");

            return *this;
        }
        /// @}

        /// @brief Limit the number of results in a query.
        ///
        /// @details
        /// This method assumes that the backend supports LIMIT clauses. However, it might need to be
        /// overriden in order to adapt it to backend-specific behaviour.
        ///
        /// @param[in] max_rows         Maximum number of rows to retrieve.
        virtual QueryBuilder& limit(std::size_t max_rows)
        {
            m_query << " LIMIT " << std::to_string(max_rows);
            return *this;
        }

        /// @brief Paginate a query.
        ///
        /// @details
        /// This method assumes that the backend supports LIMIT and OFFSET clauses. However, it might need to be
        /// overriden in order to adapt it to backend-specific behaviour.
        ///
        /// @param[in] page             Page index to fetch (zero-indexed).
        /// @param[in] page_size        Number of items to fit in a page (at most).
        virtual QueryBuilder& paginate(std::size_t page, std::size_t page_size)
        {
            m_query << " LIMIT " << std::to_string(page_size) << " OFFSET " << std::to_string(page * page_size);
            return *this;
        }

    protected:
        /// @brief Built query.
        std::ostringstream m_query;
    };
}  // namespace kouta::db::query
