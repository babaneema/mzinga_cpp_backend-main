#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>
#include <unordered_set>
#include <boost/asio/ip/network_v6.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using RouteHandler = std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&, const std::unordered_map<std::string, std::string>&)>;

namespace server {
    std::unordered_map<std::string, RouteHandler> routes;

    std::unordered_set<std::string> allowed_origins;
    std::string allowed_methods = "GET, POST, PUT, DELETE, OPTIONS";
    std::string allowed_headers = "Content-Type, Authorization";
    bool allow_credentials = true;
    int max_age = 3600; // 1 hour

    void register_route(const std::string& path, RouteHandler handler) {
        routes[path] = handler;
    }

    void add_allowed_origin(const std::string& origin) {
        allowed_origins.insert(origin);
    }

    std::unordered_map<std::string, std::string> parse_query_params(const std::string& target) {
        std::unordered_map<std::string, std::string> params;
        std::size_t pos = target.find('?');
        if (pos != std::string::npos) {
            std::string query = target.substr(pos + 1);
            std::size_t start = 0, end;
            while ((end = query.find('&', start)) != std::string::npos) {
                std::string param = query.substr(start, end - start);
                std::size_t eq = param.find('=');
                if (eq != std::string::npos) {
                    std::string key = param.substr(0, eq);
                    std::string value = param.substr(eq + 1);
                    params[key] = value;
                }
                start = end + 1;
            }
            std::string param = query.substr(start);
            std::size_t eq = param.find('=');
            if (eq != std::string::npos) {
                std::string key = param.substr(0, eq);
                std::string value = param.substr(eq + 1);
                params[key] = value;
            }
        }
        return params;
    }

    void set_cors_headers(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
        if (req.find(http::field::origin) != req.end()) {
            auto origin = req[http::field::origin];
            std::string orign_str(origin);
            
            if (allowed_origins.empty() || allowed_origins.find(orign_str) != allowed_origins.end()) {
                std::cout << "Giving headers " <<endl;
                res.set(http::field::access_control_allow_origin, orign_str);
                res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
                res.set(http::field::access_control_allow_headers, "Content-Type");
                res.set(http::field::access_control_allow_credentials, "true");

            } else {
                std::cout << "This got called " <<endl;
                // If the origin is not in the allowed list, don't set the header
                return;
            }
        }

        res.set(http::field::access_control_allow_methods, allowed_methods);
        res.set(http::field::access_control_allow_headers, allowed_headers);

        if (allow_credentials) {
            res.set(http::field::access_control_allow_credentials, "true");
        }

        res.set(http::field::access_control_max_age, std::to_string(max_age));
    }
    
    void handle_request(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
        
        set_cors_headers(req, res);

        if (req.method() == http::verb::options) {
            // Handle CORS preflight request
            res.result(http::status::no_content);
            return;
        }
        
        std::string target(req.target());

        std::string::size_type pos = target.find('?');
        std::string path = (pos != std::string::npos) ? target.substr(0, pos) : target;
        std::unordered_map<std::string, std::string> query_params = parse_query_params(target);
     
        auto route_iter = routes.find(path);
        if (route_iter != routes.end()) {
            route_iter->second(req, res, query_params);
        } else {
            res.result(http::status::not_found);
            res.set(http::field::content_type, "text/plain");
            res.body() = "404 Not Found";
        }

        res.prepare_payload();
    }

    void handle_connection(std::shared_ptr<tcp::socket> socket) {
        beast::error_code ec;
        beast::flat_buffer buffer;

        for (;;) {
            http::request<http::string_body> req;
            http::read(*socket, buffer, req, ec);

            if (ec == http::error::end_of_stream) break;
            if (ec) {
                std::cerr << "Error reading request: " << ec.message() << std::endl;
                break;
            }

            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::server, "Boost Beast Server");
            res.set(http::field::content_type, "text/plain");

            handle_request(req, res);

            http::write(*socket, res, ec);
            if (ec) {
                std::cerr << "Error writing response: " << ec.message() << std::endl;
                break;
            }

            if (!req.keep_alive()) break;
        }

        beast::error_code shutdown_ec;
        socket->shutdown(tcp::socket::shutdown_send, shutdown_ec);
    }

    void start_server(int num_threads = 1, int port = 4000) {
        try {
            net::io_context io_context{num_threads};
            tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

            std::cout << "Server running on port " << port << " with " << num_threads << " threads" << std::endl;

            std::vector<std::thread> threads;
            for (int i = 0; i < num_threads - 1; ++i) {
                threads.emplace_back([&io_context] { io_context.run(); });
            }

            for (;;) {
                auto socket = std::make_shared<tcp::socket>(io_context);
                acceptor.accept(*socket);

                std::thread(&handle_connection, socket).detach();
            }

            for (auto& t : threads) {
                t.join();
            }

        } catch (std::exception& e) {
            std::cerr << "Server Error: " << e.what() << std::endl;
        }
    }
}