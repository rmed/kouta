#pragma once

#include <functional>
#include <list>
#include <map>
#include <optional>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/url.hpp>

#include <kouta/http/method.hpp>
#include <kouta/http/server/config.hpp>
#include <kouta/http/server/response.hpp>
#include <kouta/http/server/request.hpp>
#include <kouta/http/server/router.hpp>

namespace kouta::http::server
{
    template<class TContext>
    class Worker
    {
    public:
        /// @brief Type of the context object passed to every handler.
        using ContextType = TContext;

        /// @brief Function to call internally in order to create a context per request.
        using ContextBuilder = std::function<ContextType()>;

        using HandlerType = Handler<ContextType>;
        using RouterType = Router<ContextType>;

        /// @brief Constructor.
        ///
        /// @param[in] socket             Socket that received the connection.
        /// @param[in] router             Router reference (owned by the server).
        /// @param[in] context_builder    Router reference (owned by the server).
        /// @param[in] config             Configuration reference (owned by the server).
        Worker(
            boost::asio::ip::tcp::socket&& socket,
            const RouterType& router,
            const ContextBuilder& context_builder,
            const Config& config)
            : m_stream{std::move(socket)}
            , m_router{router}
            , m_context_builder{context_builder}
            , m_config{config}
            , m_buffer{config.max_buffer_size}
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
            boost::asio::dispatch(m_stream.get_executor(), std::bind_front(&Worker<ContextType>::do_read, this));
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
                m_stream, m_buffer, m_request.req, std::bind_front(&Worker<ContextType>::handle_read, this));
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
        void process_chain()
        {
            // Extract information from the request
            auto url{boost::urls::parse_uri(m_request.req.target())};

            if (!url.has_value())
            {
                // Invalid URL, return Not Found
                return;
            }

            m_request.url = url.value();
            m_request.scheme = m_request.url.scheme();
            m_request.host = m_request.url.host();
            m_request.path = m_request.url.path();
            m_request.path_segments = m_request.url.segments();
            m_request.args = m_request.url.params();

            // Match route
            std::optional<typename RouterType::Match> match{m_router.match(m_request)};

            if (!match.has_value())
            {
                // TODO: send response
                return;
            }

            ContextType ctx{m_context_builder()};

            // Pre-middleware
            for (const auto& mware : match->handler.pre_request)
            {
                std::optional<Response> response{mware(m_request, ctx)};

                if (response.has_value())
                {
                    // TODO: send response
                    return;
                }
            }

            // Handler
            Response response{match->handler.handler(m_request, ctx)};

            // Post-middleware
            for (const auto& mware : match->handler.post_request)
            {
                std::optional<Response> post_response{mware(response, ctx)};

                if (post_response.has_value())
                {
                    // TODO: send response
                    return;
                }
            }

            // TODO: send response
        }

        void send_response(Response&& response)
        {
            bool keep_alive = response.keep_alive();

            // Write the response
            boost::beast::async_write(
                m_stream, std::move(response), std::bind_front(&Worker::handle_write, this, keep_alive));
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
            process_chain();
        }

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
        const RouterType& m_router;
        const ContextBuilder& m_context_builder;
        const Config& m_config;
        boost::beast::flat_buffer m_buffer;
        Request m_request;
    };
}  // namespace kouta::http::server
