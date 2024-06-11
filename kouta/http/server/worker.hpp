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

namespace kouta::http::server
{
    namespace worker_detail
    {
        /// @brief Complete flow used to handle a request.
        ///
        /// @tparam THandler                Type of handler function.
        /// @tparam TPreMiddleware          Type of pre-request middleware functions.
        /// @tparam TPostMiddleware         Type of post-request middleware functions.
        template<class THandler, class TPreMiddleware, class TPostMiddleware>
        struct HandlerFlow
        {
            THandler handler;
            std::list<TPreMiddleware> pre_request;
            std::list<TPostMiddleware> post_request;
        };

        /// @brief Handler map.
        ///
        /// @details
        /// The handler map contains a set of HTTP methods for a given route and specifies which handler is in charge of
        /// handling said method.
        ///
        /// Ideally, if a method is not found in the map, a 405 response is returned by the server.
        ///
        /// @tparam THandler                Type of handler function.
        /// @tparam TPreMiddleware          Type of pre-request middleware functions.
        /// @tparam TPostMiddleware         Type of post-request middleware functions.
        template<class THandler, class TPreMiddleware, class TPostMiddleware>
        using HandlerMap = std::map<http::method, HandlerFlow<THandler, TPreMiddleware, TPostMiddleware>>;

        /// @brief Routing map.
        ///
        /// @details
        /// The router maps a specific path to the server to a given handler map. The key used by the router should be
        /// the relative path (route) to the wanted endpoint, as the URL for a request will be matched using its origin
        /// form.
        ///
        /// Ideally, if a path is not found in the router, a 404 response is returned by the server.
        ///
        /// @tparam THandler                Type of handler function.
        /// @tparam TPreMiddleware          Type of pre-request middleware functions.
        /// @tparam TPostMiddleware         Type of post-request middleware functions.
        template<class THandler, class TPreMiddleware, class TPostMiddleware>
        using Router = std::map<std::string, HandlerMap<THandler, TPreMiddleware, TPostMiddleware>>;
    }  // namespace worker_detail

    template<class TContext>
    class Worker
    {
    public:
        /// @brief Type of the context object passed to every handler.
        using ContextType = TContext;

        /// @brief Function to call internally in order to create a context per request.
        using ContextBuilder = std::function<ContextType()>;

        /// @brief Pre-request middleware function type.
        ///
        /// @details
        /// These functions are intended to perform some pre-processing on the request, such as initializing a
        /// database connection or extracting certain information to use in the handlers. The @p ctx parameter can be
        /// modified to provide this information further down the processing chain.
        ///
        /// Returning a @ref Response from these functions effectively interrupts the chain, which can be useful in case
        /// a specific pre-condition has not been met.
        ///
        /// @param[in] request      Received request.
        /// @param[in,out] ctx      Context built during the request handling flow.
        ///
        /// @returns std::nullopt to continue with the processing chain, otherwise a @ref Response object to interrupt
        /// it.
        using PreMiddleware = std::function<std::optional<Response>(const Request& request, ContextType& ctx)>;

        /// @brief Post-request middleware function type.
        ///
        /// @details
        /// These functions are intended to perform some post-processing on the response, such as setting a content type
        /// or dropping a database connection. Both the @p response and @p ctx parameters can be modified to provide
        /// information further down the processing chain.
        ///
        /// Returning a @ref Response from these functions effectively interrupts the chain, which can be useful in case
        /// a specific post-condition has not been met.
        ///
        /// @param[in] response     Response returned by the handler.
        /// @param[in,out] ctx      Context built during the request handling flow.
        ///
        /// @returns std::nullopt to continue with the processing chain, otherwise a @ref Response object to interrupt
        /// it.
        using PostMiddleware = std::function<std::optional<Response>(Response& response, ContextType& ctx)>;

        /// @brief Request handler type.
        ///
        /// @param[in] request      Received request.
        /// @param[in,out] ctx      Context built during the request handling flow.
        ///
        /// @returns response to send, which may be processed further by any post-request middleware.
        using Handler = std::function<Response(const Request& request, ContextType& ctx)>;

        /// @brief Request handling flow.
        using HandlerFlow = worker_detail::HandlerFlow<Handler, PreMiddleware, PostMiddleware>;

        /// @brief Server router.
        using Router = worker_detail::Router<Handler, PreMiddleware, PostMiddleware>;

        /// @brief Constructor.
        ///
        /// @param[in] socket             Socket that received the connection.
        /// @param[in] router             Router reference (owned by the server).
        /// @param[in] context_builder    Router reference (owned by the server).
        /// @param[in] config             Configuration reference (owned by the server).
        Worker(
            boost::asio::ip::tcp::socket&& socket,
            const Router& router,
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

            // compare lengths
            // check starts with :, fill m_request.params


            // Pre-middleware

            // Handler

            // Post-middleware
        }

        void send_response() {}

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

        void handle_write(bool keep_alive, boost::beast::error_code ec, std::size_t /* bytes_transferred */) {
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
