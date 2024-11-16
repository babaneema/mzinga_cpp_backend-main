#pragma once
#include "controllers/billController.hxx"
#include "controllers/unitController.hxx"
#include "controllers/paymentController.hxx"

#include "http/meterHttp.hxx"

#include "core/sms.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"
#include "core/database.hxx"
#include "core/server.hxx"
#include "core/session_macro.hxx"


#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/json.hpp>
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
                    "bill_customer_contact", bi->get_bill_customer()->get_customer_contact()
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
        const http::request<http::string_body>& req, 
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            std::cout << "BillHttp::get Got Called " <<endl;
            auto page = query_params.find("page");
            auto page_size = query_params.find("pageSize");

            logger("BillHttp::get", "Called");

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

            
            response_json["bill_data"] = BillController::getAllBills(handle);
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

    // GET bill by UUID
    static void getByUuid(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            logger("BillHttp::getByUuid", "Called");

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();


            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            auto uuid = query_params.find("uuid");
            if (uuid != query_params.end()) {
                std::string bill_unique = uuid->second;
                auto bill_d = BillController::getBillByUuid(handle, bill_unique);
                if (bill_d) {
                    response_json["bill_data"]  = bill_to_json(bill_d);
                    std::string jsonString = boost::json::serialize(response_json);
                    res.set(http::field::content_type, "application/json");
                    res.body() = jsonString;
                    res.prepare_payload();
                    return;
                } else {
                    res.result(http::status::not_found);
                    res.body() = R"({"auth": "true","permission": "true","error": "Bill not found"})";
                    res.prepare_payload();
                    return;
                }
                
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

    // get all bils by customer
    static void getBillsByMeter(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::get) {
            logger("BillHttp::getBillsByMeter", "Called");

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();


            boost::json::object response_json;
            response_json["auth"] = "true";
            response_json["permission"] = "true";

            auto customer = query_params.find("customer");
            if (customer != query_params.end()) {
                std::string customer_unique = customer->second;
                // auto bill_d = BillController::getBillByUuid(handle, bill_unique);
                // if (bill_d) {
                //     response_json["bill_data"]  = bill_to_json(bill_d);
                //     std::string jsonString = boost::json::serialize(response_json);
                //     res.set(http::field::content_type, "application/json");
                //     res.body() = jsonString;
                //     res.prepare_payload();
                //     return;
                // } else {
                //     res.result(http::status::not_found);
                //     res.body() = R"({"auth": "true","permission": "true","error": "Bill not found"})";
                //     res.prepare_payload();
                //     return;
                // }
                
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

    static void queryBill(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ){
        if (req.method() == http::verb::get) {
            logger("BillHttp::queryBill", "Called");
            
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
                // all, paid, partial, unpaid
                if(status != "all"){
                    if(status == "paid"){
                        response_json["bill_data"] = BillController::getFullyPaidBillIds(handle);
                        std::string jsonString = boost::json::serialize(response_json);

                        res.set(http::field::content_type, "application/json");
                        res.body() = jsonString;
                        res.prepare_payload();
                        return;
                    }
                }else{
                    get(req,res,query_params); // doble check
                }
            }else{
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

    // POST to create a new bill
    static void post(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::post) {
            logger("BillHttp::post", "Called");

            boost::shared_ptr<employee> employee_session;
            CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_session);
            std::string administrative = employee_session->get_employee_administrative();

            boost::json::value parsedValue = boost::json::parse(req.body());

            if (!parsedValue.is_object()) {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Invalid JSON format. Expected an object."})";
                res.prepare_payload();
                return;
            }

            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string bill_unique = boost::uuids::to_string(uuid);
            std::string bill_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();
            std::string meter_unique = safe_get_value<std::string>(jsonBody, "meter_unique", "");
            std::string meter_read = safe_get_value<std::string>(jsonBody, "meter_read","");

            // get meter if it is valid - 1
            auto meter_d = MeterController::getMeterByUiid(handle, meter_unique); 
            if(!meter_d){
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Failed to get meter data"})";
                res.prepare_payload();
                return;
            }
            // get last leading 
            auto total_units_used = BillController::sumBillUnitsUsed(handle, meter_d->get_meter_id());
            auto total_bill_cost = BillController::sumBillCostUsed(handle, meter_d->get_meter_id());
            auto total_customer_payment  = PaymentController::sumPayAmount(handle, 
                                meter_d->get_meter_customer()->get_customer_id());

            auto total_debt = total_bill_cost - total_customer_payment;
 

            // now let get unit pointer
            auto branch_d = meter_d->get_meter_customer()->get_customer_branch();
            auto unit_d = UnitController::getLastUnitByBranch(handle, branch_d);

            // from last reading get the current on.
            total_units_used += std::stoi(meter_d->get_meter_intital_unit());
            auto unit_u = std::stoi(meter_read) - total_units_used;

            if(0 > unit_u){
                std::cout << "called here 3 " <<endl;
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "false","error": "Units can not be negative"})";
                res.prepare_payload();
                return;
            }

            auto cost_ = unit_u * std::stoi(unit_d->get_unit_price());
            auto new_dept = total_debt + cost_;
            
            std::string formatted_cost = formatedValue(cost_);
            std::string formatted_unit_u = formatedValue(unit_u);

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

                std::string massage = "Habari, Umetumia Units :  " +formatted_unit_u+
                                    " Kutoka  " +formatedValue(total_units_used)+
                                    " Hadi " +meter_read+ 
                                    " Deni lako la nyuma ni Tsh  " +formatedValue(total_debt)+
                                    " Jumla ya deni lako ni Tsh : " +formatedValue(new_dept);

                std::string phone_number = meter_d->get_meter_customer()->get_customer_contact();
                
                sendSingleSms(phone_number, massage, company_name);
                std::string sms = "";
                res.result(http::status::ok);
                res.body() = R"({"auth": "true","permission": "false","message": "Bill created successfully!"})";
                res.prepare_payload();
                return;
            } else {
                std::cout << "called here 4 " <<endl;
                res.result(http::status::bad_request);
                res.body() = R"({"auth": "true","permission": "false","error": "Failed to create bill"})";
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

    // PUT to update an existing bill
    static void put(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::put) {
            logger("BillHttp::put", "Called");

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

            if (BillController::updateBill(handle, parsedValue)) {
                res.result(http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Bill updated successfully!"})";
                res.prepare_payload();
                return;
            } else {
                res.result(http::status::bad_request);
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to update bill"})";
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

    // DELETE a bill by UUID
    static void delete_data(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            logger("BillHttp::delete", "Called");

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
                std::string bill_unique = uuid->second;
                if (BillController::deleteBill(handle, bill_unique)) {
                    res.result(http::status::ok);
                    res.body() = R"({"auth": "true","permission": "true","message": "Bill deleted successfully!"})";
                    res.prepare_payload();
                    return;
                } else {
                    res.result(http::status::bad_request);
                    res.body() = R"({"auth": "true","permission": "true","error": "Failed to delete bill"})";
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