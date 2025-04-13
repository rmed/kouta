#pragma once

#include <format>

#include <soci/connection-pool.h>
#include <soci/error.h>
#include <soci/session.h>

#include <kouta/utils/logger-aware.hpp>

#include "result.hpp"

namespace kouta::db
{
    class AbstractAdapter : public kouta::utils::LoggerAware
    {
    public:
        /// @brief Database connection pool type.
        using Pool = soci::connection_pool;

        /// @brief Backend-specific error handler function signature.
        using BackendErrorHandler = std::function<std::pair<ResultCode, int>(const soci::soci_error*)>;

        /// @brief Default constructor.
        AbstractAdapter();

        // Copyable
        AbstractAdapter(const AbstractAdapter&) = default;
        AbstractAdapter& operator=(const AbstractAdapter&) = default;

        // Movable
        AbstractAdapter(AbstractAdapter&&) = default;
        AbstractAdapter& operator=(AbstractAdapter&&) = default;

        virtual ~AbstractAdapter() = default;

        /// @brief Set the internal pointer to the connection pool.
        ///
        /// @param[in] pool             Pointer to the connection pool. Set to `nullptr` to disable the adapter.
        void set_pool(Pool* pool);

        /// @brief Set the specialized backend error handler function.
        ///
        /// @details
        /// This function should do the appropriate conversions in order to get the backend-specific error code from
        /// SOCI.
        ///
        /// Passing an empty function will cause the adapter to use the empty default handler @ref
        /// default_backend_error_handler.
        ///
        /// @param[in] handler              Handler to use.
        void set_backend_error_handler(BackendErrorHandler&& handler);

    protected:
        /// @brief Obtain a pointer to the connection pool.
        ///
        /// @note The pool must have been initialized beforehand via the @ref set_pool() method.
        ///
        /// @returns Pointer to the connection pool if initialized, `nullptr` otherwise.
        Pool* pool();

        /// @brief Run a query/statement through the pool.
        ///
        /// @details
        /// This method retrieves a session from the connection pool and passes it as the only argument to the provided
        /// functor. Said functor is expected to return an appropriate @ref Result object.
        ///
        /// In addition, common errors will be transformed into their appropriate @ref ResultCode values, so that
        /// callers need only implement the logic related to the queries themselves. However, more specific cases will
        /// need to be handled via a backend error handler function that should be registered via @ref
        /// set_backend_error_handler.
        ///
        /// @note If the pool has not been initialized, no query will be executed.
        ///
        /// @tparam TFunctor            Functor/Lambda to execute. Must receive a reference to a session as argument.
        ///
        /// @param[in] functor          Functor to invoke with the retrieved session.
        ///
        /// @returns Database @ref Result object.
        template<class TFunctor>
        std::invoke_result_t<TFunctor, soci::session&> session_run(TFunctor&& functor)
        {
            if (m_pool == nullptr)
            {
                log_error("Database pool was not initialized");
                return {ResultCode::ConnectionError};
            }

            try
            {
                soci::session conn{*m_pool};
                return functor(conn);
            }
            catch (const soci::soci_error& e)
            {
                log_error(std::format("Session error: {}", e.what()));

                // Handle common errors
                ResultCode code;
                int error_detail{};

                switch (e.get_error_category())
                {
                case soci::soci_error::connection_error:
                    code = ResultCode::ConnectionError;
                    break;
                case soci::soci_error::invalid_statement:
                    code = ResultCode::InvalidStatement;
                    break;
                case soci::soci_error::no_privilege:
                    code = ResultCode::PermissionError;
                    break;
                case soci::soci_error::no_data:
                    code = ResultCode::NotFound;
                    break;
                case soci::soci_error::constraint_violation:
                    code = ResultCode::ConstraintViolation;
                    break;
                case soci::soci_error::unknown_transaction_state:
                    code = ResultCode::TransactionError;
                    break;
                case soci::soci_error::system_error:
                    code = ResultCode::SystemError;
                    break;
                case soci::soci_error::unknown:
                    // May be a backend-specific error
                    {
                        std::pair<ResultCode, int> specialized{m_backend_error_handler(&e)};
                        code = specialized.first;
                        error_detail = specialized.second;
                    }
                    break;
                default:
                    code = ResultCode::UnknownError;
                    break;
                }

                return {code, error_detail};
            }
            catch (const std::runtime_error& e)
            {
                log_error(std::format("Standard exception encountered: {}", e.what()));

                return {ResultCode::UnknownError};
            }
            catch (...)
            {
                log_error("Unknown exception encountered");

                return {ResultCode::UnknownError};
            }
        }

    private:
        /// @brief Default function used to handle unknown errors when running queries.
        ///
        /// @note The unknown error may indicate a backend-specific error, but there is no guarantee.
        ///
        /// @param[in] e            Caught exception.
        ///
        /// @returns Pair containing UnknownError as result code.
        std::pair<ResultCode, int> default_backend_error_handler(const soci::soci_error* /*e*/);

        /// @brief Pointer to the connection pool used.
        Pool* m_pool;

        /// @brief Backend-specific error handler.
        BackendErrorHandler m_backend_error_handler;
    };
}  // namespace kouta::db
