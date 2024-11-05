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

#include "controllers/branchController.hxx"
#include "controllers/employeeController.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/database.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"


class BranchHttp{
public:
    typedef odb::query<branch> query;
    typedef odb::result<branch> result;


    static boost::json::object branch_to_json(const boost::shared_ptr<branch> br) {
        if(br){
            return {
                {"branch_id", br->get_branch_id()},
                {"branch_unique", br->get_branch_unique()},
                {"branch_name", br->get_branch_name()},
                {"branch_reg_date", br->get_branch_reg_date()}
            };
        }
        return {};
    }

    static boost::json::array branches_to_json(const std::vector<boost::shared_ptr<branch>> branches) {
        boost::json::array json_array;
        for (const auto& br : branches) {
            json_array.emplace_back(branch_to_json(br));
        }
        return json_array;
    }

    static void get(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        if (req.method() == http::verb::get) {
            logger("BranchHttp::get", "Called");
            // check for authentication and authorization 

            boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();
            
            // Worker, Manager & Administrator
            if(administrative == "Worker"){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }

            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            auto search_key = query_params.find("search");
            auto branches = BranchController::getAllBranches(handle);
            auto branches_json = branches_to_json(branches);
            response_json["branch_data"] = branches_json;
            std::string jsonString = boost::json::serialize(response_json);

            res.result(beast::http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

    static void get_branch_by_uiid(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        auto uuid = query_params.find("uuid");
        if (uuid != query_params.end()) {
            logger("BranchHttp::post", "get_branch_by_uiid");

            boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();
            
            // Worker, Manager & Administrator
            if(administrative == "Worker"){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }

            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            // Get first customer data
            std::string branch_uui = uuid->second;
            auto branch_d = BranchController::getBranchByUiid(handle, branch_uui);
            auto branch_d_json = branch_to_json(branch_d);
            response_json["branch_data"] = branch_d_json;


            std::string jsonString = boost::json::serialize(response_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
            return;
        }

    }

    static void post(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::post){
            logger("BranchHttp::post", "Called");

            boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();
            
            // Worker, Manager & Administrator
            if(administrative == "Worker"){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }

            boost::json::value parsedValue = boost::json::parse(req.body());
            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            boost::uuids::uuid uuid_u = boost::uuids::random_generator()();
            std::string branch_unique = boost::uuids::to_string(uuid_u);

            std::string branch_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();

            std::string branch_name = safe_get_value<std::string>(jsonBody, "branch_name", "Branch Name");

            boost::shared_ptr<branch> branch_d = boost::make_shared<branch>(
                                                    branch_unique, 
                                                    branch_name, 
                                                    branch_reg_date
                                                );

            if (BranchController::createBranch(handle, branch_d)) {
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"message": "Branch created successfully!"})";
            }else{                
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to create branch"})";
            }
            res.prepare_payload();
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }


    static void put(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::put){
            logger("BranchHttp::put", "Called");

            boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();
            
            // Worker, Manager & Administrator
            if(administrative == "Worker"){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }

            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            if (BranchController::updateBranch(handle, parsedValue)) {
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"message": "Branch update successfully!"})";
            }else{                
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to update branch"})";
            }
            res.prepare_payload();
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

    static void delete_data(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        if(req.method() == http::verb::delete_){

            logger("BranchHttp::delete_data", "Called");

             boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();
            
            // Worker, Manager & Administrator
            if(administrative == "Worker"){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }

            auto uuid = query_params.find("uuid");
            if(uuid != query_params.end()){
                std::string branch_unique = uuid->second;
                if (BranchController::deleteBranch(handle, branch_unique)) {
                    res.version(req.version());
                    res.result(beast::http::status::ok);
                    res.body() = R"({"message": "Branch deleted successfully!"})";
                }else{                
                    res.result(http::status::bad_request);
                    res.set(http::field::content_type, "application/json");
                    res.body() = R"({"error": "Failed to delete branch"})";
                }
                res.prepare_payload();
            
            }else{
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Bad Request."})";
                res.prepare_payload();
                return;
            }
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

};