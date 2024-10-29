#pragma once
#include "core/database.hxx"
#include "core/server.hxx"
#include <boost/json.hpp>
#include "controllers/billController.hxx"
#include "controllers/unitController.hxx"
#include "http/meterHttp.hxx"
#include "core/safe_json.hxx"
#include "core/sms.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iomanip>
#include <sstream>


class BillHttp {
public: 
    static boost::json::object bill_to_json(const boost::shared_ptr<bill> bi) {
        if(bi){
            return {
                {"bill_id", bi->get_bill_id()},
                {"bill_unique", bi->get_bill_unique()},
                {"bill_unit_used", bi->get_bill_unit_used()},
                {"bill_cost", bi->get_bill_cost()},
                {"bill_reg_date", bi->get_bill_reg_date()},
                {
                    "bill_customer", bi->get_bill_customer()->get_customer_name()
                },
                {
                    "bill_customer_unique", bi->get_bill_customer()->get_customer_unique()
                },
                {
                    "bill_meter", bi->get_bill_meter()->get_meter_number()
                },
                {
                    "bill_unit", bi->get_bill_unit()->get_unit_price()
                },
            };
        }
        return {};
    }

    static boost::json::array bills_to_json(const std::vector<boost::shared_ptr<bill>>& bills) {
        boost::json::array json_array;
        for (const auto& bi : bills) {
            json_array.emplace_back(bill_to_json(bi));
        }
        return json_array;
    }

    // GET all bills
    static void get(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::get) {
            std::cout << "http::verb::get called" << std::endl;

            auto bills = BillController::getAllBills(handle);
            auto bills_json = bills_to_json(bills);
            std::string jsonString = boost::json::serialize(bills_json);

            res.set(http::field::content_type, "application/json");
            res.body() = jsonString;
            res.prepare_payload();
        }
    }

    // GET bill by UUID
    static void getByUuid(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            std::cout << "getByUuid() " <<endl;
            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string bill_unique = uuid->second;
                auto bill_d = BillController::getBillByUuid(handle, bill_unique);
                if (bill_d) {
                    boost::json::object bill_json = bill_to_json(bill_d);
                    std::string jsonString = boost::json::serialize(bill_json);
                    res.set(http::field::content_type, "application/json");
                    res.body() = jsonString;
                } else {
                    res.result(http::status::not_found);
                    res.body() = R"({"error": "Bill not found"})";
                }
                res.prepare_payload();
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "UUID parameter is missing."})";
                res.prepare_payload();
            }
        }
    }

    // POST to create a new bill
    static void post(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            std::cout << "http::verb::post Bill called" << std::endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string bill_unique = boost::uuids::to_string(uuid);
            std::string bill_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string meter_unique = safe_get_value<std::string>(jsonBody, "meter_unique", "");
            std::string meter_read = safe_get_value<std::string>(jsonBody, "meter_read", "");

            // get meter if it is valid - 1
            std::cout << "meter_unique " << meter_unique <<endl;
            auto meter_d = MeterController::getMeterByUiid(handle, meter_unique); 
            if(!meter_d){
                std::cout << "called here 2 " <<endl;
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Failed to get meter data"})";
                res.prepare_payload();
                return;
            }
            // get last leading 
            auto total_units_used = BillController::sumBillUnitsUsed(handle, meter_d->get_meter_id());
            
            // now let get unit pointer
            auto branch_d = meter_d->get_meter_customer()->get_customer_branch();
            auto unit_d = UnitController::getLastUnitByBranch(handle, branch_d);

            // from last reading get the current on.
            auto unit_u = std::stoi(meter_read) - total_units_used;
            // std::cout << "TT : " << unit_u << endl;
            if(0 > unit_u){
                std::cout << "called here 3 " <<endl;
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Units can not be negative"})";
                res.prepare_payload();
                return;
            }

            auto cost_ = unit_u * std::stoi(unit_d->get_unit_price());
            std::ostringstream cost_stream;
            cost_stream << std::fixed << std::setprecision(2) << cost_;
            std::string formatted_cost = cost_stream.str();

            std::ostringstream cost_stream_1;
            cost_stream_1 << std::fixed << std::setprecision(2) << unit_u;
            std::string formatted_unit_u = cost_stream_1.str();

            boost::shared_ptr<bill> bill_d = boost::make_shared<bill>(
                bill_unique,
                meter_d->get_meter_customer(),
                meter_d,
                unit_d,
                formatted_unit_u,
                formatted_cost,
                bill_reg_date
            );

            if (BillController::createBill(handle, bill_d)) {
                // send message here
                std::string sms = "";
                res.result(http::status::ok);
                res.body() = R"({"message": "Bill created successfully!"})";
                res.prepare_payload();
                return;
            } else {
                std::cout << "called here 4 " <<endl;
                res.result(http::status::bad_request);
                res.body() = R"({"error": "Failed to create bill"})";
                res.prepare_payload();
                return;
            }
        }
    }

    // PUT to update an existing bill
    static void put(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::put) {
            std::cout << "http::verb::put Bill called" << std::endl;
            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            if (BillController::updateBill(handle, parsedValue)) {
                res.result(http::status::ok);
                res.body() = R"({"message": "Bill updated successfully!"})";
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"error": "Failed to update bill"})";
            }
            res.prepare_payload();
        }
    }

    // DELETE a bill by UUID
    static void delete_data(
        std::shared_ptr<odb::mysql::database>& handle,
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            std::cout << "http::verb::delete Bill called" << std::endl;
            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string bill_unique = uuid->second;
                if (BillController::deleteBill(handle, bill_unique)) {
                    res.result(http::status::ok);
                    res.body() = R"({"message": "Bill deleted successfully!"})";
                } else {
                    res.result(http::status::bad_request);
                    res.body() = R"({"error": "Failed to delete bill"})";
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