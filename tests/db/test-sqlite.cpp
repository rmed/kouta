#include <format>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/db/client.hpp>
#include <kouta/db/error/sqlite.hpp>
#include <kouta/db/params/sqlite-params.hpp>
#include <kouta/db/abstract-adapter.hpp>
#include <kouta/utils/ilogger.hpp>

namespace kouta::tests::db
{
    using namespace kouta::db;

    class MockLogger : public virtual utils::ILogger
    {
    public:
        MOCK_METHOD(void, debug, (std::string_view msg), (override));
        MOCK_METHOD(void, info, (std::string_view msg), (override));
        MOCK_METHOD(void, warning, (std::string_view msg), (override));
        MOCK_METHOD(void, error, (std::string_view msg), (override));
        MOCK_METHOD(void, critical, (std::string_view msg), (override));
    };

    class Adapter : public AbstractAdapter
    {
    public:
        Adapter()
            : AbstractAdapter{}
        {
        }

        template<class TFunctor>
        std::invoke_result_t<TFunctor, soci::session&> run_query(TFunctor&& functor)
        {
            return session_run(functor);
        }
    };

    class Tester
    {
    public:
        Tester(std::size_t pool_size)
            : m_client{pool_size}
            , m_adapter{}
            , m_logger{}
        {
            m_client.set_logger(&m_logger);
            m_adapter.set_logger(&m_logger);
            m_adapter.set_backend_error_handler(error::sqlite::handler);
        }

        Client& client()
        {
            return m_client;
        }

        Adapter& adapter()
        {
            return m_adapter;
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
                m_adapter.set_pool(m_client.pool());
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
                m_adapter.set_pool(m_client.pool());
            }
        }

    private:
        Client m_client;
        Adapter m_adapter;
        MockLogger m_logger;
    };

    /// @brief Test the connection to an SQLite database (in memory).
    ///
    /// @details
    /// The test succeeds if the connection succeeds.
    TEST(DbTest, SqliteConnection)
    {
        Tester tester{5};
        tester.connect(":memory:");

        ASSERT_EQ(tester.client().backend(), Client::Backend::Sqlite);
        ASSERT_EQ(tester.client().initialized(), true);
    }

    /// @brief Test the error handling when opening a nonexistent database.
    ///
    /// @details
    /// The test succeeds if errors are detected.
    TEST(DbTest, SqliteNonExistent)
    {
        Tester tester{5};

        // Read-only db
        tester.connect("nonexistentfiledatabase", {.readonly = true});

        ASSERT_EQ(tester.client().backend(), Client::Backend::None);
        ASSERT_EQ(tester.client().initialized(), false);

        // Regular db on non-existent directory
        tester.connect("/this/path/does/not/exist/test.db");

        ASSERT_EQ(tester.client().backend(), Client::Backend::None);
        ASSERT_EQ(tester.client().initialized(), false);
    }

    /// @brief Test the operations on an SQLite database (in memory).
    ///
    /// @details
    /// The test succeeds if all queries are executed correctly.
    TEST(DbTest, SqliteOperation)
    {
        Tester tester{5};
        tester.connect(":memory:");

        // Create tables
        {
            Result<> result = tester.adapter().run_query(
                [](soci::session& s) -> Result<>
                {
                    {
                        soci::ddl_type ddl = s.create_table("table1");
                        ddl.column("id", soci::dt_integer);
                        ddl.column("name", soci::dt_string);
                        ddl.primary_key("table1_pk", "id");
                    }

                    {
                        soci::ddl_type ddl = s.create_table("table2");
                        ddl.column("id", soci::dt_integer);
                        ddl.column("address1", soci::dt_string)("not null");
                        ddl.column("floor", soci::dt_integer);
                        ddl.column("door", soci::dt_string);
                        ddl.primary_key("table2_pk", "id");
                    }

                    return {ResultCode::Ok};
                });

            ASSERT_EQ(result.code, ResultCode::Ok);
        }

        // Insert data
        {
            Result<int> result = tester.adapter().run_query(
                [](soci::session& s) -> Result<int>
                {
                    for (int i{}; i < 10; ++i)
                    {
                        s << "INSERT INTO table1(id, name) values(:id, :name)", soci::use(i),
                            soci::use(std::format("name {}", i));
                    }

                    int count;
                    s << "select count(*) from table1", soci::into(count);

                    return {ResultCode::Ok, 0, count};
                });

            ASSERT_EQ(result.code, ResultCode::Ok);
            ASSERT_EQ(result.data.value(), 10);
        }

        // Check error in invalid query
        //
        // Will result in an UnknownError because SQLite-specific errors are not handled.
        {
            Result<> result = tester.adapter().run_query(
                [](soci::session& s) -> Result<>
                {
                    s << "INSERT INTO table3(id) values(42)";

                    return {ResultCode::Ok};
                });

            ASSERT_EQ(result.code, ResultCode::DatabaseBackendError);
            ASSERT_EQ(result.error_detail, 1);  // Generic SQLite error
        }
    }
}  // namespace kouta::tests::db
