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

#include "controllers/taskController.hxx"
#include "controllers/employeeController.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"


class TaskHttp {
public:
    typedef odb::query<Task> query;
    typedef odb::result<Task> result;

    static boost::json::object task_to_json(const boost::shared_ptr<Task> tsk) {
        if (tsk) {
            return {
                {"task_id", tsk->get_task_id()},
                {"task_unique", tsk->get_task_unique()},
                {"task_item", tsk->get_task_item()},
                {"task_amount", tsk->get_task_amount()},
                {"task_start", tsk->get_task_start()},
                {"task_end", tsk->get_task_end()},
                {"task_reg_date", tsk->get_task_reg_date()},
                {
                    "task_employee", tsk->get_task_employee()->get_employee_name()
                }
            };
        }
        return {};
    }

    static boost::json::array tasks_to_json(const std::vector<boost::shared_ptr<Task>>& tasks) {
        boost::json::array json_array;
        for (const auto& tsk : tasks) {
            json_array.emplace_back(task_to_json(tsk));
        }
        return json_array;
    }

    static void get(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            logger("TaskHttp::get", "Called");

            boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();

            if (administrative == "Worker") {
                std::cout << "Here out 2" << std::endl;
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
                res.prepare_payload();
                return;
            }
            std::cout << "Here out 1" << std::endl;
            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            auto tasks = TaskController::getAllTasks(handle);
            response_json["task_data"] = tasks_to_json(tasks);
            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = boost::json::serialize(response_json);
            res.prepare_payload();
            return;
        } else {
            std::cout << "Here out " << std::endl;
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

    static void post(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            logger("TaskHttp::post", "Called");

            boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();

            if (administrative == "Worker") {
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
            std::string task_unique = boost::uuids::to_string(uuid_u);
            std::string task_creation_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string task_employee = safe_get_value<std::string>(jsonBody, "task_employee", "Employee");
            std::string task_item = safe_get_value<std::string>(jsonBody, "task_item", "Item");
            std::string task_amount = safe_get_value<std::string>(jsonBody, "task_amount", "Amount");

            // Get employee
            auto employee___d  = EmployeeController::getEmployeeByUiid(handle, task_employee);
            if(!employee___d){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get customer data"})";
                res.prepare_payload();
                return;
            }
            
            boost::shared_ptr<Task> task_d = boost::make_shared<Task>(
                task_unique,
                employee___d,
                task_item,
                task_amount,
                task_creation_date,
                task_creation_date,
                task_creation_date
            );

            if (TaskController::createTask(handle, task_d)) {
                res.result(http::status::ok);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"message": "Task created successfully!"})";
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to create task"})";
            }
            res.prepare_payload();
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
        }
    }

     static void payrol(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            logger("TaskHttp::payrol", "Called");

            boost::shared_ptr<employee> employee_d;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d);
            std::string administrative = employee_d->get_employee_administrative();

            if (administrative == "Worker") {
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
            std::string task_unique = boost::uuids::to_string(uuid_u);
            std::string task_creation_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string task_employee = safe_get_value<std::string>(jsonBody, "task_employee", "Employee");
            std::string task_start = safe_get_value<std::string>(jsonBody, "task_start", "Item");
            std::string task_end = safe_get_value<std::string>(jsonBody, "task_end", "Amount");

            // Get employee
            auto employee___d  = EmployeeController::getEmployeeByUiid(handle, task_employee);
            if(!employee___d){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get customer data"})";
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

//     static void delete_data(
//         const http::request<http::string_body>& req,
//         http::response<http::string_body>& res,
//         const std::unordered_map<std::string, std::string>& query_params
//     ) {
//         if (req.method() == http::verb::delete_) {
//             logger("MeterHttp::delete", "Called");

//             boost::shared_ptr<employee> employee_session;
//             CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
//             std::string administrative = employee_session->get_employee_administrative();

//             // Worker, Manager & Administrator
//             if(administrative == "Worker"){
//                 res.result(http::status::bad_request);
//                 res.set(http::field::content_type, "application/json");
//                 res.body() = R"({"auth": "true","permission": "false","error": "Bad Request."})";
//                 res.prepare_payload();
//                 return;
//             }
//             auto uuid = query_params.find("uuid");
//             if (uuid != query_params.end()) {
//                 std::string task_unique = uuid->second;
//                 if (TaskController::deleteTask(handle, task_unique)) {
//                     res.result(http::status::ok);
//                     res.body() = R"({"auth": "true","permission": "true","message": "Meter deleted successfully!"})";
//                     res.prepare_payload();
//                     return;
//                 } else {
//                     res.result(http::status::bad_request);
//                     res.body() = R"({"auth": "true","permission": "true","error": "Failed to delete meter"})";
//                     res.prepare_payload();
//                     return;
//                 }
               
//             } else {
//                 res.result(http::status::bad_request);
//                 res.set(http::field::content_type, "application/json");
//                 res.body() = R"({"auth": "true","permission": "true","error": "UUID parameter is missing."})";
//                 res.prepare_payload();
//                 return;
//             }
//         }else{
//             res.result(http::status::bad_request);
//             res.set(http::field::content_type, "application/json");
//             res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
//             res.prepare_payload();
//             return;
//         }
//     }
};
