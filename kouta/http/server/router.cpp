#include <kouta/http/server/router.hpp>

namespace kouta::http::server
{
    Router::Match Router::match(const Request& request) const
    {
        return match(request.url, request.method);
    }

    Router::Match Router::match(const boost::urls::url_view& path, Method method) const
    {
        bool path_found{false};

        boost::urls::segments_view segments{path.segments()};

        for (const Rule& rule : m_rules)
        {
            boost::urls::segments_view rule_segments{rule.path};

            // Check path length
            if (rule_segments.size() != segments.size())
            {
                continue;
            }

            // Check static parts
            bool valid{true};

            for (std::size_t idx : rule.static_segments)
            {
                auto rule_it{rule_segments.begin()};
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

            // Extract parameterized parts
            valid = true;
            std::map<std::string, std::string> params{};

            for (const router_detail::Parameter& param : rule.parameterized_segments)
            {
                auto segment_it{segments.begin()};

                std::advance(segment_it, param.index);

                // Parameter cannot be empty
                if ((*segment_it).size() == 0)
                {
                    valid = false;
                    break;
                }

                params[param.key] = *segment_it;
            }

            if (!valid)
            {
                continue;
            }

            path_found = true;

            // Check method
            if (rule.method != method)
            {
                continue;
            }

            return Match{.result = MatchResult::Ok, .flow = &rule.flow, .params = params};
        }

        // Reaching this point means that either the path was not found or the method was not found
        return Match{.result = path_found ? MatchResult::MethodNotAllowed : MatchResult::NotFound};
    }

    void Router::add_rule(std::string_view path, Method method, const HandlerFlowType& flow)
    {
        Rule rule{.path = std::string{path}, .method = method, .flow = flow};

        // Process path to extract segments
        boost::urls::segments_view segments{rule.path};
        auto segment_it{segments.begin()};
        std::size_t i = 0;

        while (segment_it != segments.end())
        {
            std::string_view seg_view{*segment_it};

            if (seg_view.starts_with(":") && seg_view.length() > 1)
            {
                // Parameter
                rule.parameterized_segments.emplace_back(i, std::string{seg_view.substr(1)});
            }
            else
            {
                // Static
                rule.static_segments.emplace_back(i);
            }

            std::advance(segment_it, 1);
            i++;
        }

        // Remove rule if it already exists
        const auto& it{std::find_if(
            m_rules.cbegin(),
            m_rules.cend(),
            [&rule](const Rule& item) { return rule.path == item.path && rule.method == item.method; })};

        if (it != m_rules.cend())
        {
            m_rules.erase(it);
        }

        m_rules.emplace_back(std::move(rule));
    }

    void Router::remove_rule(std::string_view path, Method method)
    {
        const auto& it{std::find_if(
            m_rules.cbegin(),
            m_rules.cend(),
            [&path, method](const Rule& item) { return path == item.path && method == item.method; })};

        if (it != m_rules.cend())
        {
            m_rules.erase(it);
        }
    }
}  // namespace kouta::http::server
