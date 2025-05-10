#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/db/query/query-builder.hpp>

namespace kouta::tests::db::query
{
    using namespace kouta::db::query;

    /// @brief Test SELECT queries using the query builder.
    ///
    /// @details
    /// The test succeeds if all queries are correct.
    TEST(DbTest, SelectQuery)
    {
        QueryBuilder builder{};

        // Select all columns
        builder.select("mytable1", {});

        ASSERT_EQ(builder.query(), "SELECT * FROM mytable1");

        builder.select("mytable1", {"COUNT(*)"});

        ASSERT_EQ(builder.query(), "SELECT COUNT(*) FROM mytable1");

        // Select some columns
        builder.select("mytable2", {"col1", "col2", "col3"});

        ASSERT_EQ(builder.query(), "SELECT col1, col2, col3 FROM mytable2");

        // Select all columns with conditions
        builder.select("mytable3", {})
            .where(
                [](WhereBuilder& b)
                {
                    b.and_(
                         [](ConditionGroup& c)
                         {
                             c.and_("col1 = :col1");
                         })
                        .and_(
                            [](ConditionGroup& c)
                            {
                                c.or_("col2 > :col2_1").or_("col2 < :col2_2");
                            });
                });

        ASSERT_EQ(
            builder.query(), "SELECT * FROM mytable3 WHERE (col1 = :col1) AND (col2 > :col2_1 OR col2 < :col2_2)");

        // Select some columns with conditions
        builder.select("mytable4", {"col5", "col9"})
            .where(
                [](WhereBuilder& b)
                {
                    b.and_(
                         [](ConditionGroup& c)
                         {
                             c.and_("col1 <= :col1");
                         })
                        .and_(
                            [](ConditionGroup& c)
                            {
                                c.in("col3", {":col3_1", ":col3_2", ":col3_3"});
                            })
                        .or_(
                            [](ConditionGroup& c)
                            {
                                c.and_("col2 = :col2");
                            });
                });

        ASSERT_EQ(
            builder.query(),
            "SELECT col5, col9 FROM mytable4 WHERE (col1 <= :col1) AND (col3 IN (:col3_1, :col3_2, :col3_3)) OR (col2 "
            "= "
            ":col2)");

        // Limit a query
        builder.select("mytable5", {})
            .where(
                [](WhereBuilder& b)
                {
                    b.and_(
                        [](ConditionGroup& c)
                        {
                            c.and_("col1 = :col1");
                        });
                })
            .limit(4);

        ASSERT_EQ(builder.query(), "SELECT * FROM mytable5 WERE col1 = :col1 LIMIT 4");

        // Paginate a query
        builder.select("mytable5", {}).paginate(0, 1000);

        ASSERT_EQ(builder.query(), "SELECT * FROM mytable5 LIMIT 1000 OFFSET 0");

        builder.select("mytable5", {}).paginate(3, 1000);

        ASSERT_EQ(builder.query(), "SELECT * FROM mytable5 LIMIT 1000 OFFSET 3000");

        // Order a query
        builder.select("mytable6", {}).order_by({"col1"}, QueryBuilder::Order::Ascending);

        ASSERT_EQ(builder.query(), "SELECT * FROM mytable6 ORDER BY col1 ASC");

        builder.select("mytable6", {"col1", "col2", "col3"}).order_by({"col1, col2"}, QueryBuilder::Order::Descending);

        ASSERT_EQ(builder.query(), "SELECT col1, col2, col3 FROM mytable6 ORDER BY col1, col2 DESC");

        // Join query
        builder.select("mytable7", {}).join("mytable8", {});

        ASSERT_EQ(builder.query(), "SELECT * FROM mytable7 JOIN mytable8");

        builder.select("mytable7", {}).join("mytable8", "mytable7.id = mytable8.parent_id");

        ASSERT_EQ(builder.query(), "SELECT * FROM mytable7 JOIN mytable8 ON mytable7.id = mytable8.parent_id");
    }

    /// @brief Test INSERT queries using the query builder.
    ///
    /// @details
    /// The test succeeds if all queries are correct.
    TEST(DbTest, InsertQuery)
    {
        QueryBuilder builder{};

        builder.insert("mytable1", {{"col1", ""}, {"col2", ""}, {"col3", ":col3"}, {"col4", "test"}});

        ASSERT_EQ(builder.query(), "INSERT INTO mytable1 (col1, col2, col3, col4) VALUES (:col1, :col2, :col3, :test)");
    }

    /// @brief Test UPDATE queries using the query builder.
    ///
    /// @details
    /// The test succeeds if all queries are correct.
    TEST(DbTest, UpdateQuery)
    {
        QueryBuilder builder{};

        builder.update("mytable1", {{"col1", ""}, {"col2", ""}, {"col3", ":col3"}, {"col4", "test"}});

        ASSERT_EQ(builder.query(), "UPDATE mytable1 SET col1 = :col1, col2 = :col2, col3 = :col3, col4 = :test");
    }

    /// @brief Test DELETE queries using the query builder.
    ///
    /// @details
    /// The test succeeds if all queries are correct.
    TEST(DbTest, DeleteQuery)
    {
        QueryBuilder builder{};

        // Single record
        builder.erase("mytable1")
            .where(
                [](WhereBuilder& b)
                {
                    b.and_(
                        [](ConditionGroup& c)
                        {
                            c.and_("id = :id");
                        });
                });

        ASSERT_EQ(builder.query(), "DELETE FROM mytable1 WHERE (id = :id)");

        // Multiple records
        builder.erase("mytable1")
            .where(
                [](WhereBuilder& b)
                {
                    b.and_(
                        [](ConditionGroup& c)
                        {
                            c.and_("date >= :date");
                        });
                });

        ASSERT_EQ(builder.query(), "DELETE FROM mytable1 WHERE (date >= :date)");
    }
}  // namespace kouta::tests::db::query
