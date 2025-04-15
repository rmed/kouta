#pragma once

#include <format>
#include <sstream>
#include <string_view>
#include <vector>

namespace kouta::db::query
{
    /// @brief SQL condition grouping.
    ///
    /// @details
    /// This class is used to group together a set of SQL conditions to provide to a WHERE clause. Usually, this is not
    /// used directly, but rather via the @ref QueryBuilder class.
    ///
    /// The conditions are provided as-is to the query, so placeholders must be explicitly specified by the user.
    ///
    /// This class assumes generic SQL, but it may be derived from in order to override the methods to account for
    /// backend-specific behaviour.
    ///
    /// @note This class follows the builder pattern.
    class ConditionGroup
    {
    public:
        /// @brief Default constructor
        ConditionGroup() = default;

        // Copyable
        ConditionGroup(const ConditionGroup&) = delete;
        ConditionGroup& operator=(const ConditionGroup&) = delete;

        // Movable
        ConditionGroup(ConditionGroup&&) = default;
        ConditionGroup& operator=(ConditionGroup&&) = default;

        virtual ~ConditionGroup() = default;

        /// @brief Add an AND condition.
        ///
        /// @param[in] condition            Condition to add.
        virtual ConditionGroup& and_(std::string_view condition)
        {
            if (m_has_conditions)
            {
                m_conditions << " AND ";
            }

            m_conditions << condition;
            m_has_conditions = true;

            return *this;
        }

        /// @brief Add an OR condition.
        ///
        /// @param[in] condition            Condition to add.
        virtual ConditionGroup& or_(std::string_view condition)
        {
            if (m_has_conditions)
            {
                m_conditions << " OR ";
            }

            m_conditions << condition;
            m_has_conditions = true;

            return *this;
        }

        /// @brief Add an IN condition.
        ///
        /// @param[in] column           Column whose value should be in the set of values.
        /// @param[in] values           Values that should contain the column. May be raw values, or placeholders.
        /// @{
        ConditionGroup& in(std::string_view column, std::initializer_list<std::string_view> values)
        {
            return in(column, std::vector<std::string_view>{values});
        }

        ConditionGroup& in(std::string_view column, const std::vector<std::string_view>& values)
        {
            if (m_has_conditions)
            {
                m_conditions << " AND ";
            }

            m_has_conditions = true;

            m_conditions << column << " IN (";

            std::size_t last_index = values.size() - 1;

            for (std::size_t i{}; i < values.size(); ++i)
            {
                m_conditions << values[i];

                if (i < last_index)
                {
                    // Add comma to all except last
                    m_conditions << ", ";
                }
            }

            m_conditions << ")";
            return *this;
        }
        /// @}

        /// @brief Add a NOT IN condition.
        ///
        /// @param[in] column           Column whose value should be in the set of values.
        /// @param[in] values           Values that should contain the column. May be raw values, or placeholders.
        /// @{
        ConditionGroup& not_in(std::string_view column, std::initializer_list<std::string_view> values)
        {
            return in(column, std::vector<std::string_view>{values});
        }

        ConditionGroup& not_in(std::string_view column, const std::vector<std::string_view>& values)
        {
            if (m_has_conditions)
            {
                m_conditions << " AND ";
            }

            m_has_conditions = true;

            m_conditions << column << " NOT IN (";

            std::size_t last_index = values.size() - 1;

            for (std::size_t i{}; i < values.size(); ++i)
            {
                m_conditions << values[i];

                if (i < last_index)
                {
                    // Add comma to all except last
                    m_conditions << ", ";
                }
            }

            m_conditions << ")";
            return *this;
        }
        /// @}

        /// @brief Build the final SQL condition group for the query.
        std::string build() const
        {
            return std::format("({})", m_conditions.str());
        }

    private:
        bool m_has_conditions;
        std::ostringstream m_conditions;
    };
}  // namespace kouta::db::query
