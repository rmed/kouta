#pragma once

#include <soci/session.h>

#include <kouta/utils/logger-aware.hpp>

namespace kouta::db::migration
{
    /// @brief Abstract migration version definition.
    ///
    /// @details
    /// A version consists on a couple of methods @ref upgrade() and @ref downgrade() that are invoked in order to
    /// modify a database schema.
    class AbstractVersion : public kouta::utils::LoggerAware
    {
    public:
        // Default constructor
        AbstractVersion() = default;

        // Copyable
        AbstractVersion(const AbstractVersion&) = default;
        AbstractVersion& operator=(const AbstractVersion&) = default;

        // Movable
        AbstractVersion(AbstractVersion&&) = default;
        AbstractVersion& operator=(AbstractVersion&&) = default;

        virtual ~AbstractVersion() = default;

        /// @brief Upgrade method.
        ///
        /// @details
        /// This method should be overriden in order to implement the upgrade behaviour.
        ///
        /// @returns true if the upgrade succeeded (default), otherwise false.
        virtual bool upgrade(soci::session& session)
        {
            return true;
        }

        /// @brief Downgrade method.
        ///
        /// @details
        /// This method should be overriden in order to implement the downgrade behaviour.
        ///
        /// @returns true if the downrade succeeded (default), otherwise false.
        virtual bool downgrade(soci::session& session)
        {
            return true;
        }
    };
}  // namespace kouta::db::migration
