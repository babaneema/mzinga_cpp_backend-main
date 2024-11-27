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

#include "controllers/pricesController.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"

class PriceHttp {
public:
    static boost::json::object price_to_json(const boost::shared_ptr<Price>& pr) {
        if (pr) {
            return {
                {"price_id", pr->get_price_id()},
                {"price_unique", pr->get_price_unique()},
                {"price_item", pr->get_price_item()},
                {"price_amount", pr->get_price_amount()},
                {"price_reg_date", pr->get_price_reg_date()}
            };
        }
        return {};
    }

    static boost::json::array prices_to_json(const std::vector<boost::shared_ptr<Price>>& prices) {
        boost::json::array json_array;
        for (const auto& pr : prices) {
            json_array.emplace_back(price_to_json(pr));
        }
        return json_array;
    }

    static void get(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            logger("PriceHttp::get", "Called");

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

            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            auto prices = PriceController::getAllPrices(handle);
            auto prices_json = prices_to_json(prices);
            response_json["price_data"] = prices_json;

            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            res.body() = boost::json::serialize(response_json);
            res.prepare_payload();
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
        }
    }

    static void post(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            logger("PriceHttp::post", "Called");

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
            std::string price_unique = boost::uuids::to_string(uuid_u);

            std::string price_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string price_item = safe_get_value<std::string>(jsonBody, "price_item", "Price Item");
            std::string price_amount = safe_get_value<std::string>(jsonBody, "price_amount", "Price Amount");

            boost::shared_ptr<Price> price_d = boost::make_shared<Price>(
                price_unique, price_item, price_amount, price_reg_date);

            if (PriceController::createPrice(handle, price_d)) {
                res.version(req.version());
                res.result(http::status::ok);
                res.body() = R"({"message": "Price created successfully!"})";
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to create price"})";
            }
            res.prepare_payload();
        } else {
            res.result(http::status::bad_request);
            res.set(http::field::content_type, "application/json");
            res.body() = R"({"error": "Bad Request."})";
            res.prepare_payload();
        }
    }

    // Additional methods like put, delete_data can follow the same pattern.
    static void delete_data(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            logger("PriceHttp::delete", "Called");
            std::cout << "PriceHttp Called 1" << std::endl;

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
                std::string price_unique = uuid->second;
                std::cout << price_unique << std::endl;
                if (PriceController::deletePrice(handle, price_unique)) {
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
