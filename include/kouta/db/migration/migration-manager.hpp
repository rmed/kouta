#pragma once

#include <format>

#include <soci/connection-pool.h>

#include <kouta/utils/logger-aware.hpp>

#include "abstract-version.hpp"

namespace kouta::db::migration
{
    namespace migration_manager_detail
    {
        /// @brief Query used to retrieve the installed version.
        constexpr std::string_view QUERY_GET_VERSION("SELECT version FROM {}");

        /// @brief Query used to remove versions.
        constexpr std::string_view QUERY_DELETE_VERSION{"DELETE FROM {}"};

        /// @brief Query used to insert a new version.
        constexpr std::string_view QUERY_INSERT_VERSION{"INSERT INTO {} VALUES (:id)"};
    }  // namespace migration_manager_detail

    /// @brief Default database table in which to store currently installed schema.
    constexpr std::string_view DEFAULT_SCHEMA_VERSION_TABLE{"_schema_versions_"};

    /// @brief Simple database schema migration manager.
    ///
    /// @details
    /// This class orchestrates an ordered list of @ref AbstractVersion pointers that are invoked in order to upgrade or
    /// downgrade the database schema. Note that the order in which versions are registered matters, as their version
    /// identifier will be computed based on their position in the list.
    ///
    /// Versions may be registered via the @ref versions() method, or @ref add_version() and @ref add_versions().
    class MigrationManager : public kouta::utils::LoggerAware
    {
    public:
        /// @brief Database connection pool type.
        using Pool = soci::connection_pool;

        /// @brief Container for the versions.
        using VersionContainer = std::vector<AbstractVersion*>;

        /// @brief Default constructor.
        ///
        /// @details
        /// Uses @ref DEFAULT_SCHEMA_VERSION_TABLE as the name of the table used to store installed version.
        MigrationManager()
            : MigrationManager{DEFAULT_SCHEMA_VERSION_TABLE}
        {
        }

        /// @brief Constructor specifying table to use.
        ///
        /// @param[in] table            Name of the table to use.
        explicit MigrationManager(std::string_view table)
            : m_pool{nullptr}
            , m_versions{}
            , m_table{table}
        {
        }

        // Copyable
        MigrationManager(const MigrationManager&) = default;
        MigrationManager& operator=(const MigrationManager&) = default;

        // Movable
        MigrationManager(MigrationManager&&) = default;
        MigrationManager& operator=(MigrationManager&&) = default;

        virtual ~MigrationManager() = default;

        /// @brief Obtain a reference to the version container.
        /// @{
        const VersionContainer& versions() const
        {
            return m_versions;
        }

        VersionContainer& versions()
        {
            return m_versions;
        }
        /// @}

        /// @brief Set the internal pointer to the connection pool.
        ///
        /// @param[in] pool             Pointer to the connection pool. Set to `nullptr` to disable the adapter.
        void set_pool(Pool* pool)
        {
            m_pool = pool;
        }

        /// @brief Add a new version at the end of the list.
        ///
        /// @param[in] version          Pointer to the version to add.
        void add_version(AbstractVersion* version)
        {
            m_versions.emplace_back(version);
        }

        /// @brief Add a set of versions at the end of the list.
        ///
        /// @param[in] version          Pointer to the version to add.
        void add_versions(std::initializer_list<AbstractVersion*> versions)
        {
            m_versions.insert(m_versions.end(), versions);
        }

        /// @brief Retrieve currently installed version.
        ///
        /// @note This should be overriden to account for specific database behaviour.
        ///
        /// @param[in] session          Database session.
        ///
        /// @returns Currently installed version, or -1 if no version is installed. std::nullopt is returned if the
        /// table does not exist (or an error was encountered).
        virtual std::optional<int> retrieve_installed_version(soci::session& session)
        {
            try
            {
                int result{};
                session << std::format(migration_manager_detail::QUERY_GET_VERSION, m_table), soci::into(result);

                if (session.got_data())
                {
                    return std::make_optional(result);
                }

                return std::make_optional(-1);
            }
            catch (const soci::soci_error& e)
            {
                log_error(std::format("Failed to retrieve version: {}", e.what()));
                return std::nullopt;
            }
        }

        /// @brief Upgrade to the latest version registered.
        ///
        /// @returns true if the upgrade process succeeded, otherwise false.
        bool upgrade()
        {
            return upgrade(m_versions.size() - 1);
        }

        /// @brief Upgrade to a specific version number.
        ///
        /// @details
        /// The version will be translated to the position of the @ref AbstractVersion pointer in the internal list.
        /// This method will:
        ///
        /// 1. Check the identifier of the currently installed version (if table is found).
        /// 2. Install versions until the one requested is successfully installed.
        ///
        /// @note The pool must have been configured beforehand.
        ///
        /// @param[in] id               Identifier of the version to install.
        ///
        /// @returns true if the upgrade process succeeded, otherwise false.
        bool upgrade(std::size_t id)
        {
            log_info(std::format("Requested upgrade to version {}", id));

            if (id >= m_versions.size())
            {
                log_critical(std::format("There are only {} versions in the list", m_versions.size()));
                return false;
            }

            if (m_pool == nullptr)
            {
                log_critical("Connection pool has not been configured");
                return false;
            }

            soci::session session{*m_pool};

            // Check installed version
            log_info(std::format("Retrieving installed version from {}", m_table));

            std::optional<int> installed{retrieve_installed_version(session)};
            int start_id{};

            if (!installed.has_value())
            {
                // Failed to retrieve version, attempt to create table
                log_info(std::format("Creating schema table {}", m_table));

                if (!create_schema_table(session))
                {
                    return false;
                }

                log_info("Table created successfully");

                start_id = 0;
            }
            else if (installed.value() >= 0)
            {
                log_info(std::format("Installed version: {}", installed.value()));

                // Check if already in a recent version
                if (installed >= id)
                {
                    log_info("Installed version is higher or equal than requested version. Nothing to do");
                    return true;
                }

                start_id = installed.value() + 1;
            }

            auto installed_it{m_versions.begin()};
            std::advance(installed_it, start_id);

            auto end_it{m_versions.begin()};
            std::advance(end_it, id);

            while (installed_it <= end_it)
            {
                int to_install = std::distance(m_versions.begin(), installed_it);
                log_info(std::format("Installing version {}", to_install));

                if (!(*installed_it)->upgrade(session))
                {
                    log_critical("Failed to install version");
                    return false;
                }

                if (!delete_installed_version(session))
                {
                    log_critical("Failed to delete installed version");
                    return false;
                }

                if (!add_installed_version(session, to_install))
                {
                    log_critical("Failed to mark version as installed");
                    return false;
                }

                log_info("Version installed");

                ++installed_it;
            }

            return true;
        }

        /// @brief Downgrade a specific version number.
        ///
        /// @details
        /// The version will be translated to the position of the @ref AbstractVersion pointer in the internal list.
        /// This method will:
        ///
        /// 1. Check the identifier of the currently installed version (if table is found).
        /// 2. Downgrade all versions in order, including the one specified in @p id.
        ///
        /// @note The pool must have been configured beforehand.
        ///
        /// @param[in] id               Identifier of the version to downgrade.
        ///
        /// @returns true if the downgrade process succeeded, otherwise false.
        bool downgrade(std::size_t id)
        {
            log_info(std::format("Requested downgrade of version {}", id));

            if (id >= m_versions.size())
            {
                log_critical(std::format("There are only {} versions in the list", m_versions.size()));
                return false;
            }

            if (m_pool == nullptr)
            {
                log_critical("Connection pool has not been configured");
                return false;
            }

            soci::session session{*m_pool};

            // Check installed version
            log_info(std::format("Retrieving installed version from {}", m_table));

            std::optional<int> installed{retrieve_installed_version(session)};

            if (!installed.has_value())
            {
                return false;
            }

            log_info(std::format("Installed version: {}", installed.value()));

            // Check if version is not installed
            if (id > installed)
            {
                log_info("Installed version is lower than requested downgrade. Nothing to do");
                return true;
            }

            auto installed_it{m_versions.begin()};
            std::advance(installed_it, installed.value());

            auto end_it{m_versions.begin()};
            std::advance(end_it, id);

            while (installed_it >= end_it)
            {
                int to_remove = std::distance(m_versions.begin(), installed_it);
                log_info(std::format("Downgrading version {}", to_remove));

                if (!(*installed_it)->downgrade(session))
                {
                    log_critical("Failed to downgrade version");
                    return false;
                }

                if (!delete_installed_version(session))
                {
                    log_critical("Failed to delete installed version");
                    return false;
                }

                if (to_remove > 0 && !add_installed_version(session, to_remove - 1))
                {
                    log_critical("Failed to mark version as removed");
                    return false;
                }

                log_info("Version downgraded");

                --installed_it;
            }

            return true;
        }

    protected:
        /// @brief Create the schema table.
        ///
        /// @note This should be overriden to account for specific database behaviour.
        ///
        /// @param[in] session          Database session.
        ///
        /// @returns true if the creation succeeded, otherwise false.
        virtual bool create_schema_table(soci::session& session)
        {
            try
            {
                {
                    soci::ddl_type ddl = session.create_table(m_table);
                    ddl.column("version", soci::dt_integer);
                }
            }
            catch (const soci::soci_error& e)
            {
                log_error(std::format("Creation error: {}", e.what()));
                return false;
            }

            return true;
        }

        /// @brief Delete currently installed version.
        ///
        /// @note This should be overriden to account for specific database behaviour.
        ///
        /// @param[in] session          Database session.
        ///
        /// @returns Whether the operation was successful.
        virtual bool delete_installed_version(soci::session& session)
        {
            try
            {
                session << std::format(migration_manager_detail::QUERY_DELETE_VERSION, m_table);
            }
            catch (const soci::soci_error& e)
            {
                log_error(std::format("Failed to delete version: {}", e.what()));
                return false;
            }

            return true;
        }

        /// @brief Set the currently installed version.
        ///
        /// @note This should be overriden to account for specific database behaviour.
        ///
        /// @param[in] session          Database session.
        /// @param[in] id               ID of the version to add.
        ///
        /// @returns Whether the operation was successful.
        virtual bool add_installed_version(soci::session& session, int id)
        {
            try
            {
                session << std::format(migration_manager_detail::QUERY_INSERT_VERSION, m_table), soci::use(id, "id");
            }
            catch (const soci::soci_error& e)
            {
                log_error(std::format("Failed to insert version: {}", e.what()));
                return false;
            }

            return true;
        }

    private:
        /// @brief Pointer to the connection pool used.
        Pool* m_pool;

        /// @brief Ordered list of versions to manage.
        VersionContainer m_versions;

        /// @brief Name of the table to use in order to store schema versions.
        std::string m_table;
    };
}  // namespace kouta::db::migration
