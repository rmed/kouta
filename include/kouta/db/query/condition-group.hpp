#pragma once

#include <format>
#include <sstream>
#include <string_view>
#include <variant>
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
        /// @brief Value types that may be provided to condition functions.
        ///
        /// @details
        /// All of the values provided must be convertible to a string. Note that the values are converted as-is. In the
        /// case of string values, these may be actual values or placeholders to set later on.
        using ValueType = std::variant<
            std::uint8_t,
            std::int8_t,
            std::uint16_t,
            std::int16_t,
            std::uint32_t,
            std::int32_t,
            std::uint64_t,
            std::int64_t,
            double,
            std::string>;

        /// @brief Default constructor
        ConditionGroup() = default;

        // Copyable
        ConditionGroup(const ConditionGroup& other)
        {
            m_has_conditions = other.m_has_conditions;
            m_conditions.str("");
            m_conditions << other.m_conditions.str();
        }

        ConditionGroup& operator=(const ConditionGroup& other)
        {
            m_has_conditions = other.m_has_conditions;
            m_conditions.str("");
            m_conditions << other.m_conditions.str();

            return *this;
        }

        // Movable
        ConditionGroup(ConditionGroup&&) = default;
        ConditionGroup& operator=(ConditionGroup&&) = default;

        /// @brief Determine whether the builder has any conditions
        explicit operator bool() const noexcept
        {
            return m_has_conditions;
        }

        virtual ~ConditionGroup() = default;

        /// @brief Build the final SQL condition group for the query.
        std::string build() const
        {
            return std::format("({})", m_conditions.str());
        }

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

        /// @brief Add a parameterized AND condition.
        ///
        /// @param[in] item             Item that must meet the condition.
        /// @param[in] condition        Condition that must be met.
        /// @param[in] value            Value related to the condition.
        virtual ConditionGroup& and_(std::string_view item, std::string_view condition, const ValueType& value)
        {
            if (m_has_conditions)
            {
                m_conditions << " AND ";
            }

            m_conditions << item << " " << condition << " " << parse_value_type(value);
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

        /// @brief Add a parameterized OR condition.
        ///
        /// @param[in] item             Item that must meet the condition.
        /// @param[in] condition        Condition that must be met.
        /// @param[in] value            Value related to the condition.
        virtual ConditionGroup& or_(std::string_view item, std::string_view condition, const ValueType& value)
        {
            if (m_has_conditions)
            {
                m_conditions << " OR ";
            }

            m_conditions << item << " " << condition << " " << parse_value_type(value);
            m_has_conditions = true;

            return *this;
        }

        /// @brief Add an IN condition.
        ///
        /// @param[in] column           Column whose value should be in the set of values.
        /// @param[in] values           Values that should contain the column. May be raw values, or placeholders.
        /// @{
        ConditionGroup& in(std::string_view column, std::initializer_list<ValueType> values)
        {
            return in(column, std::vector<ValueType>{values});
        }

        ConditionGroup& in(std::string_view column, const std::vector<ValueType>& values)
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
                m_conditions << parse_value_type(values[i]);

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
        ConditionGroup& not_in(std::string_view column, std::initializer_list<ValueType> values)
        {
            return not_in(column, std::vector<ValueType>{values});
        }

        ConditionGroup& not_in(std::string_view column, const std::vector<ValueType>& values)
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
                m_conditions << parse_value_type(values[i]);

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

    private:
        /// @brief Parse a value type into a string.
        ///
        /// @param[in] value        Value to parse.
        ///
        /// @returns String representation. String values are returned as-is.
        std::string parse_value_type(const ValueType& value)
        {
            return std::visit(
                [](auto&& arg)
                {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>)
                    {
                        // Numeric value
                        return std::to_string(arg);
                    }
                    else
                    {
                        // String value
                        return arg;
                    }
                },
                value);
        }

        bool m_has_conditions;
        std::ostringstream m_conditions;
    };
}  // namespace kouta::db::query
