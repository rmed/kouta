#pragma once

#include <map>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/url.hpp>

#include <kouta/http/method.hpp>

namespace kouta::http::server
{
    /// @brief HTTP request wrapper.
    ///
    /// @details
    /// The request contains the underlying Beast request object, as well as some helper methods to extract certain
    /// information.
    class Request : public boost::beast::http::request<boost::beast::http::string_body>
    {
    public:
        using PathParams = std::map<std::string, std::string>;

        // Default constructible.
        Request() = default;

        // Copyable
        Request(const Request&) = default;
        Request& operator=(const Request&) = default;

        // Movable
        Request(Request&&) = default;
        Request& operator=(Request&&) = default;

        virtual ~Request() = default;

        /// @brief Obtain URL parsed parameters.
        const PathParams& path_params() const;

        /// @brief Set the parsed URL parameters.
        ///
        /// @details
        /// These come from the router rule matching process.
        void set_path_params(const PathParams& params);

    private:
        PathParams m_path_params;
    };

}  // namespace kouta::http::server
