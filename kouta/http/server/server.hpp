#pragma once

#include <functional>
#include <list>
#include <map>

#include <kouta/http/method.hpp>
#include <kouta/http/server/response.hpp>
#include <kouta/http/server/request.hpp>

namespace kouta::http::server
{
    namespace server_detail
    {
        /// @brief Empty context type to use as a default context for the server.
        struct EmptyContext
        {
        };

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
    }  // namespace server_detail

    /// @brief HTTP server.
    ///
    /// @details
    /// The HTTP server works as a router that receives requests to specific routes (with specific HTTP methods) and
    /// delegates the processing of said requests to registered handlers.
    ///
    /// The processing chain for a request is as follows:
    ///
    /// 1. Pre-request middleware functions are invoked
    /// 2. Handler is invoked
    /// 3. Post-request middleware functions are invoked
    ///
    /// Given that this flow is supposed to be **stateless**, and in order to allow for more *complex* processing
    /// chains and information exchange between middleware functions and handler, the server creates a transient
    /// **context object** which each function in the chain is free to modify. **This object is discarded when the
    /// response is sent**.
    ///
    /// @tparam TContext        Type of the context to provide to the request processing chain.
    template<class TContext = server_detail::EmptyContext>
    class Server
    {
    public:
        /// Type of the context object passed to every handler.
        using ContextType = TContext;

        /// Function to call internally in order to create a context per request.
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
        using HandlerFlow = server_detail::HandlerFlow<Handler, PreMiddleware, PostMiddleware>;

        /// @brief Server router.
        using Router = server_detail::Router<Handler, PreMiddleware, PostMiddleware>;

        // Not default-constructible.
        Server() = delete;

        /// @brief Constructor without context builder.
        ///
        /// @details
        /// This version of the constructor assumes that the context will either be default constructible, or set later
        /// on via the @ref set_context_builder() method.
        ///
        /// @param[in] host         Host address to use.
        /// @param[in] port         Port to use.
        /// TODO: handle address better
        Server(const std::string& host, std::uint16_t port)
            // clang-format off
            : Server{host, port, [](){ return ContextType{}; }}
        // clang-format on
        {
        }

        /// @brief Constructor with context builder.
        ///
        /// @details
        /// This version of the constructor assumes that the context will either be default constructible, or set later
        /// on via the @ref set_context_builder() method.
        ///
        /// @param[in] host         Host address to use.
        /// @param[in] port         Port to use.
        /// TODO: handle address better
        Server(const std::string& host, std::uint16_t port, const ContextBuilder& context_builder)
            : m_context_builder{context_builder}
            , m_router{}
        {
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

    private:
        ContextBuilder m_context_builder;
        Router m_router;
    };
}  // namespace kouta::http::server
