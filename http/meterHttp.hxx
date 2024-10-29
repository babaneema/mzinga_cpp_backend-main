#pragma once
#include "core/database.hxx"
#include "core/server.hxx"
#include <boost/json.hpp>
#include "controllers/meterController.hxx"
#include "controllers/customerController.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::get) {
            std::cout << "http::verb::get called" << std::endl;

            auto meters = MeterController::getAllMeter(handle);
            auto meters_json = meters_to_json(meters);
            std::string jsonString = boost::json::serialize(meters_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }
    }

    static void queryMeter(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ){
        if (req.method() == http::verb::get) {
            std::cout << "http::verb::get called" << std::endl;
            auto query = query_params.find("query");
            if( query != query_params.end()){
                std::string status = query->second;
                // we are expecting the queries to be
                // all, active, locked, unlocked, service

                if(status != "all"){
                    auto meters = MeterController::getMetersByStatus(handle,status);
                    auto meters_json = meters_to_json(meters);
                    std::string jsonString = boost::json::serialize(meters_json);

                    res.set(http::field::content_type, "application/json");
                    res.body() = jsonString;
                    res.prepare_payload();
                }else{
                    get(handle,req,res);
                }

            }
            else{
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

    // GET meter by UUID
    static void getByUuid(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string meter_unique = uuid->second;
                auto meter_d = MeterController::getMeterByUiid(handle, meter_unique);
                if (meter_d) {
                    boost::json::object response_json;
                    boost::json::object meter_json = meter_to_json(meter_d);
                    response_json["meter_data"] = meter_json;
                    std::string jsonString = boost::json::serialize(response_json);
                    res.set(http::field::content_type, "application/json");
                    res.body() = jsonString;
                } else {
                    res.result(http::status::not_found);
                    res.body() = R"({"error": "Meter not found"})";
                }
                res.prepare_payload();
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "UUID parameter is missing."})";
                res.prepare_payload();
            }
        }
    }

    // GET meter by Customer UUID
    static void getByCustomerUuid(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
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
                auto meters_json = meters_to_json(meters_v);
                std::string jsonString = boost::json::serialize(meters_json);

                res.set(http::field::content_type, "application/json");
                res.body() = jsonString;
                res.prepare_payload();
                return;
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "UUID parameter is missing."})";
                res.prepare_payload();
                return;
            }
        }
    }

    // POST to create a new meter
    static void post(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            std::cout << "http::verb::post Meter called" << std::endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
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
                res.body() = R"({"error": "Failed to get customer data"})";
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
                res.body() = R"({"message": "Meter created successfully!"})";
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "Failed to create meter"})";
            }
            res.prepare_payload();
        }
    }

    // PUT to update an existing meter
    static void put(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::put) {
            std::cout << "http::verb::put Meter called" << std::endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            if (MeterController::updateMeter(handle, parsedValue)) {
                res.result(http::status::ok);
                res.body() = R"({"message": "Meter updated successfully!"})";
            } else {
                std::cout << "Failed 3" << endl;
                res.result(http::status::bad_request);
                res.body() = R"({"error": "Failed to update meter"})";
            }
            res.prepare_payload();
        }
    }

    // DELETE a meter by UUID
    static void delete_data(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            std::cout << "http::verb::delete Meter called" << std::endl;
            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string meter_unique = uuid->second;
                if (MeterController::deleteMeter(handle, meter_unique)) {
                    res.result(http::status::ok);
                    res.body() = R"({"message": "Meter deleted successfully!"})";
                } else {
                    res.result(http::status::bad_request);
                    res.body() = R"({"error": "Failed to delete meter"})";
                }
                res.prepare_payload();
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "UUID parameter is missing."})";
                res.prepare_payload();
            }
        }
    }

};
