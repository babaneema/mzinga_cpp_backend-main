// // main.cpp
// #include "core/database.hxx"
// #include "core/server.hxx"
// #include "router.hpp"
// #include "middleware/json_parser.hpp"
// #include "middleware/cors_handler.hpp"

// int main(int argc, char* argv[]) {
//     auto db_handle = database::init_db();
//     Router router(db_handle);

//     // Set up middleware
//     router.use(JsonParser());
//     router.use(CorsHandler());

//     // Set up routes
//     router.get("/api/v1/branch", BranchController::getAllBranches);
//     router.post("/api/v1/branch", BranchController::createBranch);
//     // Add other routes similarly...

//     server::start(router);

//     return 0;
// }

// router.hpp
// #include <functional>
// #include <unordered_map>
// #include <memory>
// #include "core/database.hxx"
// #include <boost/beast/http.hpp>

// namespace beast = boost::beast;
// namespace http = beast::http;

// using RouteHandler = std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&, const std::unordered_map<std::string, std::string>&)>;

// class Router {
// public:
//     Router(std::shared_ptr<odb::mysql::database> db) : db_(db) {}

//     void get(const std::string& path, RouteHandler handler) {
//         routes_[{http::verb::get, path}] = handler;
//     }

//     void post(const std::string& path, RouteHandler handler) {
//         routes_[{http::verb::post, path}] = handler;
//     }

//     // Add other HTTP methods as needed...

//     void use(std::function<void(http::request<http::string_body>&, http::response<http::string_body>&)> middleware) {
//         middleware_.push_back(middleware);
//     }

//     void handle_request(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
//         // Apply middleware
//         for (const auto& mw : middleware_) {
//             mw(const_cast<http::request<http::string_body>&>(req), res);
//         }

//         // Find and execute route handler
//         auto it = routes_.find({req.method(), req.target().to_string()});
//         if (it != routes_.end()) {
//             it->second(req, res, parse_query_params(req));
//         } else {
//             res.result(http::status::not_found);
//             res.set(http::field::content_type, "application/json");
//             res.body() = R"({"error": "Route not found"})";
//         }
//     }

// private:
//     std::shared_ptr<odb::mysql::database> db_;
//     std::unordered_map<std::pair<http::verb, std::string>, RouteHandler> routes_;
//     std::vector<std::function<void(http::request<http::string_body>&, http::response<http::string_body>&)>> middleware_;

//     std::unordered_map<std::string, std::string> parse_query_params(const http::request<http::string_body>& req) {
//         // Implement query parameter parsing
//     }
// };