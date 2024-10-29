#include <boost/beast/http.hpp>
#include <boost/json.hpp>

namespace http = boost::beast::http;

class JsonParser {
public:
    void operator()(http::request<http::string_body>& req, http::response<http::string_body>& res) {
        if (req.method() == http::verb::post || req.method() == http::verb::put) {
            try {
                boost::json::parse(req.body());
            } catch (const boost::json::system_error& e) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format"})";
            }
        }
    }
};