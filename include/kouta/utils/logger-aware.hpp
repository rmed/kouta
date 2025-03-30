#pragma once

#include "ilogger.hpp"

namespace kouta::utils
{
    /// @brief Abstract class providing integration with external loggers.
    ///
    /// @details
    /// The integration is provided via the @ref ILogger interface. This class expects to receive a pointer to an object
    /// implementing the @ref ILogger interface, which will be used by the internal logging methods to determine whether
    /// the message received can be sent to a logger or not.
    ///
    /// @warning
    /// Developers must guarantee that the pointer to the logger remains valid during lifetime of the logger aware
    /// class.
    class LoggerAware
    {
    public:
        // Copyable
        LoggerAware(const LoggerAware&) = default;
        LoggerAware& operator=(const LoggerAware&) = default;

        // Movable
        LoggerAware(LoggerAware&&) = default;
        LoggerAware& operator=(LoggerAware&&) = default;

        virtual ~LoggerAware() = default;

        /// @brief Update the internal logger pointer.
        ///
        /// @param[in] logger           Pointer to the logger to use. Set to `nullptr` to disable logging.
        void set_logger(ILogger* logger)
        {
            m_logger = logger;
        }

    protected:
        /// @brief Default constructor
        LoggerAware() = default;

        /// @brief Constructor using an existing logger.
        ///
        /// @param[in] logger           Pointer to the logger to use. Set to ǹullptr` to disable logging.
        explicit LoggerAware(ILogger* logger)
            : m_logger{logger}
        {
        }

        /// @brief Log a message with DEBUG (or equivalent) level.
        ///
        /// @note If no logger has been configured, this method simply discards the message.
        void log_debug(std::string_view msg)
        {
            if (m_logger != nullptr)
            {
                m_logger->debug(msg);
            }
        }

        /// @brief Log a message with INFO (or equivalent) level.
        ///
        /// @note If no logger has been configured, this method simply discards the message.
        void log_info(std::string_view msg)
        {
            if (m_logger != nullptr)
            {
                m_logger->info(msg);
            }
        }

        /// @brief Log a message with WARNING (or equivalent) level.
        ///
        /// @note If no logger has been configured, this method simply discards the message.
        void log_warning(std::string_view msg)
        {
            if (m_logger != nullptr)
            {
                m_logger->warning(msg);
            }
        }

        /// @brief Log a message with ERROR (or equivalent) level.
        ///
        /// @note If no logger has been configured, this method simply discards the message.
        void log_error(std::string_view msg)
        {
            if (m_logger != nullptr)
            {
                m_logger->error(msg);
            }
        }

        /// @brief Log a message with CRITICAL (or equivalent) level.
        ///
        /// @note If no logger has been configured, this method simply discards the message.
        void log_critical(std::string_view msg)
        {
            if (m_logger != nullptr)
            {
                m_logger->critical(msg);
            }
        }

    private:
        ILogger* m_logger;
    };
}  // namespace kouta::utils
