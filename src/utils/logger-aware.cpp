#include "kouta/utils/logger-aware.hpp"

namespace kouta::utils
{
    void LoggerAware::set_logger(ILogger* logger)
    {
        m_logger = logger;
    }

    LoggerAware::LoggerAware(ILogger* logger)
        : m_logger{logger}
    {
    }

    void LoggerAware::log_debug(std::string_view msg) const
    {
        if (m_logger != nullptr)
        {
            m_logger->debug(msg);
        }
    }

    void LoggerAware::log_info(std::string_view msg) const
    {
        if (m_logger != nullptr)
        {
            m_logger->info(msg);
        }
    }

    void LoggerAware::log_warning(std::string_view msg) const
    {
        if (m_logger != nullptr)
        {
            m_logger->warning(msg);
        }
    }

    void LoggerAware::log_error(std::string_view msg) const
    {
        if (m_logger != nullptr)
        {
            m_logger->error(msg);
        }
    }

    void LoggerAware::log_critical(std::string_view msg) const
    {
        if (m_logger != nullptr)
        {
            m_logger->critical(msg);
        }
    }
}  // namespace kouta::utils
