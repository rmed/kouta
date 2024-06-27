#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <kouta/io/timer.hpp>

namespace kouta::tests::io
{
    using namespace kouta::io;

    namespace
    {
        /// @brief Mock the Root to provide the event loop and receive callbacks.
        class RootMock : public base::Root
        {
        public:
            MOCK_METHOD(void, handler_timeout, (Timer &), ());
        };

        /// @brief Mock the Root to provide the event loop and also stop tests after some time.
        class RootMockTimed : public base::Root
        {
        public:
            explicit RootMockTimed(std::chrono::milliseconds timeout)
                : base::Root{}
                , m_test_timeout{this, timeout, std::bind_front(&RootMockTimed::handle_test_timeout, this)}
            {
            }

            MOCK_METHOD(void, handler_timeout, (Timer &), ());

            /// @brief Add a timeout to the test.
            void run() override
            {
                m_test_timeout.start();
                Root::run();
            }

        private:
            /// @brief Stop the test after a while
            void handle_test_timeout(Timer &)
            {
                stop();
            }

            Timer m_test_timeout;
        };
    }  // namespace

    /// @brief Test the nominal behaviour of the timer.
    ///
    /// @details
    /// The test succeeds if all values are correctly added to the packer.
    TEST(IoTest, TimerElapsed)
    {
        RootMock root{};
        std::chrono::milliseconds timeout{200};

        Timer timer{&root, timeout, std::bind_front(&RootMock::handler_timeout, &root)};

        auto now{std::chrono::system_clock::now()};

        EXPECT_CALL(root, handler_timeout)
            .WillOnce(
                [&timeout, &now, &root]()
                {
                    auto after_timeout{std::chrono::system_clock::now()};

                    ASSERT_TRUE((now + timeout) <= after_timeout);

                    root.stop();
                });

        timer.start();

        alarm(2);
        root.run();
        alarm(0);
    }

    /// @brief Test the behaviour of the timer when it is not started.
    ///
    /// @details
    /// The test succeeds if the timer never ticks.
    TEST(IoTest, TimerNotStarted)
    {
        RootMockTimed root{std::chrono::milliseconds{500}};
        std::chrono::milliseconds timeout{100};

        Timer timer{&root, timeout, std::bind_front(&RootMockTimed::handler_timeout, &root)};

        auto now{std::chrono::system_clock::now()};

        EXPECT_CALL(root, handler_timeout).Times(0);

        alarm(2);
        root.run();
        alarm(0);
    }

    /// @brief Test the behaviour of the timer when stopped.
    ///
    /// @details
    /// The test succeeds if the timer never ticks.
    TEST(IoTest, TimerStopped)
    {
        RootMockTimed root{std::chrono::milliseconds{500}};
        std::chrono::milliseconds timeout{100};

        Timer timer{&root, timeout, std::bind_front(&RootMockTimed::handler_timeout, &root)};

        auto now{std::chrono::system_clock::now()};

        EXPECT_CALL(root, handler_timeout).Times(0);

        timer.start();
        timer.stop();

        alarm(2);
        root.run();
        alarm(0);
    }

    /// @brief Test that the timer only ticks once unless explicitly rearmed.
    ///
    /// @details
    /// The test succeeds if the timer ticks once.
    TEST(IoTest, TimerElapsedOnce)
    {
        RootMockTimed root{std::chrono::milliseconds{500}};
        std::chrono::milliseconds timeout{100};

        Timer timer{&root, timeout, std::bind_front(&RootMockTimed::handler_timeout, &root)};

        auto now{std::chrono::system_clock::now()};

        EXPECT_CALL(root, handler_timeout).Times(1);

        timer.start();

        alarm(2);
        root.run();
        alarm(0);
    }

    /// @brief Test that the timer can be rearmed.
    ///
    /// @details
    /// The test succeeds if the timer ticks twice.
    TEST(IoTest, TimerRearmed)
    {
        RootMockTimed root{std::chrono::milliseconds{500}};
        std::chrono::milliseconds timeout{100};

        Timer timer{&root, timeout, std::bind_front(&RootMockTimed::handler_timeout, &root)};

        auto now{std::chrono::system_clock::now()};

        EXPECT_CALL(root, handler_timeout)
            .WillOnce(
                [&timer]()
                {
                    // Rearm
                    timer.start();
                })
            .WillOnce(
                [&root]()
                {
                    root.stop();
                });

        timer.start();

        alarm(2);
        root.run();
        alarm(0);
    }

    /// @brief Test that the timer can be rearmed with a different duration.
    ///
    /// @details
    /// The test succeeds if the timer ticks twice.
    TEST(IoTest, TimerRearmedDifferentDuration)
    {
        RootMockTimed root{std::chrono::milliseconds{1000}};
        std::chrono::milliseconds timeout{100};
        std::chrono::milliseconds timeout2{500};

        Timer timer{&root, timeout, std::bind_front(&RootMockTimed::handler_timeout, &root)};

        auto now{std::chrono::system_clock::now()};

        EXPECT_CALL(root, handler_timeout)
            .WillOnce(
                [&timer, &now, &timeout, &timeout2]()
                {
                    auto after_timeout{std::chrono::system_clock::now()};

                    ASSERT_TRUE((now + timeout) <= after_timeout);

                    // Rearm
                    timer.set_duration(std::chrono::milliseconds{timeout2});
                    timer.start();
                })
            .WillOnce(
                [&root, &now, &timeout, &timeout2]()
                {
                    auto after_timeout{std::chrono::system_clock::now()};

                    ASSERT_TRUE((now + timeout + timeout2) <= after_timeout);

                    root.stop();
                });

        timer.start();

        alarm(2);
        root.run();
        alarm(0);
    }
}  // namespace kouta::tests::io
