#pragma once
#include "controllers/meterController.hxx"
#include "controllers/customerController.hxx"

#include "http/meterHttp.hxx"
#include "http/customerHttp.hxx"

#include "core/sms.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/json.hpp>


class MeterReadHttp {
public:
    // customer name, phone number, meater number.
    static void seachCustomerInformation(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ){
        // priority is customer name. 
        logger("MeterReadHttp::seachCustomerInformation", "Called");

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

        const boost::json::object& jsonBody = parsedValue.as_object();
        std::string meter_read_query = safe_get_value<std::string>(jsonBody, "meter_read_query", "");

        if(meter_read_query.empty()){
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"auth": "true","permission": "true","error": "Empty Query."})";
            res.prepare_payload();
            return;
        }
        
        boost::json::object response_json;
        response_json["auth"] = "true";
        response_json["permission"] = "true";

        // Serch first for customer name
        auto customer_data_by_name = CustomerController::searchCustomerByName(handle, meter_read_query);
        if(customer_data_by_name.size()){
            auto customer_json = CustomerHttp::customers_to_json(customer_data_by_name);
            response_json["customer_data"] = customer_json;
            std::string jsonString = boost::json::serialize(response_json);
            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
            return;
        }

        auto customer_data_by_phone = CustomerController::searchCustomerByPhone(handle, meter_read_query);
        if(customer_data_by_phone.size()){
            auto customer_json = CustomerHttp::customers_to_json(customer_data_by_phone);
            response_json["customer_data"] = customer_json;
            std::string jsonString = boost::json::serialize(response_json);
            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
            return;
        }

        auto customer_data_by_meter = MeterController::searchMeterByNumber(handle, meter_read_query);
        if(customer_data_by_meter.size()){
            auto customer_meter_json = MeterHttp::meters_to_json(customer_data_by_meter);
            response_json["customer_meter_data"] = customer_meter_json;
            std::string jsonString = boost::json::serialize(response_json);
            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
            return;
        }

        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        res.prepare_payload();
        return;
    }
};