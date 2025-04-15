#pragma once

#include <functional>
#include <sstream>

#include "condition-group.hpp"

namespace kouta::db::query
{
    /// @brief SQL WHERE clause builder.
    ///
    /// @details
    /// The conditions are provided as-is to the query, so placeholders must be explicitly specified by the user.
    ///
    /// This class assumes generic SQL, but it may be derived from in order to override the methods to account for
    /// backend-specific behaviour.
    ///
    /// @note This class follows the builder pattern.
    class WhereBuilder
    {
    public:
        /// @brief Default constructor
        WhereBuilder() = default;

        // Copyable
        WhereBuilder(const WhereBuilder&) = delete;
        WhereBuilder& operator=(const WhereBuilder&) = delete;

        // Movable
        WhereBuilder(WhereBuilder&&) = default;
        WhereBuilder& operator=(WhereBuilder&&) = default;

        virtual ~WhereBuilder() = default;

        /// @brief Add an AND condition.
        ///
        /// @details
        /// This method expects a @p builder function that will receive a @ref ConditionGroup object that, after the
        /// builder function has been called, will result in the conditional clauses.
        ///
        /// @param[in] builder              Function in which the clause will be built. Receives a @ref ConditionGroup.
        WhereBuilder& and_(std::function<void(ConditionGroup&)>&& builder)
        {
            ConditionGroup group{};
            builder(group);

            if (m_has_conditions)
            {
                m_conditions << " AND ";
            }

            m_conditions << group.build();
            m_has_conditions = true;

            return *this;
        }

        /// @brief Add an OR condition.
        ///
        /// @details
        /// This method expects a @p builder function that will receive a @ref ConditionGroup object that, after the
        /// builder function has been called, will result in the conditional clauses.
        ///
        /// @param[in] builder              Function in which the clause will be built. Receives a @ref ConditionGroup.
        WhereBuilder& or_(std::function<void(ConditionGroup&)>&& builder)
        {
            ConditionGroup group{};
            builder(group);

            if (m_has_conditions)
            {
                m_conditions << " OR ";
            }

            m_conditions << group.build();
            m_has_conditions = true;

            return *this;
        }

        /// @brief Build the final SQL WHERE clause for the query.
        std::string build() const
        {
            return m_conditions.str();
        }

    private:
        bool m_has_conditions;
        std::ostringstream m_conditions;
    };
}  // namespace kouta::db::query
