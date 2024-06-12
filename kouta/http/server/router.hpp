#pragma once

#include <vector>
#include <string>

#include <boost/url/segments_view.hpp>

#include <kouta/http/method.hpp>
#include <kouta/http/server/handler.hpp>
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
        ///
        /// @tparam TContext        Type of the context to provide to the request processing chain.
        template<class TContext>
        struct Rule
        {
            std::string path;
            method http_method;
            Handler<TContext> handler;
            boost::urls::segments_view segments;
            std::vector<std::size_t> static_segments;
            std::vector<Parameter> parameterized_segments;
        };

        /// @brief Route match.
        ///
        /// @details
        /// This object represents a match in the routing list and provides the parameters extracted from the path (if
        /// any).
        ///
        /// @note The @ref handler element references the original element in the @ref Rule object.
        template<class TContext>
        struct Match
        {
            Handler<TContext>& handler;
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
    ///
    /// @tparam TContext        Type of the context to provide to the request processing chain.
    template<class TContext>
    class Router
    {
    public:
        /// Type of the context used when handling requests.
        using ContextType = TContext;

        /// Handler flow.
        using HandlerType = Handler<ContextType>;

        /// Routing match.
        using Match = router_detail::Match<ContextType>;

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

        std::optional<Match> match(const std::string& path, method http_method) const
        {
            return match(boost::urls::url_view{path}, http_method);
        }

        std::optional<Match> match(const Request& request)
        {
            return match(request.url, request.req.method());
        }

        std::optional<Match> match(const boost::urls::url_view& path, method http_method) const
        {
            boost::urls::segments_view segments{path.segments()};

            for (const Rule& rule : m_rules)
            {
                // Check method
                if (rule.http_method != http_method)
                {
                    continue;
                }

                // Check path length
                if (rule.segments.size() != segments.size())
                {
                    continue;
                }

                // Check static parts
                bool valid{true};

                for (std::size_t idx : rule.static_segments)
                {
                    auto rule_it{rule.segments.begin()};
                    auto segment_it{segments.begin()};

                    std::advance(rule_it, idx);
                    std::advance(segment_it, idx);

                    if (*rule_it != *segment_it)
                    {
                        valid = false;
                        break;
                    }
                }

                if (!valid)
                {
                    continue;
                }

                // Rule is already valid
                Match match{.handler{rule.handler}};

                // Extract parameterized parts
                for (const router_detail::Parameter& param : rule.parameterized_segments)
                {
                    auto segment_it{segments.begin()};

                    std::advance(segment_it, param.index);

                    match.params[param.key] = *segment_it;
                }

                return std::make_optional(match);
            }

            return std::nullopt;
        }

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
        /// @param[in] http_method      HTTP method to match against.
        /// @param[in] handler          Handler to execute in case a match is found.
        void add_rule(const std::string& path, method http_method, const HandlerType& handler)
        {
            Rule rule{.path = path, .http_method = http_method, .handler = handler};

            // Process path to extract segments
            rule.segments = path;

            for (std::size_t i = 0; i < rule.segments.size(); i++)
            {
                const auto& segment{rule.segments[i]};

                if (segment.starts_with(":") && segment.length() > 1)
                {
                    // Parameter
                    rule.parameterized_segments.emplace_back(i, segment.substr(1));
                }
                else
                {
                    // Static
                    rule.static_segments.emplace_back(i);
                }
            }

            // Remove rule if it already exists
            const auto& it{std::find_if(
                m_rules.cbegin(),
                m_rules.cend(),
                [&rule](const Rule& item) { return rule.path == item.path && rule.http_method == item.http_method; })};

            if (it != m_rules.cend())
            {
                m_rules.erase(it);
            }

            m_rules.emplace_back(std::move(rule));
        }

        /// @brief Remove a rule from the router.
        ///
        /// @details
        /// The rule will be searched for by (exact) path and HTTP method. If it is not found, this method does nothing.
        ///
        /// @param[in] path             Path to match against (can be parameterized).
        /// @param[in] http_method      HTTP method to match against.
        void remove_rule(const std::string& path, method http_method)
        {
            const auto& it{std::find_if(
                m_rules.cbegin(),
                m_rules.cend(),
                [&path, http_method](const Rule& item)
                { return path == item.path && http_method == item.http_method; })};

            if (it != m_rules.cend())
            {
                m_rules.erase(it);
            }
        }

    private:
        using Rule = router_detail::Rule<ContextType>;
        using Ruleset = std::vector<Rule>;

        Ruleset m_rules;
    };
}  // namespace kouta::http::server
