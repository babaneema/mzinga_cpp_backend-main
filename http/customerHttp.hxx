#pragma once
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>

#include "controllers/customerController.hxx"
#include "controllers/branchController.hxx"

#include "http/meterHttp.hxx"
#include "http/billHttp.hxx"
#include "http/paymentHttp.hxx"

#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"



class CustomerHttp{
public:
    static boost::json::object customer_to_json(const boost::shared_ptr<customer> cu) {
        if(cu){
            return {
                {"customer_id", cu->get_customer_id()},
                {"customer_unique", cu->get_customer_unique()},
                {"customer_name", cu->get_customer_name()},
                {"customer_gender", cu->get_customer_gender()},
                {"customer_contact", cu->get_customer_contact()},
                {"customer_address", cu->get_customer_address()},
                {"customer_house_number", cu->get_customer_house_number()},
                {"customer_reg_date", cu->get_customer_reg_date()},
                {
                    "branch_name", cu->get_customer_branch()->get_branch_name()
                },
            };
        }
        return {};
    }

    static boost::json::array customers_to_json(const std::vector<boost::shared_ptr<customer>> customers) {
        boost::json::array json_array;
        for (const auto& cr : customers) {
            json_array.emplace_back(customer_to_json(cr));
        }
        return json_array;
    }

    static void get(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res
        )
    {
        if (req.method() == http::verb::get) {
            logger("CustomerHttp::get", "Called");

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

            auto customers = CustomerController::getAllCustomers(handle);
            response_json["customer_data"] = customers_to_json(customers);
            std::string jsonString = boost::json::serialize(response_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
            return;
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

    static void get_uiid_full_data(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        auto uuid = query_params.find("uuid");
        if (uuid != query_params.end()) {
            logger("CustomerHttp::get_uiid_full_data", "Called");

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
            std::string customer_uui = uuid->second;
            auto customer = CustomerController::getCustomerByUiid(handle, customer_uui);
            auto custommer_json = customer_to_json(customer);
            response_json["user_data"] = custommer_json;

            // get meter data
            auto meters = MeterController::getMeterByCustomerId(handle,customer->get_customer_id());
            auto meters_json = MeterHttp::meters_to_json(meters);
            response_json["meters_data"] = meters_json;

            std::cout << customer->get_customer_id() << std::endl;

            // Now let get bills data
            auto bills = BillController::getBillsByCustomerId(handle,customer->get_customer_id());
            auto bills_json = BillHttp::bills_to_json(bills);
            response_json["bills_data"] = bills_json;

            // Now let get bills data
            auto payments_d = PaymentController::getPaymentsByCustomerId(handle,customer->get_customer_id());
            auto payments_d_json = PaymentHttp::payments_to_json(payments_d);
            response_json["payments_data"] = payments_d_json;

            std::string jsonString = boost::json::serialize(response_json);

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

    static void getCustomerById(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        auto id = query_params.find("id");
        if (id != query_params.end()) {
            logger("CustomerHttp::getCustomerById", "Called");

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
            std::string customer_id = id->second;
            int customer_id_int = std::stoi(customer_id);
            auto customer = CustomerController::getCustomerById(handle, customer_id_int);
            auto custommer_json = customer_to_json(customer);
            response_json["customer_data"] = custommer_json;


            std::string jsonString = boost::json::serialize(response_json);

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
    
    // Get Customer Report 
    static void getCustomerReport(
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res, 
        const std::unordered_map<std::string, std::string>& query_params
        )
    {
        auto report = query_params.find("report");
        if(report != query_params.end()){
            logger("CustomerHttp::getCustomerReport", "Called");

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

            auto start_date = query_params.find("start_date");
            auto end_date = query_params.find("end_date");

            std::string start_ =  start_date->second;
            std::string end_ =  end_date->second;


            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            auto customers_data = CustomerController::GetCustomerReport(handle, start_, end_);
            auto customers_json = customers_to_json(customers_data);
            response_json["customers_data"] = customers_json;

            std::string jsonString = boost::json::serialize(response_json);

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

    // Handle POST request to create a customer
    static void post(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            logger("CustomerHttp::post", "Called");

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
            std::string customer_unique = boost::uuids::to_string(uuid);
            std::string customer_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();

            std::string customer_branch = safe_get_value<std::string>(jsonBody, "customer_branch", "Customer Branch");
            std::string customer_name = safe_get_value<std::string>(jsonBody, "customer_name", "Customer Name");
            std::string customer_gender = safe_get_value<std::string>(jsonBody, "customer_gender", "Customer Gender");
            std::string customer_contact = safe_get_value<std::string>(jsonBody, "customer_contact", "Customer Contact");
            std::string customer_address = safe_get_value<std::string>(jsonBody, "customer_address", "Customer Address");
            std::string customer_house_number = safe_get_value<std::string>(jsonBody, "customer_house_number", "Customer House Number");

            auto branch_d = BranchController::getBranchByUiid(handle, customer_branch);
            if(!branch_d){ // this should not happen
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get branch customer"})";
                res.prepare_payload();
                return;
            }

            // Create customer object
            boost::shared_ptr<customer> customer_d = boost::make_shared<customer>(
                customer_unique, 
                branch_d,
                customer_name, 
                customer_gender, 
                customer_contact, 
                customer_address, 
                customer_house_number, 
                customer_reg_date
            );

            if (CustomerController::createCustomer(handle, customer_d)) {
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Customer created successfully!"})";
                res.prepare_payload();
                return;
            } else {                
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to create customer"})";
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

    static void put(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::put) {
            logger("CustomerHttp::put", "Called");

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

            if (CustomerController::updateCustomer(handle, parsedValue)) {
                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Customer updated successfully!"})";
            } else {                
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to update customer"})";
            }
            res.prepare_payload();
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }

    // Handle DELETE request to remove a customer by UUID
    static void delete_data(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            logger("CustomerHttp::delete", "Called");

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
                std::string customer_unique = uuid->second;
                if (CustomerController::deleteCustomer(handle, customer_unique)) {
                    res.version(req.version());
                    res.result(beast::http::status::ok);
                    res.body() = R"({"auth": "true","permission": "true","message": "Customer deleted successfully!"})";
                } else {                
                    res.result(http::status::bad_request);
                    res.set(http::field::content_type, "application/json");
                    res.body() = R"({"auth": "true","permission": "true","error": "Failed to delete customer"})";
                }
                res.prepare_payload();
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Bad Request."})";
                res.prepare_payload();
            }
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }
    
    static void searchCustomerMeter(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ){
        if (req.method() == http::verb::post) {
            logger("CustomerHttp::delete", "Called");

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
      
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string search_query = safe_get_value<std::string>(jsonBody, "search_query", "uuid");

            // Priotize name, phone number and last meter number
           

            auto customer_v1 = CustomerController::searchCustomerByName(handle, search_query);
            if(!customer_v1.empty()){
                auto customers_json =  customers_to_json(customer_v1);
                response_json["customers_data"] = customers_json;
                std::string jsonString = boost::json::serialize(response_json);
                res.set(http::field::content_type, "application/json");
                res.body() = jsonString;
                return;
            }

            auto customer_v2 = CustomerController::searchCustomerByPhone(handle, search_query);
             if(!customer_v2.empty()){
                auto customers_json = CustomerHttp::customers_to_json(customer_v2);
                response_json["customers_data"] = customers_json;
                std::string jsonString = boost::json::serialize(response_json);
                res.set(http::field::content_type, "application/json");
                res.body() = jsonString;
                return;
            }

            auto customer_v3 = MeterController::searchMeterByNumber(handle, search_query);
            if(!customer_v3.empty()){
                auto customers_json = MeterHttp::meters_to_json(customer_v3, true);
                response_json["customers_data"] = customers_json;
                std::string jsonString = boost::json::serialize(response_json);
                res.set(http::field::content_type, "application/json");
                res.body() = jsonString;
                return;
            }
            
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "true","permission": "true","error": "Failed to get customer data"})";
            res.prepare_payload();
            return;
        }else{
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "false","permission": "false","error": "Bad Request."})";
            res.prepare_payload();
            return;
        }
    }
};  