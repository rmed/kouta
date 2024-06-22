#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/base/branch.hpp>
#include <kouta/base/callback.hpp>
#include <kouta/base/root.hpp>

#include "dummy-component.hpp"

namespace kouta::tests::base
{
    /// @brief Mock the Root to provide the event loop and receive callbacks.
    class RootMock : public Root
    {
    public:
        MOCK_METHOD(void, handler_a, (std::uint16_t value), ());
        MOCK_METHOD(void, handler_b, (std::int32_t value_a, const std::string& value_b), ());
        MOCK_METHOD(void, handler_c, (const std::vector<std::uint8_t>& value), ());
        MOCK_METHOD(void, handler_d, (std::thread::id), ());
    };

    /// @brief Test the behaviour of the empty callback.
    ///
    /// @details
    /// The test succeeds if invoking the empty callback throws an exception.
    TEST(BaseTest, EmptyCallback)
    {
        Callback<std::uint16_t> cb{};

        EXPECT_THROW(cb(42), std::bad_function_call);
    }

    /// @brief Test the behaviour of the direct callback.
    ///
    /// @details
    /// The test succeeds if all callbacks are invoked in the correct order.
    TEST(BaseTest, DirectCallback)
    {
        ::testing::InSequence s;
        RootMock root{};

        callback::DirectCallback<std::uint16_t> cb_a{&root, &RootMock::handler_a};
        callback::DirectCallback<std::int32_t, const std::string&> cb_b{&root, &RootMock::handler_b};
        callback::DirectCallback<const std::vector<std::uint8_t>&> cb_c{&root, &RootMock::handler_c};

        std::uint16_t data_a{127};

        std::int32_t data_b_a{42};
        std::string data_b_b{"this is a test"};

        std::vector<std::uint8_t> data_c{1, 2, 4, 5, 7, 8, 9, 212, 48, 2, 84};

        EXPECT_CALL(root, handler_a(data_a)).Times(1);

        EXPECT_CALL(root, handler_b(data_b_a, data_b_b)).Times(1);

        EXPECT_CALL(root, handler_c(data_c)).Times(1);

        cb_a(data_a);
        cb_b(data_b_a, data_b_b);
        cb_c(data_c);
    }

    /// @brief Test the behaviour of the deferred callback.
    ///
    /// @details
    /// The test succeeds if all callbacks are invoked in the correct order.
    TEST(BaseTest, DeferredCallback)
    {
        ::testing::InSequence s;
        RootMock root{};

        callback::DeferredCallback<std::uint16_t> cb_a{&root, &RootMock::handler_a};
        callback::DirectCallback<std::int32_t, const std::string&> cb_b{&root, &RootMock::handler_b};
        callback::DeferredCallback<const std::vector<std::uint8_t>&> cb_c{&root, &RootMock::handler_c};

        std::uint16_t data_a{127};

        std::int32_t data_b_a{42};
        std::string data_b_b{"this is a test"};

        std::vector<std::uint8_t> data_c{1, 2, 4, 5, 7, 8, 9, 212, 48, 2, 84};
        auto data_c_copy{data_c};

        EXPECT_CALL(root, handler_b(data_b_a, data_b_b)).Times(1);

        EXPECT_CALL(root, handler_c(data_c_copy)).Times(1);

        EXPECT_CALL(root, handler_a(data_a))
            .WillOnce(
                [&root]
                {
                    root.stop();
                });

        // Clear the vector to verify that the data is copied.
        cb_c(data_c);
        data_c.clear();

        cb_a(data_a);
        cb_b(data_b_a, data_b_b);

        alarm(1);
        root.run();
        alarm(0);
    }

    /// @brief Test the behaviour of the callback list.
    ///
    /// @details
    /// The test succeeds if all callbacks are invoked in the correct order.
    TEST(BaseTest, CallbackList)
    {
        ::testing::InSequence s;
        RootMock root{};

        callback::CallbackList<std::uint16_t> cb_list_direct{
            callback::DirectCallback{&root, &RootMock::handler_a},
            callback::DirectCallback{&root, &RootMock::handler_a},
            callback::DirectCallback{&root, &RootMock::handler_a},
        };

        callback::CallbackList<std::int32_t, const std::string&> cb_list_deferred{
            callback::DeferredCallback{&root, &RootMock::handler_b},
            callback::DeferredCallback{&root, &RootMock::handler_b},
            callback::DeferredCallback{&root, &RootMock::handler_b}};

        std::uint16_t data_a{127};

        std::int32_t data_b_a{42};
        std::string data_b_b{"this is a test"};

        EXPECT_CALL(root, handler_a(data_a)).Times(3);

        EXPECT_CALL(root, handler_b(data_b_a, data_b_b)).Times(2);

        EXPECT_CALL(root, handler_b(data_b_a, data_b_b))
            .WillOnce(
                [&root]
                {
                    root.stop();
                });

        cb_list_deferred(data_b_a, data_b_b);
        cb_list_direct(data_a);

        alarm(1);
        root.run();
        alarm(0);
    }

    /// @brief Test the behaviour of the event loop when stopped.
    ///
    /// @details
    /// The test succeeds if no event is dispatched.
    TEST(BaseTest, PostStoppedEventLoop)
    {
        RootMock root{};

        EXPECT_CALL(root, handler_a).Times(0);

        EXPECT_CALL(root, handler_b).Times(0);

        EXPECT_CALL(root, handler_c).Times(0);

        root.post(&RootMock::handler_a, std::uint16_t{42});
        root.post(&RootMock::handler_b, -512, "This is a test");
        root.post(&RootMock::handler_c, std::vector<std::uint8_t>{12, 34});
    }

    /// @brief Test the behaviour of the event loop when running.
    ///
    /// @details
    /// The test succeeds if events are dispatched in the correct order.
    TEST(BaseTest, PostRunningEventLoop)
    {
        ::testing::InSequence s;
        RootMock root{};

        EXPECT_CALL(root, handler_a).Times(1);

        EXPECT_CALL(root, handler_b).Times(1);

        EXPECT_CALL(root, handler_c)
            .WillOnce(
                [&root]
                {
                    root.stop();
                });

        root.post(&RootMock::handler_a, std::uint16_t{42});
        root.post(&RootMock::handler_b, -512, "This is a test");
        root.post(&RootMock::handler_c, std::vector<std::uint8_t>{12, 34});

        alarm(1);
        root.run();
        alarm(0);
    }

    /// @brief Test the behaviour of the children when the event loop is stopped.
    ///
    /// @details
    /// The test succeeds if no deferred event is dispatched, but the direct ones are.
    TEST(BaseTest, ChildrenStoppedEventLoop)
    {
        RootMock root{};

        DummyComponent comp_a{
            &root,
            callback::DirectCallback{&root, &RootMock::handler_a},
            callback::DeferredCallback{&root, &RootMock::handler_b},
            callback::DeferredCallback{&root, &RootMock::handler_c}};

        DummyComponent comp_b{
            &comp_a,
            callback::DeferredCallback{&root, &RootMock::handler_a},
            callback::DeferredCallback{&root, &RootMock::handler_b},
            callback::DeferredCallback{&root, &RootMock::handler_c}};

        std::uint16_t data_a{42};

        EXPECT_CALL(root, handler_a(data_a)).Times(1);

        EXPECT_CALL(root, handler_b).Times(0);

        EXPECT_CALL(root, handler_c).Times(0);

        // Only this callback will be invoked, because it is direct
        comp_a.call_a(data_a);
        comp_a.call_b(-512, "this is a test");
        comp_a.call_c({1, 2, 3, 4, 5});

        comp_b.call_a(128);
        comp_b.call_b(-1000, "this is a test again");
        comp_b.call_c({9, 8, 7, 6, 5, 4, 3});
    }

    /// @brief Test the behaviour of the children when the event loop is running.
    ///
    /// @details
    /// The test succeeds if all events are dispatched in order.
    TEST(BaseTest, ChildrenRunningEventLoop)
    {
        ::testing::InSequence s;

        RootMock root{};

        DummyComponent comp_a{
            &root,
            callback::DirectCallback{&root, &RootMock::handler_a},
            callback::DeferredCallback{&root, &RootMock::handler_b},
            callback::DeferredCallback{&root, &RootMock::handler_c}};

        DummyComponent comp_b{
            &comp_a,
            callback::DeferredCallback{&root, &RootMock::handler_a},
            callback::DeferredCallback{&root, &RootMock::handler_b},
            callback::DeferredCallback{&root, &RootMock::handler_c}};

        std::uint16_t data_a_a{42};
        std::int32_t data_a_b1{-512};
        std::string data_a_b2{"this is a test"};
        std::vector<std::uint8_t> data_a_c{1, 2, 3, 4, 5, 6, 7};

        std::uint16_t data_b_a{128};
        std::int32_t data_b_b1{-1000};
        std::string data_b_b2{"this is another test"};
        std::vector<std::uint8_t> data_b_c{9, 8, 7, 6, 5, 4};

        // Direct callback
        EXPECT_CALL(root, handler_a(data_a_a)).Times(1);

        EXPECT_CALL(root, handler_b(data_a_b1, data_a_b2)).Times(1);

        EXPECT_CALL(root, handler_c(data_a_c)).Times(1);

        EXPECT_CALL(root, handler_a(data_b_a)).Times(1);

        EXPECT_CALL(root, handler_b(data_b_b1, data_b_b2)).Times(1);

        EXPECT_CALL(root, handler_c(data_b_c))
            .WillOnce(
                [&root]
                {
                    root.stop();
                });

        comp_a.call_b(data_a_b1, data_a_b2);
        comp_a.call_c(data_a_c);

        comp_b.call_a(data_b_a);
        comp_b.call_b(data_b_b1, data_b_b2);
        comp_b.call_c(data_b_c);

        // This callback will be invoked first
        comp_a.call_a(data_a_a);

        alarm(1);
        root.run();
        alarm(0);
    }

    /// @brief Test the behaviour of a component in a worker thread.
    ///
    /// @details
    /// The test succeeds if all events are dispatched in order.
    TEST(BaseTest, BranchComponent)
    {
        ::testing::InSequence s;

        RootMock root{};
        Branch<DummyComponent> worker{
            &worker,
            callback::DirectCallback{&root, &RootMock::handler_a},
            callback::DeferredCallback{&root, &RootMock::handler_b},
            callback::DeferredCallback{&root, &RootMock::handler_c},
            callback::DeferredCallback{&root, &RootMock::handler_d}};

        std::uint16_t data_a_a{42};
        std::int32_t data_a_b1{-512};
        std::string data_a_b2{"this is a test"};
        std::vector<std::uint8_t> data_a_c{1, 2, 3, 4, 5, 6, 7};

        auto main_thread{std::this_thread::get_id()};

        // Direct callback
        EXPECT_CALL(root, handler_a(data_a_a)).Times(1);

        EXPECT_CALL(root, handler_b(data_a_b1, data_a_b2)).Times(1);

        EXPECT_CALL(root, handler_c(data_a_c)).Times(1);

        // Thread of the dummy component must be different due to the worker
        EXPECT_CALL(root, handler_d(::testing::Ne(main_thread)))
            .WillOnce(
                [&root]
                {
                    root.stop();
                });

        worker.component().post(&DummyComponent::call_b, data_a_b1, data_a_b2);
        worker.component().post(&DummyComponent::call_c, data_a_c);
        worker.component().post(&DummyComponent::call_d);

        // This callback will be invoked first
        worker.component().call_a(data_a_a);

        alarm(1);
        worker.run();
        root.run();
        alarm(0);
    }
}  // namespace kouta::tests::base
