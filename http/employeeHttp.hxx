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
#include "controllers/branchController.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"


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
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::get) {
            std::cout << "http::verb::get called" << std::endl;

            // Fetch all employees from the controller
            auto employees = EmployeeController::getAllEmployees(handle);
            auto employees_json = employees_to_json(employees);
            std::string jsonString = boost::json::serialize(employees_json);

            // Set the response content type and payload
            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }
    }

    static void post(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::post){
            std::cout << "http::verb::post Employee called" <<endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string employee_unique = boost::uuids::to_string(uuid);

            std::string employee_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string fullName    = safe_get_value<std::string>(jsonBody, "employee_name", "Employee Name");
            std::string gender      = safe_get_value<std::string>(jsonBody, "employee_gender", "Employee Gender");
            std::string contact     = safe_get_value<std::string>(jsonBody, "employee_contact", "Employee Contact"); 
            std::string address     = safe_get_value<std::string>(jsonBody, "employee_address", "Employee Address"); 
            std::string password    = safe_get_value<std::string>(jsonBody, "employee_password", "1234567"); 
            std::string branch_     = safe_get_value<std::string>(jsonBody, "employee_branch", "uuid");
           
            
            auto branch_d = BranchController::getBranchByUiid(handle, branch_);
            if(!branch_d){ // this should not happen
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to get branch customer"})";
                res.prepare_payload();
                return;
            }

            std::string hash_password = hashPassword(password);

            boost::shared_ptr<employee> employee_d = boost::make_shared<employee>(
                employee_unique,
                branch_d,
                fullName,
                gender,
                contact,
                address,
                hash_password,
                employee_reg_date
            );

            if (EmployeeController::createEmployee(handle, employee_d)) {
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"message": "Employee created successfully!"})";
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
    // login employee
    static void login(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::post){
            std::cout << "http::verb::login Employee called" <<endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            const boost::json::object& jsonBody = parsedValue.as_object();

            std::string contact     = safe_get_value<std::string>(jsonBody, "employee_contact", "Employee Contact"); 
            std::string password    = safe_get_value<std::string>(jsonBody, "employee_password", "1234567"); 
           
            auto atempt_employee =  EmployeeController::getEmployeeByContact(handle, contact);

            if (atempt_employee) {
                // now check passord
                std::string password_harsh = atempt_employee->get_employee_password();
                if(!verifyPassword(password, password_harsh)){
                    res.result(http::status::bad_request);
                    res.set(http::field::content_type, "application/json");
                    res.body() = R"({"error": "Authentication failed."})";
                    res.prepare_payload();
                    return;
                }
                
                // here create a midware session 
                // SessionMiddleware::set_session(atempt_employee->get_employee_id(), res);

                auto employee_json =  employee_to_json(atempt_employee);
                std::string jsonString = boost::json::serialize(employee_json);

                res.version(req.version());
                res.result(beast::http::status::ok);
                res.set(http::field::content_type, "application/json");
                res.body() = jsonString;
                res.prepare_payload();
                return;
                
            }
            else{                
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to create branch"})";
                res.prepare_payload();
                return;
            }
            
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Authentication failed."})";
            res.prepare_payload();
            return;
        }
    }
};
