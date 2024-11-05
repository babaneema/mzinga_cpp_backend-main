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
#include "core/session_macro.hxx"

class EmployeeHttp {
public:
    // Convert a single employee to JSON
    static boost::json::object employee_to_json(const boost::shared_ptr<employee> emp) {
        if (emp) {
            return {
                {"employee_id", emp->get_employee_id()},
                {"employee_unique", emp->get_employee_unique()},
                {"employee_name", emp->get_employee_name()},
                {"employee_gender", emp->get_employee_gender()},
                {"employee_contact", emp->get_employee_contact()},
                {"employee_address", emp->get_employee_address()},
                {"employee_reg_date", emp->get_employee_reg_date()},
                {"employee_branch", emp->get_employee_branch()->get_branch_name()}
            };
        }
        return {};
    }

    // Convert a vector of employees to JSON array
    static boost::json::array employees_to_json(const std::vector<boost::shared_ptr<employee>> employees) {
        boost::json::array json_array;
        for (const auto& emp : employees) {
            json_array.emplace_back(employee_to_json(emp));
        }
        return json_array;
    }

    // HTTP GET request handler for fetching employees
    static void get(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::get) {
            logger("EmployeeHttp::get", "Called");

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

            // Fetch all employees from the controller
            auto employees = EmployeeController::getAllEmployees(handle);
            response_json["employee_data"]  = employees_to_json(employees);
            std::string jsonString = boost::json::serialize(response_json);

            // Set the response content type and payload
            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }
    }

    static void post(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::post){
            logger("EmployeeHttp::post", "Called");

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
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }
            std::cout << "Called 6" <<std::endl;

            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string employee_unique = boost::uuids::to_string(uuid);

            std::string employee_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string fullName    = safe_get_value<std::string>(jsonBody, "employee_name", "Employee Name");
            std::string gender      = safe_get_value<std::string>(jsonBody, "employee_gender", "Employee Gender");
            std::string contact     = safe_get_value<std::string>(jsonBody, "employee_contact", "Employee Contact"); 
            std::string address     = safe_get_value<std::string>(jsonBody, "employee_address", "Employee Address"); 
            std::string admin       = safe_get_value<std::string>(jsonBody, "employee_administrative", "employee_administrative"); 
            std::string password    = safe_get_value<std::string>(jsonBody, "employee_password", "employee_password"); 
            std::string branch_     = safe_get_value<std::string>(jsonBody, "employee_branch", "uuid");
            
            auto branch_d = BranchController::getBranchByUiid(handle, branch_);
            if(!branch_d){ // this should not happen
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get branch Employee"})";
                res.prepare_payload();
                return;
            }
            boost::shared_ptr<employee> employee_d = boost::make_shared<employee>(
                employee_unique,
                branch_d,
                fullName,
                gender,
                contact,
                address,
                admin,
                employee_reg_date
            );
            std::cout << "Called 9" <<std::endl;
            if (EmployeeController::createEmployee(handle, employee_d)) {
                // create user record. 
                std::string company = "mzingamaji";
                std::string password_hash = hashPassword(password);
                boost::shared_ptr<users> user_d = boost::make_shared<users>(
                    company,
                    contact,
                    password_hash,
                    employee_reg_date
                );
                auto admin_handle = database::get_connection_by_company("admin");
                UserController::createUser(admin_handle,user_d);
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Employee created successfully!"})";
                res.prepare_payload();
                return;
            }else{               
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Failed to create branch"})";
                res.prepare_payload();
                return;
            }
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }
};
