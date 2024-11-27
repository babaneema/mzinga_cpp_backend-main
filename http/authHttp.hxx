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
    
    static void logout(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ){
        if(req.method() == http::verb::get){
            logger("AuthHttp::login", "Called");
            auto session_uuid = ServerSession::check_session_in_request(req);
            if(session_uuid){
                // now we can logout
                if(ServerSession::delete_session_by_uuid(*session_uuid)){
                    std::cout << "Logout success fuly"<< std::endl;
                    res.result(http::status::bad_request);
                    res.set(http::field::content_type, "application/json");
                    res.body() = R"({"auth": "logout"})";
                    res.prepare_payload();
                    return;
                }else{
                    std::cout << "Error occured Deleting sessions"<< std::endl;
                    res.result(http::status::bad_request);
                    res.set(http::field::content_type, "application/json");
                    res.body() = R"({"auth": "logout"})";
                    res.prepare_payload();
                    return;
                }
                
            }else{
                // session does not exist
                std::cout << "session does not exist" << std::endl;
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "logout"})";
                res.prepare_payload();
                return;
            } 
        }else{
            std::cout << "Called 1" << std::endl;
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }
    static void login(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::post){
            std::cout << "Called " << std::endl;
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
                std::cout << " that very bad actual " << std::endl;
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
                res.body() = R"({"auth": "true", "permission": "true","data": "Authenticated"})";
                res.prepare_payload();
                return;
            }

            std::string s_uuid =  ServerSession::set_session_in_response(res, phone, user->get_company());
            std::string set_cookie = "uuid=" + s_uuid + "; Path=/; SameSite=None; Secure";

            res.set(http::field::set_cookie, set_cookie);
            res.version(req.version());
            res.result(beast::http::status::ok);
            res.set(http::field::content_type, "application/json");
            std::string responseBody = R"({"auth": "true", "permission": "true","data": ")" + s_uuid + R"("})";
            res.body() = responseBody;
            res.prepare_payload();
            return;

            // next route
            auto user_handle = database::get_connection_by_company(user->get_company());
            auto employee_d = EmployeeController::getEmployeeByContact(user_handle, phone);

            if(!employee_d){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }

            std::string administrative = employee_d->get_employee_administrative();
            res.set("User-Type", administrative);
            // Worker, Manager & Administrator
            if(administrative == "Worker"){
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.set(http::field::content_type, "application/json");
                std::string responseBody = R"({"auth": "true", "permission": "true", "user": "Worker","data": ")" + s_uuid + R"("})";
                res.body() = responseBody;
                res.prepare_payload();
                return;
            }

            // res.version(req.version());
            // res.result(beast::http::status::ok);
            // res.set(http::field::content_type, "application/json");
            // std::string responseBody = R"({"auth": "true", "permission": "true","user": "Admin","data": ")" + s_uuid + R"("})";
            // res.body() = responseBody;
            // res.prepare_payload();
            // return;
        }else{
            std::cout << "Called 1" << std::endl;
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }
};