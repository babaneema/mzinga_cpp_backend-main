#pragma once
#include "core/database.hxx"
#include "core/server.hxx"
#include <boost/json.hpp>
#include "controllers/meterController.hxx"
#include "controllers/customerController.hxx"
#include "http/meterHttp.hxx"
#include "http/cusomerHttp.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>


class MeterReadHttp {
public:
    // customer name, phone number, meater number.
    static void seachCustomerInformation(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ){
        // priority is customer name. 
        std::cout << "seachCustomerInformation " << std::endl;
        boost::json::value parsedValue = boost::json::parse(req.body());

        if (!parsedValue.is_object()) {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
            res.prepare_payload();
            return;
        }

        const boost::json::object& jsonBody = parsedValue.as_object();
        std::string meter_read_query = safe_get_value<std::string>(jsonBody, "meter_read_query", "");

        if(meter_read_query.empty()){
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Empty Query."})";
            res.prepare_payload();
            return;
        }
        
        boost::json::object response_json;

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