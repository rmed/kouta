#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/db/client.hpp>
#include <kouta/db/migration/migration-manager.hpp>
#include <kouta/db/params/sqlite-params.hpp>
#include <kouta/utils/ilogger.hpp>

namespace kouta::tests::db::migration
{
    using namespace kouta::db;
    using namespace kouta::db::migration;

    class MockLogger : public virtual utils::ILogger
    {
    public:
        MOCK_METHOD(void, debug, (std::string_view msg), (override));
        MOCK_METHOD(void, info, (std::string_view msg), (override));
        MOCK_METHOD(void, warning, (std::string_view msg), (override));
        MOCK_METHOD(void, error, (std::string_view msg), (override));
        MOCK_METHOD(void, critical, (std::string_view msg), (override));
    };

    class MockVersion : public AbstractVersion
    {
    public:
        MOCK_METHOD(bool, upgrade, (soci::session&), (override));
        MOCK_METHOD(bool, downgrade, (soci::session&), (override));
    };

    class Tester
    {
    public:
        Tester(std::size_t pool_size)
            : m_client{pool_size}
            , m_migration_manager{}
            , m_logger{}
        {
            m_client.set_logger(&m_logger);
            m_migration_manager.set_logger(&m_logger);
        }

        Client& client()
        {
            return m_client;
        }

        MigrationManager& migration_manager()
        {
            return m_migration_manager;
        }

        MockLogger& logger()
        {
            return m_logger;
        }

        /// @brief Open a database.
        ///
        /// @param[in] db_path          Path to the database (or :memory: for memory db);
        void connect(std::string_view db_path)
        {
            if (m_client.connect_sqlite(db_path))
            {
                m_migration_manager.set_pool(m_client.pool());
            }
        }

        /// @brief Open a database with parameters.
        ///
        /// @param[in] db_path          Path to the database (or :memory: for memory db);
        /// @param[in] p                Database connection parameters.
        void connect(std::string_view db_path, const params::SqliteParams& p)
        {
            if (m_client.connect_sqlite(db_path, p))
            {
                m_migration_manager.set_pool(m_client.pool());
            }
        }

    private:
        Client m_client;
        MigrationManager m_migration_manager;
        MockLogger m_logger;
    };

    /// @brief Test the connection to an SQLite database (in memory).
    ///
    /// @details
    /// The test succeeds if the connection succeeds.
    TEST(DbTest, SqliteMigration)
    {
        ::testing::InSequence s;

        Tester tester{5};
        tester.connect(":memory:");

        MockVersion v0{};
        MockVersion v1{};
        MockVersion v2{};

        tester.migration_manager().add_versions({&v0, &v1, &v2});

        // Migrate to first
        EXPECT_CALL(v0, upgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        ASSERT_EQ(tester.migration_manager().upgrade(0), true);

        // Check already in latest version
        ASSERT_EQ(tester.migration_manager().upgrade(0), true);

        // Migrate to last
        EXPECT_CALL(v1, upgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        EXPECT_CALL(v2, upgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        ASSERT_EQ(tester.migration_manager().upgrade(2), true);

        // Check already in latest version
        ASSERT_EQ(tester.migration_manager().upgrade(2), true);

        // Upgrade to non-existent version
        ASSERT_EQ(tester.migration_manager().upgrade(100), false);

        // Downgrade latest version
        EXPECT_CALL(v2, downgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        ASSERT_EQ(tester.migration_manager().downgrade(2), true);

        // Check already downgraded
        ASSERT_EQ(tester.migration_manager().downgrade(2), true);

        // Downgrade all
        EXPECT_CALL(v1, downgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        EXPECT_CALL(v0, downgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        ASSERT_EQ(tester.migration_manager().downgrade(0), true);

        // Verify there is no version installed
        {
            soci::session session{*(tester.client().pool())};
            ASSERT_EQ(tester.migration_manager().retrieve_installed_version(session).value(), -1);
        }

        // Failed upgrade
        EXPECT_CALL(v0, upgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return false;
                });

        ASSERT_EQ(tester.migration_manager().upgrade(1), false);

        // Failed upgrade
        EXPECT_CALL(v0, upgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        EXPECT_CALL(v1, upgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return true;
                });

        EXPECT_CALL(v2, upgrade)
            .WillOnce(
                [](soci::session&)
                {
                    return false;
                });

        ASSERT_EQ(tester.migration_manager().upgrade(2), false);

        {
            soci::session session{*(tester.client().pool())};
            ASSERT_EQ(tester.migration_manager().retrieve_installed_version(session).value(), 1);
        }
    }

}  // namespace kouta::tests::db::migration
