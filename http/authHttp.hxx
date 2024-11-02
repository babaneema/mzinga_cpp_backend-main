#pragma once
#include "core/database.hxx"
#include "core/server.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iomanip>

#include "controllers/employeeController.hxx"
#include "controllers/usersController.hxx"
#include "controllers/usersController.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"

class AuthHttp {
public:
    static void login(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::post){
            logger("AuthHttp::login", "Called");
            auto handle = database::get_connection_by_company("admin");
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "false","error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            const boost::json::object& jsonBody = parsedValue.as_object();

            std::string phone = safe_get_value<std::string>(jsonBody, "phone", "phone");
            std::string password = safe_get_value<std::string>(jsonBody, "password", "password");

            auto user =  UserController::getUserByPhone(handle,phone);
            if(!user){
                res.result(http::status::not_found);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "false","error": "Wrong credintial / User not found"})";
                res.prepare_payload();
                return;
            }

            if(!verifyPassword(password, user->get_password())){
                res.result(http::status::not_found);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "false", "error": "Wrong credintial / User not found"})";
                res.prepare_payload();
                return;
            }

            // check if user have session 
            if(auto uuid = ServerSession::check_session_in_request(req)){
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"auth": "true","data": "Authenticated"})";
                res.prepare_payload();
                return;
            }

            ServerSession::set_session_in_response(res, user->get_company());
            res.version(req.version());
            res.result(beast::http::status::ok);
            res.body() = R"({"auth": "true","data": "Authenticated"})";
            res.prepare_payload();
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }
};