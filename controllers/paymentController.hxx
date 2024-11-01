#pragma once
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <odb/pgsql/database.hxx>
#include <odb/transaction.hxx>
#include <odb/query.hxx> 
#include <odb/schema-catalog.hxx>
#include <odb/result.hxx>
#include <memory>
#include "../models/payment.hxx"
#include "../models/payment-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"

class PaymentController {
public:
    typedef odb::query<payments> query;
    typedef odb::result<payments> result;

    // Fetch all payments
    static auto getAllPayments(const std::shared_ptr<odb::mysql::database> & db) {
        std::vector<boost::shared_ptr<payments>> paymentList;
        try {
            odb::transaction t(db->begin());
            odb::result<payments> r(db->query<payments>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                paymentList.emplace_back(boost::make_shared<payments>(*i));
            }
            t.commit();
            return paymentList;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching payments: " << e.what() << std::endl;
            return paymentList;
        }
    }

    // Fetch payments by customer ID
    static auto getPaymentsByCustomerId(const std::shared_ptr<odb::mysql::database> & db, const int & customerId) {
        std::vector<boost::shared_ptr<payments>> paymentList;
        try {
            odb::transaction t(db->begin());
            odb::result<payments> r(db->query<payments>(query::pay_customer == customerId));
            for (auto i = r.begin(); i != r.end(); ++i) {
                paymentList.emplace_back(boost::make_shared<payments>(*i));
            }
            t.commit();
            return paymentList;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching payments by customer ID: " << e.what() << std::endl;
            return paymentList;
        }
    }

    // Fetch payments by bill ID
    static auto getPaymentsByBillId(const std::shared_ptr<odb::mysql::database> & db, const int & billId) {
        std::vector<boost::shared_ptr<payments>> paymentList;
        try {
            odb::transaction t(db->begin());
            odb::result<payments> r(db->query<payments>(query::pay_bill == billId));
            for (auto i = r.begin(); i != r.end(); ++i) {
                paymentList.emplace_back(boost::make_shared<payments>(*i));
            }
            t.commit();
            return paymentList;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching payments by bill ID: " << e.what() << std::endl;
            return paymentList;
        }
    }

    // Fetch payment by UUID
    static boost::shared_ptr<payments> getPaymentByUuid(const std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<payments> payment_data(
                db->query_one<payments>(query::pay_unique == uuid));
            t.commit();
            return payment_data;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching payment by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static bool createPayment(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<payments> payments_d){
        try{
            odb::transaction t(db->begin());
            db->persist(payments_d);
            t.commit();
            return true;
        }catch (const std::exception& e) {
            std::cerr << "Error : " << e.what() << std::endl;
            return false;
        }
    }

    // Update payment
    static auto updatePayment(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values) {
        try {
            const boost::json::object& jsonBody = values.as_object();

            std::string pay_uuid = safe_get_value<std::string>(jsonBody, "pay_unique", "Payment UUID");
            std::string method = safe_get_value<std::string>(jsonBody, "pay_method", "Payment Method");
            std::string type = safe_get_value<std::string>(jsonBody, "pay_type", "Payment Type");
            std::string amount = safe_get_value<std::string>(jsonBody, "pay_amount", "Payment Amount");

            auto payment_d = getPaymentByUuid(db, pay_uuid);
            if (!payment_d) {
                return false;
            }

            payment_d->set_pay_method(method);
            payment_d->set_pay_type(type);
            payment_d->set_pay_amount(amount);

            odb::transaction t(db->begin());
            db->update(payment_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error updating payment: " << e.what() << std::endl;
            return false;
        }
    }

    // Delete payment by UUID
    static auto deletePayment(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            auto payment_d = getPaymentByUuid(db, uuid);
            if (!payment_d) {
                std::cout << "Payment not found" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(payment_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error deleting payment: " << e.what() << std::endl;
            return false;
        }
    }

    static double sumPayAmount(const std::shared_ptr<odb::mysql::database> & db, const int & customer_id) {
        double total_payment = 0.0;

        try {
            odb::transaction t(db->begin());
            odb::result<payments> r(db->query<payments>(query::pay_customer == customer_id));

            for (auto i = r.begin(); i != r.end(); ++i) {
                try {
                    // Convert bill_cost to double and add to total
                    total_payment += std::stod(i->get_pay_amount());
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid pay_amount format for bill ID " << i->get_pay_amount() << ": " << e.what() << std::endl;
                }
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error summing bill units used: " << e.what() << std::endl;
        }

        return total_payment;
    }
};
