#pragma once

#include <vector>
#include <string>

#include <boost/url/segments_view.hpp>

#include <kouta/http/method.hpp>
#include <kouta/http/server/handler-flow.hpp>
#include <kouta/http/server/request.hpp>

namespace kouta::http::server
{
    namespace router_detail
    {
        /// @brief Parameters contained in a route path.
        ///
        /// @note The name of these parameters must be unique per route.
        struct Parameter
        {
            std::size_t index;
            std::string key;
        };

        /// @brief Routing rule.
        struct Rule
        {
            std::string path;
            Method method;
            HandlerFlow flow;
            std::vector<std::size_t> static_segments;
            std::vector<Parameter> parameterized_segments;
        };

        /// @brief Code representing the result of the matching.
        enum class MatchResult : std::uint8_t
        {
            /// Request was valid and endpoint found.
            Ok,
            /// No rule matched the provided path.
            NotFound,
            /// There was at least one rule that matched the path, but not the method.
            MethodNotAllowed
        };

        /// @brief Route match.
        ///
        /// @details
        /// This object represents a match in the routing list and provides the parameters extracted from the path (if
        /// any).
        ///
        /// @note The @ref flow element points to the original element in the @ref Rule object.
        struct Match
        {
            /// Result of the matching.
            MatchResult result;
            /// Flow to execute (if a rule matched)
            const HandlerFlow* flow;
            /// Parameters extracted from the path (if any).
            std::map<std::string, std::string> params;
        };
    }  // namespace router_detail

    /// @brief Router router.
    ///
    /// @details
    /// The router maps a specific path to the server to a given handler map. The key used by the router should be
    /// the relative path (route) to the wanted endpoint, as the URL for a request will be matched using its origin
    /// form.
    ///
    /// Ideally, if a path is not found in the router, a 404 response is returned by the server.
    class Router
    {
    public:
        /// Handler flow.
        using HandlerFlowType = HandlerFlow;

        /// Match result code.
        using MatchResult = router_detail::MatchResult;

        /// Routing match.
        using Match = router_detail::Match;

        /// @brief Default constructor.
        ///
        /// @details
        /// Routes can be added via the @ref add() method.
        Router() = default;

        // Copyable
        Router(const Router&) = default;
        Router& operator=(const Router&) = default;

        // Movable
        Router(Router&&) = default;
        Router& operator=(Router&&) = default;

        virtual ~Router() = default;

        /// @brief Match against a URL path and a method.
        ///
        /// @param[in] path             Path to match against.
        /// @param[in] method           HTTP method requested.
        ///
        /// @returns Match object if found, otherwise std::nullptr.
        Match match(const boost::urls::url_view& path, Method method) const;

        /// @brief Add a rule to the router.
        ///
        /// @details
        /// Rules are unique and identified by path and HTTP method. Adding a rule that already exists will overwrite
        /// the rule.
        ///
        /// Certain parts of a path can be marked as parameterized by marking them with a `:` prefix. For instance:
        ///
        /// ```
        /// /my/url/:param1/rules/:param2
        /// ```
        ///
        /// Will extract paramters `param1` and `param2` when performing the matching.
        ///
        /// @param[in] path             Path to match against (can be parameterized).
        /// @param[in] metod            HTTP method to match against.
        /// @param[in] flow             Handler flow to execute in case a match is found.
        void add_rule(std::string_view path, Method method, const HandlerFlowType& flow);

        /// @brief Remove a rule from the router.
        ///
        /// @details
        /// The rule will be searched for by (exact) path and HTTP method. If it is not found, this method does nothing.
        ///
        /// @param[in] path             Path to match against (can be parameterized).
        /// @param[in] method           HTTP method to match against.
        void remove_rule(std::string_view path, Method method);

    private:
        using Rule = router_detail::Rule;
        using Ruleset = std::vector<Rule>;

        Ruleset m_rules;
    };
}  // namespace kouta::http::server
