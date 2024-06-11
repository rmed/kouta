#pragma once

#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <kouta/http/server/config.hpp>
#include <kouta/http/server/worker.hpp>

namespace kouta::http::server
{
    namespace server_detail
    {
        /// @brief Empty context type to use as a default context for the server.
        struct EmptyContext
        {
        };
    }  // namespace server_detail

    /// @brief HTTP server.
    ///
    /// @details
    /// The HTTP server works as a router that receives requests to specific routes (with specific HTTP methods) and
    /// delegates the processing of said requests to registered handlers.
    ///
    /// The processing chain for a request is as follows:
    ///
    /// 1. Match the request against a route
    /// 2. Pre-request middleware functions are invoked
    /// 3. Handler is invoked
    /// 4. Post-request middleware functions are invoked
    ///
    /// Given that this flow is supposed to be **stateless**, and in order to allow for more *complex* processing
    /// chains and information exchange between middleware functions and handler, the server creates a transient
    /// **context object** which each function in the chain is free to modify. **This object is discarded when the
    /// response is sent**.
    ///
    /// The server creates a number of workers to handle the requests, each one in their own dedicated thread. However,
    /// all of them accept requests from the same acceptor.
    ///
    /// @tparam TContext        Type of the context to provide to the request processing chain.
    template<class TContext = server_detail::EmptyContext>
    class Server
    {
    public:
        using WorkerType = Worker<TContext>;

        /// @see Worker::ContextType
        using ContextType = WorkerType::ContextType;

        /// @brief Worker::ContextBuilder
        using ContextBuilder = WorkerType::ContextBuilder;

        /// @see Worker::PreMiddleware
        using PreMiddleware = WorkerType::PreMiddleware;

        /// @see Worker::PostMiddleware
        using PostMiddleware = WorkerType::PostMiddleware;

        /// @see Worker::Handler
        using Handler = WorkerType::Handler;

        /// @see Worker::HandlerFlow
        using HandlerFlow = WorkerType::HandlerFlow;

        /// @see Worker::Router
        using Router = WorkerType::Router;

        // Not default-constructible.
        Server() = delete;

        /// @brief Constructor without context builder.
        ///
        /// @details
        /// This version of the constructor assumes that the context will either be default constructible, or set later
        /// on via the @ref set_context_builder() method.
        ///
        /// @param[in] host             Host address to use.
        /// @param[in] port             Port to use.
        /// @param[in] num_threads      Number of threads to create.
        Server(const std::string& host, std::uint16_t port, int num_threads)
            // clang-format off
            : Server{host, port, num_threads, Config{}, [](){ return ContextType{}; }}
        // clang-format on
        {
        }

        /// @brief Constructor with context builder.
        ///
        /// @param[in] host             Host address to use.
        /// @param[in] port             Port to use.
        /// @param[in] num_threads      Number of threads to create.
        /// @param[in] config           Additional server configuration.
        /// @param[in] context_builder  Function used to create the context for the processing.
        Server(const std::string& host, std::uint16_t port, int num_threads, const Config& config, const ContextBuilder& context_builder)
            : m_host{host}
            , m_port{port}
            , m_num_threads{num_threads}
            , m_config{config}
            , m_context_builder{context_builder}
            , m_router{}
            , m_context{num_threads > 0 ? num_threads : 1}
            , m_acceptor{}
            , m_workers{}
        {
            m_workers.reserve(num_threads - 1);
        }

        // Not copyable
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

        // Not movable
        Server(Server&&) = delete;
        Server& operator=(Server&&) = delete;

        virtual ~Server() = default;

        /// @brief Specify the function to use in order to build a request context.
        ///
        /// @warning This should be modified during runtime.
        ///
        /// @param[in] builder          Function to call to provide a context to the handlers.
        void set_context_builder(const ContextBuilder& builder)
        {
            m_context_builder = builder;
        }

        /// @brief Register a route in the server.
        ///
        /// @details
        /// This effectively enables handling of requests for a specific route for the given method.
        ///
        /// @note If a route-method pair already exists, this method will overwrite them.
        ///
        /// @warning
        /// The router is shared with the workers via reference, so this method is not thread-safe. It is discouraged to
        /// modify routes during runtime.
        ///
        /// @param[in] route            Relative route to match against.
        /// @param[in] method           Method to handle.
        /// @param[in] flow             Flow to use in order to handle the request.
        void register_route(const std::string& route, http::method method, const HandlerFlow& flow)
        {
            if (m_router.contains(route))
            {
                m_router[route][method] = flow;
            }
            else
            {
                m_router[route] = {method, flow};
            }
        }

        /// @brief Unregister a route in the server.
        ///
        /// @details
        /// When a route is unregistered, requests arriving at said route will no longer be handled by the server.
        ///
        /// @note If a route-method pair does not exist, this method does nothing.
        ///
        /// @warning
        /// The router is shared with the workers via reference, so this method is not thread-safe. It is discouraged to
        /// modify routes during runtime.
        void unregister_route(const std::string& route, http::method method)
        {
            auto route_it{m_router.find(route)};

            if (route_it != m_router.end())
            {
                route_it.erase(method);

                // Remove route from map if there are no methods left
                if (route_it.empty())
                {
                    m_router.erase(route);
                }
            }
        }

        /// @brief Start the server.
        ///
        /// @details
        /// Starting the server attempts to bind to the provided address and creates workers to start serving requests.
        /// This method blocks until the event loop is explicitly stopped.
        void run()
        {
            if (!configure_acceptor())
            {
                // Error in the setup
                m_acceptor->close();
                m_acceptor.reset();
                return;
            }

            // Create workers
            for (auto i = 0; i < ((m_num_threads > 0 ? m_num_threads : 1) - 1); i++)
            {
                m_workers.emplace_back([this]() { m_context.run(); });
            }

            // Start receiving connections
            do_accept();

            // Block here until the event loop terminates
            m_context.run();

            // Clean-up resources
            for (auto& i : m_workers)
            {
                if (i.joinable())
                {
                    i.join();
                }
            }

            m_workers.clear();
            m_acceptor.reset();
        }

        /// @brief Stop accepting connections and the event loop.
        ///
        /// @details
        /// This method is thread-safe and will cause the @ref run() method to unblock and clean up any allocated
        /// resources.
        void stop()
        {
            if (m_acceptor.has_value())
            {
                m_acceptor->close();
            }

            m_context.stop();
        }

    private:
        /// @brief Configure the acceptor to start handling requests.
        ///
        /// @returns true if the configuration is correct/valid, otherwise false.
        bool configure_acceptor()
        {
            auto const address{boost::asio::ip::make_address(m_host)};
            auto const endpoint{boost::asio::ip::tcp::endpoint{address, m_port}};

            // The acceptor must be created each time the sever starts
            m_acceptor.emplace(boost::asio::make_strand(m_context));

            boost::beast::error_code ec;

            m_acceptor->open(endpoint.protocol(), ec);
            if (ec)
            {
                return false;
            }

            m_acceptor->set_option(boost::asio::socket_base::reuse_address{true}, ec);
            if (ec)
            {
                return false;
            }

            m_acceptor->bind(endpoint, ec);
            if (ec)
            {
                return false;
            }

            m_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
            if (ec)
            {
                return false;
            }
        }

        /// @brief Begin accepting connections.
        ///
        /// @details
        /// When a connection is received, it is set to execute in a new strand and continue the flow via the @ref
        /// handle_accept() method.
        void do_accept()
        {
            m_acceptor->async_accept(
                boost::asio::make_strand(m_context), std::bind_front(&Server<TContext>::handle_accept, this));
        }

        /// @brief Handle a new connection.
        ///
        /// @details
        /// Each connection is passed to a new @ref Worker that is destroyed after the processing has been completed.
        void handle_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            if (ec)
            {
                // Error in the accept process
                return;
            }

            // Create the worker and hand over the socket
            std::make_shared<WorkerType>(std::move(socket), m_router, m_context_builder)->run();

            // Accept more connections
            do_accept();
        }

        std::string m_host;
        std::uint16_t m_port;
        int m_num_threads;
        Config m_config;
        ContextBuilder m_context_builder;
        Router m_router;
        boost::asio::io_context m_context;
        std::optional<boost::asio::ip::tcp::acceptor> m_acceptor;
        std::vector<std::thread> m_workers;
    };
}  // namespace kouta::http::server
