#pragma once

#include <functional>
#include <memory>
#include <optional>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/url.hpp>

#include <kouta/http/method.hpp>
#include <kouta/http/server/config.hpp>
#include <kouta/http/server/context.hpp>
#include <kouta/http/server/response.hpp>
#include <kouta/http/server/request.hpp>
#include <kouta/http/server/router.hpp>

namespace kouta::http::server
{
    template<class TContext>
    class Worker : public std::enable_shared_from_this<Worker<TContext>>
    {
    public:
        /// @brief Type of the context object passed to every handler.
        using ContextType = TContext;

        /// @brief Function to call internally in order to create a context per request.
        using ContextBuilder = context::ContextBuilder<ContextType>;

        /// @brief Constructor.
        ///
        /// @param[in] socket             Socket that received the connection.
        /// @param[in] router             Router reference (owned by the server).
        /// @param[in] config             Configuration reference (owned by the server).
        /// @param[in] context_builder    Router reference (owned by the server).
        Worker(
            boost::asio::ip::tcp::socket&& socket,
            const Router& router,
            const Config& config,
            const ContextBuilder& context_builder)
            : m_stream{std::move(socket)}
            , m_router{router}
            , m_context_builder{context_builder}
            , m_config{config}
            , m_buffer{m_config.max_buffer_size}
            , m_request{}
        {
        }

        // Not copyable
        Worker(const Worker&) = delete;
        Worker& operator=(const Worker&) = delete;

        // Not movable
        Worker(Worker&&) = delete;
        Worker& operator=(Worker&&) = delete;

        virtual ~Worker() = default;

        /// @brief Start processing the request.
        ///
        /// @details
        /// Asynchronous operations are executed in the strand provided by the stream (which in turn is created by the
        /// server).
        void run()
        {
            boost::asio::dispatch(
                m_stream.get_executor(), std::bind_front(&Worker<ContextType>::do_read, this->shared_from_this()));
        }

    private:
        /// @brief Close the connection.
        void do_close()
        {
            boost::beast::error_code ec;
            m_stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
        }

        /// @brief Read data from the established connection.
        void do_read()
        {
            // Clear the request
            m_request = Request{};

            // Set a timeout
            m_stream.expires_after(m_config.request_timeout);

            boost::beast::http::async_read(
                m_stream,
                m_buffer,
                m_request,
                std::bind_front(&Worker<ContextType>::handle_read, this->shared_from_this()));
        }

        /// @brief Craft a response with base information set in the configuration.
        ///
        /// @returns Response.
        Response craft_response()
        {
            Response response{};

            response.version(m_request.version());
            response.keep_alive(m_request.keep_alive());

            // Set default fields specified in configuration
            for (const auto& [field, value] : m_config.base_response_fields)
            {
                response.set(field, value);
            }

            return response;
        }

        /// @brief Request processing chain.
        ///
        /// @details
        /// The processing chain for a request is as follows:
        ///
        /// 1. Match the request against a route
        /// 2. Pre-request middleware functions are invoked
        /// 3. Handler is invoked
        /// 4. Post-request middleware functions are invoked
        Response process_chain()
        {
            auto url{boost::urls::parse_relative_ref(m_request.target())};
            Response response{craft_response()};

            if (!url.has_value())
            {
                // Invalid URL, return Not Found
                response.result(boost::beast::http::status::not_found);
                response.prepare_payload();
                return response;
            }

            // Match route
            Router::Match match{m_router.match(url.value(), m_request.method())};

            if (match.result == Router::MatchResult::NotFound)
            {
                response.result(boost::beast::http::status::not_found);
                response.prepare_payload();
                return response;
            }
            else if (match.result == Router::MatchResult::MethodNotAllowed)
            {
                response.result(boost::beast::http::status::method_not_allowed);
                response.prepare_payload();
                return response;
            }

            m_request.set_path_params(match.params);

            // Set context for the handlers
            context::set_context(m_context_builder());

            try
            {
                // Pre-middleware
                for (const auto& mware : match.flow->pre_request)
                {
                    if (!mware(m_request, response))
                    {
                        response.prepare_payload();
                        return response;
                    }
                }

                // Handler
                if (!match.flow->handler(m_request, response))
                {
                    response.prepare_payload();
                    return response;
                }

                // Post-middleware
                for (const auto& mware : match.flow->post_request)
                {
                    if (!mware(response))
                    {
                        response.prepare_payload();
                        return response;
                    }
                }
            }
            catch (...)
            {
                // Discard any data already in the response
                response = craft_response();
                response.result(boost::beast::http::status::internal_server_error);
            }

            response.prepare_payload();
            return response;
        }

        /// @brief Send a response to the connected client.
        ///
        /// @param[in] response         Response to send.
        void send_response(boost::beast::http::message_generator&& response)
        {
            bool keep_alive{response.keep_alive()};

            // Write the response
            boost::beast::async_write(
                m_stream,
                std::move(response),
                std::bind_front(&Worker<ContextType>::handle_write, this->shared_from_this(), keep_alive));
        }

        /// @brief Handle data received from the connection.
        void handle_read(boost::beast::error_code ec, std::size_t /* bytes_transferred*/)
        {
            if (ec == boost::beast::http::error::end_of_stream)
            {
                // Connection was closed;
                return do_close();
            }

            if (ec)
            {
                // Failed to read
                return;
            }

            // Process the request
            send_response(process_chain());
        }

        /// @brief Handle data sent over the connection.
        void handle_write(bool keep_alive, boost::beast::error_code ec, std::size_t /* bytes_transferred */)
        {
            if (ec)
            {
                // Failed to write
                return;
            }

            if (!keep_alive)
            {
                // Connection must be closed
                return do_close();
            }

            // Read another request
            do_read();
        }

        boost::beast::tcp_stream m_stream;
        const Router& m_router;
        const ContextBuilder& m_context_builder;
        const Config& m_config;
        boost::beast::flat_buffer m_buffer;
        Request m_request;
    };
}  // namespace kouta::http::server
