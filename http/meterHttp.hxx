#pragma once
#include "core/database.hxx"
#include "core/server.hxx"
#include "core/sms.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"

#include "controllers/meterController.hxx"
#include "controllers/customerController.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/json.hpp>

class MeterHttp {
public:
    static boost::json::object meter_to_json(const boost::shared_ptr<meter> me) {
        if (me) {
            return {
                {"meter_id", me->get_meter_id()},
                {"meter_unique", me->get_meter_unique()},
                {"meter_owner", me->get_meter_owner()},
                {"meter_number", me->get_meter_number()},
                {"meter_intital_unit", me->get_meter_intital_unit()},
                {"meter_joinging_price", me->get_meter_joinging_price()},
                {"meter_lock", me->get_meter_lock()},
                {"meter_in_service", me->get_meter_in_service()},
                {"meter_reg_date", me->get_meter_reg_date()},
                {
                    "meter_customer", me->get_meter_customer()->get_customer_name()
                },
                {
                    "meter_customer_id", me->get_meter_customer()->get_customer_id()
                },
                {
                    "meter_customer_unique", me->get_meter_customer()->get_customer_unique()
                },
                {
                    "meter_customer_contact", me->get_meter_customer()->get_customer_contact()
                },
            };
        }
        return {};
    }

    static boost::json::object meter_to_json_custom(const boost::shared_ptr<meter> me) {
        if (me) {
            return {
                {
                    "customer_unique", me->get_meter_customer()->get_customer_unique()
                },
                {
                    "customer_name",   me->get_meter_customer()->get_customer_name()
                },
                {
                    "customer_gender", me->get_meter_customer()->get_customer_gender()
                },
                {
                    "customer_contact", me->get_meter_customer()->get_customer_contact()
                },
                {
                    "customer_address", me->get_meter_customer()->get_customer_address()
                },
                {
                    "customer_house_number", me->get_meter_customer()->get_customer_house_number()
                },
            };
        }
        return {};
    }

    static boost::json::array meters_to_json(const std::vector<boost::shared_ptr<meter>>& meters, bool custom = false) {
        boost::json::array json_array;
        if(custom){
            for (const auto& met : meters) {
                json_array.emplace_back(meter_to_json_custom(met));
            }
            return json_array;
        }
        for (const auto& met : meters) {
            json_array.emplace_back(meter_to_json(met));
        }
        return json_array;
    }

    // GET all meters
    static void get(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::get) {
            logger("MeterHttp::get", "Called");

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

            auto meters = MeterController::getAllMeter(handle);
            response_json["meter_data"] = meters_to_json(meters);
            std::string jsonString = boost::json::serialize(response_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }
    }

    static void queryMeter(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ){
        if (req.method() == http::verb::get) {
            logger("MeterHttp::queryMeter", "Called");

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

            auto query = query_params.find("query");
            if( query != query_params.end()){
                std::string status = query->second;
                // we are expecting the queries to be
                // all, active, locked, unlocked, service

                if(status != "all"){
                    auto meters = MeterController::getMetersByStatus(handle,status);
                    response_json["meter_data"] = meters_to_json(meters);
                    std::string jsonString = boost::json::serialize(response_json);

                    res.set(http::field::content_type, "application/json");
                    res.body() = jsonString;
                    res.prepare_payload();
                    return;
                }else{
                    get(req,res); // doble check
                }

            }
            else{
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Bad Request."})";
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

    // GET meter by UUID
    static void getByUuid(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            logger("MeterHttp::getByUuid", "Called");

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

            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string meter_unique = uuid->second;
                auto meter_d = MeterController::getMeterByUiid(handle, meter_unique);
                if (meter_d) {
                    
                    response_json["meter_data"] = meter_to_json(meter_d);
                    std::string jsonString = boost::json::serialize(response_json);
                    res.set(http::field::content_type, "application/json");
                    res.body() = jsonString;
                    res.prepare_payload();
                    return;
                } else {
                    res.result(http::status::not_found);
                    res.body() = R"({"auth": "true","permission": "true","error": "Meter not found"})";
                    res.prepare_payload();
                    return;
                }
                
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"auth": "false","permission": "false","error": "UUID parameter is missing."})";
                res.prepare_payload();
                return;
            }
        }
    }

    // GET meter by Customer UUID
    static void getByCustomerUuid(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            logger("MeterHttp::getByCustomerUuid", "Called");

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

            auto customer = query_params.find("customer");
            if (customer != query_params.end()) {
                std::string customer_unique = customer->second;
                auto customer_d = CustomerController::getCustomerByUiid(handle, customer_unique);

                if (!customer_d) {
                    res.result(http::status::bad_request);
                    res.set(http::field::content_type, "application/json");
                    res.body() = R"({"error": "Failed to get customer data"})";
                    res.prepare_payload();
                    return;
                } 
                // Now get all meters by customer Id
                auto meters_v = MeterController::searchMeterByCustomerUiid(handle, customer_d);
                response_json["meter_data"] = meters_to_json(meters_v);
                std::string jsonString = boost::json::serialize(response_json);

                res.set(http::field::content_type, "application/json");
                res.body() = jsonString;
                res.prepare_payload();
                return;
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"auth": "true","permission": "true","error": "UUID parameter is missing."})";
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

    // POST to create a new meter
    static void post(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            logger("MeterHttp::post", "Called");

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
            std::string meter_unique = boost::uuids::to_string(uuid);
            std::string meter_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string meter_customer = safe_get_value<std::string>(jsonBody, "meter_customer", "Customer uuid");
            std::string meter_owner = safe_get_value<std::string>(jsonBody, "meter_owner", "");
            std::string meter_number = safe_get_value<std::string>(jsonBody, "meter_number", "");
            std::string meter_initial_unit = safe_get_value<std::string>(jsonBody, "meter_initial_unit", "0.0");
            std::string meter_joinging_price = safe_get_value<std::string>(jsonBody, "meter_joinging_price", "0.0");
            std::string meter_lock = safe_get_value<std::string>(jsonBody, "meter_lock", "No");
            std::string meter_in_service = safe_get_value<std::string>(jsonBody, "meter_in_service", "No");

            auto customer_d = CustomerController::getCustomerByUiid(handle, meter_customer);
            
            if(!customer_d){ // this should not happen
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get customer data"})";
                res.prepare_payload();
                return;
            }

            boost::shared_ptr<meter> meter_d = boost::make_shared<meter>(
                meter_unique,
                customer_d,
                meter_owner,
                meter_number,
                meter_initial_unit,
                meter_joinging_price,
                meter_lock,
                meter_in_service,
                meter_reg_date
            );

            if (MeterController::createMeter(handle, meter_d)) {
                res.result(http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Meter created successfully!"})";
                res.prepare_payload();
                return;
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to create meter"})";
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

    // PUT to update an existing meter
    static void put(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::put) {
            logger("MeterHttp::put", "Called");

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

            if (MeterController::updateMeter(handle, parsedValue)) {
                res.result(http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Meter updated successfully!"})";
                res.prepare_payload();
                return;
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to update meter"})";
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

    // DELETE a meter by UUID
    static void delete_data(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            logger("MeterHttp::delete", "Called");

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
                std::string meter_unique = uuid->second;
                if (MeterController::deleteMeter(handle, meter_unique)) {
                    res.result(http::status::ok);
                    res.body() = R"({"auth": "true","permission": "true","message": "Meter deleted successfully!"})";
                    res.prepare_payload();
                    return;
                } else {
                    res.result(http::status::bad_request);
                    res.body() = R"({"auth": "true","permission": "true","error": "Failed to delete meter"})";
                    res.prepare_payload();
                    return;
                }
               
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "UUID parameter is missing."})";
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
