#pragma once
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/database.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"

#include "controllers/unitController.hxx"
#include "controllers/branchController.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/json.hpp>

class UnitHttp{
public:
    static boost::json::object unit_to_json(const boost::shared_ptr<unit> un) {
        if(un){
            return {
                {"unit_id", un->get_unit_id()},
                {"unit_unique", un->get_unit_unique()},
                {"unit_price", un->get_unit_price()},
                {"unit_reg_date", un->get_unit_reg_date()},
                {
                    "branch_name", un->get_unit_branch()->get_branch_name()
                },
            };
        }
        return {};
    }

    static boost::json::array units_to_json(const std::vector<boost::shared_ptr<unit>> units) {
        boost::json::array json_array;
        for (const auto& un : units) {
            json_array.emplace_back(unit_to_json(un));
        }
        return json_array;
    }

    static void get(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res
        )
    {
        if (req.method() == http::verb::get) {
            logger("UnitHttp::get", "Called");

            // check for authentication and authorization 

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();
            
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

            auto units = UnitController::getAllUnits(handle);
            response_json["unit_data"] = units_to_json(units);
            std::string jsonString = boost::json::serialize(response_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }
    }
    
    static void get_unit_by_uiid(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        auto uuid = query_params.find("uuid");
        if (uuid != query_params.end()) {

            logger("UnitHttp::get_unit_by_uiid", "Called");

            // check for authentication and authorization 

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();
            
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
            std::string unit_uuid = uuid->second;
            auto unit_d = UnitController::getUnitByUuid(handle, unit_uuid);
            auto unit_d_json = unit_to_json(unit_d);
            response_json["unit_data"] = unit_d_json;


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
    ) {
        if (req.method() == http::verb::post) {
            logger("UnitHttp::post", "Called");
            // check for authentication and authorization 
            
            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();

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
                res.body() = R"({"auth": "true","permission": "true","error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string unit_unique = boost::uuids::to_string(uuid);
            std::string unit_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string unit_price = safe_get_value<std::string>(jsonBody, "unit_price", "0.0");
            std::string unit_branch = safe_get_value<std::string>(jsonBody, "unit_branch", "Branch");

            auto branch_d = BranchController::getBranchByUiid(handle, unit_branch);
            if(!branch_d){ // this should not happen
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get branch"})";
                res.prepare_payload();
                return;
            }

            boost::shared_ptr<unit> unit_d = boost::make_shared<unit>(
                unit_unique,
                branch_d,
                unit_price,
                unit_reg_date
            );

            if (UnitController::createUnit(handle, unit_d)) {
                res.result(http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Unit created successfully!"})";
                res.prepare_payload();
                return;
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "Failed to create unit"})";
                res.prepare_payload();
                return;
            }
            
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","false": "true","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

    static void put(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::put) {
            logger("UnitHttp::put", "Called");
            // check for authentication and authorization 

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();
            
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
                res.body() = R"({"auth": "true","permission": "true","error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            if (UnitController::updateUnit(handle, parsedValue)) {
                res.result(http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Unit updated successfully!"})";
                res.prepare_payload();
                return;
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to update unit"})";
                res.prepare_payload();
                return;
            }
            
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
        }
    }

    static void delete_data(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            logger("UnitHttp::delete", "Called");
            // check for authentication and authorization 

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();
            
            // Worker, Manager & Administrator
            if(administrative == "Worker"){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }

            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string unit_unique = uuid->second;
                if (UnitController::deleteUnit(handle, unit_unique)) {
                    res.result(http::status::ok);
                    res.body() = R"({"auth": "true","permission": "true","message": "Unit deleted successfully!"})";
                    res.prepare_payload();
                    return;
                } else {
                    res.result(http::status::bad_request);
                    res.body() = R"({"auth": "true","permission": "true","error": "Failed to delete unit"})";
                    res.prepare_payload();
                    return;
                }
                res.prepare_payload();
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

};