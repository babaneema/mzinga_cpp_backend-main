#pragma once
#include "core/server.hxx"
#include <iostream>

struct CommonHttp{
    static void optional(http::response<http::string_body>& res ) {
        std::cout << "http::verb::options called" <<endl;
        res.result(http::status::ok);
        res.set(beast::http::field::access_control_allow_origin, "*");
        res.set(beast::http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
        res.set(beast::http::field::access_control_allow_headers, "Content-Type");
        res.set(beast::http::field::access_control_allow_credentials, "false");
        res.prepare_payload();
        return;
    }

    static void unsupported(http::response<http::string_body>& res){
        res.result(http::status::method_not_allowed);
        res.set(http::field::content_type, "application/json");
        res.body() = R"({"error": "Method not allowed"})";
        res.prepare_payload();
    }
};