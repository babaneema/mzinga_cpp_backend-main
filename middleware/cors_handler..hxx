#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

class CorsHandler {
public:
    void operator()(http::request<http::string_body>& req, http::response<http::string_body>& res) {
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type");
    }
};