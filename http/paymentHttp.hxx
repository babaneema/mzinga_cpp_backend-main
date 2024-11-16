#pragma once
#include "core/database.hxx"
#include "core/server.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>

#include "core/sms.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"
#include "core/session.hxx"
#include "core/session_macro.hxx"

#include "controllers/paymentController.hxx"
#include "controllers/customerController.hxx"
#include "controllers/billController.hxx"
#include "controllers/unitController.hxx"

class PaymentHttp {
public:
    // Convert a single payment to JSON
    static boost::json::object payment_to_json(const boost::shared_ptr<payments> payment) {
        if (payment) {
            return {
                {"pay_id", payment->get_pay_id()},
                {"pay_unique", payment->get_pay_unique()},
                {"pay_method", payment->get_pay_method()},
                {"pay_type", payment->get_pay_type()},
                {"pay_amount", payment->get_pay_amount()},
                {"pay_reg_date", payment->get_pay_reg_date()},
                {
                    "unit_used", payment->get_pay_bill()->get_bill_unit_used()
                },
                {
                    "total_cost", payment->get_pay_bill()->get_bill_cost()
                }
            };
        }
        return {};
    }

    // Convert a vector of payments to JSON array
    static boost::json::array payments_to_json(const std::vector<boost::shared_ptr<payments>> payments_d) {
        boost::json::array json_array;
        for (const auto& payment : payments_d) {
            json_array.emplace_back(payment_to_json(payment));
        }
        return json_array;
    }

    // HTTP GET request handler for fetching payments
    static void get(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    ) {
        if (req.method() == http::verb::get) {
            logger("PaymentHttp::get", "Called");

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

            // Fetch all payments from the controller
            auto payments = PaymentController::getAllPayments(handle);
            response_json["payment_data"] = payments_to_json(payments);
            std::string jsonString = boost::json::serialize(response_json);

            // Set the response content type and payload
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

    static void payBill(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res
    )
    {
        if(req.method() == http::verb::post){
            logger("PaymentHttp::post", "Called");

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
            std::string pay_unique = boost::uuids::to_string(uuid);

            std::string pay_reg_date = getCurrentDate();

            const boost::json::object& jsonBody = parsedValue.as_object();

            std::string pay_bill = safe_get_value<std::string>(jsonBody, "pay_bill", "1");
            std::string pay_customer = safe_get_value<std::string>(jsonBody, "pay_customer", "1");
            std::string pay_method = "Bank";
            std::string pay_type = "Bill";
            std::string pay_amount = safe_get_value<std::string>(jsonBody, "pay_amount", "0");
            

            auto customer_d = CustomerController::getCustomerByUiid(handle, pay_customer);
            
            if(!customer_d){ // this should not happen
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get customer data"})";
                res.prepare_payload();
                return;
            }

            auto bill_d = BillController::getBillByUiid(handle, pay_bill);
            
            if(!bill_d){ // this should not happen
                std::cout << "Here 3" << endl;
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to get bills data"})";
                res.prepare_payload();
                return;
            }


            boost::shared_ptr<payments> payments_d = boost::make_shared<payments>(
                pay_unique, 
                bill_d, 
                customer_d,
                pay_method,
                pay_type,
                pay_amount,
                pay_reg_date
            );

            if (PaymentController::createPayment(handle, payments_d)) {

                auto meter_id = bill_d->get_bill_meter()->get_meter_id();
                auto total_units_used = BillController::sumBillUnitsUsed(handle, meter_id);
                auto total_bill_cost = BillController::sumBillCostUsed(handle, meter_id);

                auto total_customer_payment  = PaymentController::sumPayAmount(handle, customer_d->get_customer_id());

                auto total_debt = total_bill_cost - total_customer_payment;

                std::string massage = "Habari, Umefanya malipo ya shiling : " +pay_amount+
                                      " Deni ni : "+formatedValue(total_debt);

                std::string phone_number = customer_d->get_customer_contact();
                sendSingleSms(phone_number, massage, company_name);

                res.version(req.version());
                res.result(beast::http::status::ok);
                res.body() = R"({"auth": "true","permission": "true","message": "Payment accepted successfully!"})";
            }else{              
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "Failed to do payment"})";
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

     // DELETE a bill by UUID
    static void delete_data(
        const http::request<http::string_body>& req,
        http::response<http::string_body>& res,
        const std::unordered_map<std::string, std::string>& query_params
    ) {
        if (req.method() == http::verb::delete_) {
            logger("PaymentHttp::delete", "Called");

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
                std::string pay_unique = uuid->second;
                if (PaymentController::deletePayment(handle, pay_unique)) {
                    res.result(http::status::ok);
                    res.body() = R"({"auth": "true","permission": "true","message": "Payment deleted successfully!"})";
                } else {
                    res.result(http::status::bad_request);
                    res.body() = R"({"auth": "true","permission": "true","error": "Failed to delete payment"})";
                }
                res.prepare_payload();
            } else {
                res.result(http::status::bad_request);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"auth": "true","permission": "true","error": "UUID parameter is missing."})";
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
};
