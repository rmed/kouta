#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <kouta/utils/logger-aware.hpp>

namespace kouta::tests::utils
{
    using namespace kouta::utils;

    class LoggerMock : public virtual ILogger
    {
    public:
        MOCK_METHOD(void, debug, (std::string_view msg), (override));
        MOCK_METHOD(void, info, (std::string_view msg), (override));
        MOCK_METHOD(void, warning, (std::string_view msg), (override));
        MOCK_METHOD(void, error, (std::string_view msg), (override));
        MOCK_METHOD(void, critical, (std::string_view msg), (override));
    };

    class TestClass : public LoggerAware
    {
    public:
        TestClass() = default;

        void do_log_debug(std::string_view msg)
        {
            log_debug(msg);
        }

        void do_log_info(std::string_view msg)
        {
            log_info(msg);
        }

        void do_log_warning(std::string_view msg)
        {
            log_warning(msg);
        }

        void do_log_error(std::string_view msg)
        {
            log_error(msg);
        }

        void do_log_critical(std::string_view msg)
        {
            log_critical(msg);
        }
    };

    /// @brief Test the behaviour of a logger-aware class if no logger is specified.
    TEST(UtilsTest, EmptyLogger)
    {
        LoggerMock mock{};
        TestClass obj{};

        EXPECT_CALL(mock, debug).Times(0);
        EXPECT_CALL(mock, info).Times(0);
        EXPECT_CALL(mock, warning).Times(0);
        EXPECT_CALL(mock, error).Times(0);
        EXPECT_CALL(mock, critical).Times(0);

        obj.do_log_debug("Message not shown");
        obj.do_log_info("Message not shown");
        obj.do_log_warning("Message not shown");
        obj.do_log_error("Message not shown");
        obj.do_log_critical("Message not shown");
    }

    /// @brief Test the behaviour of a logger-aware class if logger is specified.
    TEST(UtilsTest, DefinedLogger)
    {
        LoggerMock mock{};
        TestClass obj{};

        obj.set_logger(&mock);

        std::string debug_msg{"This is a debug message"};
        std::string info_msg{"This is a info message"};
        std::string warning_msg{"This is a warning message"};
        std::string error_msg{"This is a error message"};
        std::string critical_msg{"This is a critical message"};

        EXPECT_CALL(mock, debug(debug_msg)).Times(1);
        EXPECT_CALL(mock, info(info_msg)).Times(1);
        EXPECT_CALL(mock, warning(warning_msg)).Times(1);
        EXPECT_CALL(mock, error(error_msg)).Times(1);
        EXPECT_CALL(mock, critical(critical_msg)).Times(1);

        obj.do_log_debug(debug_msg);
        obj.do_log_info(info_msg);
        obj.do_log_warning(warning_msg);
        obj.do_log_error(error_msg);
        obj.do_log_critical(critical_msg);

        // Remove logger and try to log again
        obj.set_logger(nullptr);

        obj.do_log_debug(debug_msg);
        obj.do_log_info(info_msg);
        obj.do_log_warning(warning_msg);
        obj.do_log_error(error_msg);
        obj.do_log_critical(critical_msg);
    }
}  // namespace kouta::tests::utils
