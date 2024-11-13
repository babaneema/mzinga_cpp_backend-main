#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <odb/result.hxx>
#include <odb/pgsql/database.hxx>
#include <odb/transaction.hxx>
#include <odb/query.hxx> 
#include <odb/schema-catalog.hxx>
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>

#include <odb/session.hxx>

#include "core/safe_json.hxx"

#include "../models/bill.hxx" 
#include "../models/bill-odb.hxx"
#include "../models/payment.hxx"
#include "../models/payment-odb.hxx"


class BillController {
public: 
    typedef odb::query<bill> query;
    typedef odb::result<bill> result;


    static boost::json::array getAllBills(const std::shared_ptr<odb::mysql::database> &db) {
      std::cout << "getAllBills called " <<std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        boost::json::array bills_json;

        try {
            std::string query = R"(
                SELECT 
                    b.bill_id, 
                    b.bill_cost, 
                    b.bill_unit_used,
                    b.bill_reg_date,
                    m.customer_name, 
                    m.customer_contact 
                FROM bill b
                LEFT JOIN customer m ON b.bill_customer = m.customer_id 
                GROUP BY b.bill_id, m.customer_name, m.customer_contact
                ORDER BY b.bill_id
            )";

            odb::transaction t(db->begin());

            auto conn = dynamic_cast<odb::mysql::connection*>(db->connection().get());
            if (!conn) {
                throw std::runtime_error("Failed to get MySQL connection");
            }
        
            if (mysql_query(conn->handle(), query.c_str()) != 0) {
                throw std::runtime_error(mysql_error(conn->handle()));
            }
        
            MYSQL_RES* result = mysql_store_result(conn->handle());
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                unsigned long* lengths = mysql_fetch_lengths(result);
                if (lengths && row[0]) {
                    boost::json::object bill_json;

                    bill_json["bill_id"] = std::stoi(row[0]);
                    bill_json["bill_cost"] = row[1] ? std::stod(row[1]) : 0.0;
                    bill_json["bill_unit_used"] = row[2] ? std::stod(row[2]) : 0.0;
                    bill_json["bill_reg_date"] = row[3] ? row[3] : "";
                    bill_json["bill_customer"] = row[4] ? row[4] : "";
                    bill_json["bill_customer_contact"] = row[5] ? row[5] : "";
                    
                    bills_json.emplace_back(std::move(bill_json));
                }
            }
            
            // Clean up
            mysql_free_result(result);
            t.commit();

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        // Output the elapsed time in seconds
        std::cout << "Execution getAllBills time: " << duration.count() << " seconds" << std::endl;

        return  bills_json;
    }


    static boost::json::array getFullyPaidBillIds(const std::shared_ptr<odb::mysql::database>& db) {
        std::cout << "getFullyPaidBillIds called " <<std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        boost::json::array bills_json;

        try {
            std::string query = R"(
                SELECT 
                    b.bill_id, 
                    b.bill_cost, 
                    b.bill_unit_used,
                    b.bill_reg_date,
                    m.customer_name, 
                    m.customer_contact 
                FROM bill b
                LEFT JOIN payments p ON b.bill_id = p.pay_bill
                LEFT JOIN customer m ON b.bill_customer = m.customer_id 
                GROUP BY b.bill_id, m.customer_name, m.customer_contact
                HAVING SUM(CAST(p.pay_amount AS DECIMAL(10,2))) >= CAST(CAST(b.bill_cost AS CHAR) AS DECIMAL(10,2))
                ORDER BY b.bill_id
            )";

            odb::transaction t(db->begin());

            auto conn = dynamic_cast<odb::mysql::connection*>(db->connection().get());
            if (!conn) {
                throw std::runtime_error("Failed to get MySQL connection");
            }
        
            if (mysql_query(conn->handle(), query.c_str()) != 0) {
                throw std::runtime_error(mysql_error(conn->handle()));
            }
        
            MYSQL_RES* result = mysql_store_result(conn->handle());
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                unsigned long* lengths = mysql_fetch_lengths(result);
                if (lengths && row[0]) {
                    boost::json::object bill_json;

                    bill_json["bill_id"] = std::stoi(row[0]);
                    bill_json["bill_cost"] = row[1] ? std::stod(row[1]) : 0.0;
                    bill_json["bill_unit_used"] = row[2] ? std::stod(row[2]) : 0.0;
                    bill_json["bill_reg_date"] = row[3] ? row[3] : "";
                    bill_json["bill_customer"] = row[4] ? row[4] : "";
                    bill_json["bill_customer_contact"] = row[5] ? row[5] : "";
                    
                    bills_json.emplace_back(std::move(bill_json));
                }
            }
            
            // Clean up
            mysql_free_result(result);
            t.commit();

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        // Output the elapsed time in seconds
        std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;

        return  bills_json;
    }



    static auto getBillsByCustomerId(std::shared_ptr<odb::mysql::database> & db, const int & id) {
        std::vector<boost::shared_ptr<bill>> bills;
        try {
            odb::transaction t(db->begin());
            odb::result<bill> r(db->query<bill>(query::bill_customer == id));
            for (auto i = r.begin(); i != r.end(); ++i) {
                bills.emplace_back(boost::make_shared<bill>(*i));
            }
            t.commit();
            return bills;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching bills by customer ID: " << e.what() << std::endl;
            return bills;
        }
    }

    static boost::shared_ptr<bill> getBillByUiid(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<bill> bill_data (
                db->query_one<bill>(query::bill_unique == uuid));
            t.commit();
            return bill_data;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching bill by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static boost::shared_ptr<bill> getBillByUuid(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid){
        try{
            odb::transaction t(db->begin());
            boost::shared_ptr<bill> bill_data (
                db->query_one<bill> ( query::bill_unique == uuid));
            t.commit();
            return bill_data;

        }catch (const std::exception& e) {
            std::cerr << "Error here: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static bool createBill(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<bill> bill_d){
        try{
            odb::transaction t(db->begin());
            db->persist(bill_d);
            t.commit();
            return true;
        }catch (const std::exception& e) {
            std::cerr << "Error : " << e.what() << std::endl;
            return false;
        }
    }

    static auto updateBill(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values) {
        try {
            const boost::json::object& jsonBody = values.as_object();

            std::string bill_uuid = safe_get_value<std::string>(jsonBody, "bill_unique", "Bill UUID");
            std::string unit_used = safe_get_value<std::string>(jsonBody, "bill_unit_used", "Unit Used");
            std::string cost = safe_get_value<std::string>(jsonBody, "bill_cost", "Bill Cost");

            auto bill_d = getBillByUiid(db, bill_uuid);
            if (!bill_d) {
                return false;
            }

            bill_d->set_bill_unit_used(unit_used);
            bill_d->set_bill_cost(cost);

            odb::transaction t(db->begin());
            db->update(bill_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error updating bill: " << e.what() << std::endl;
            return false;
        }
    }

    static auto deleteBill(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            auto bill_d = getBillByUiid(db, uuid);
            if (!bill_d) {
                std::cout << "Could not find bill" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(bill_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error deleting bill: " << e.what() << std::endl;
            return false;
        }
    }

    static double sumBillUnitsUsed(const std::shared_ptr<odb::mysql::database> & db, const int & meter_id) {
        double total_units_used = 0.0;

        try {
            odb::transaction t(db->begin());
            odb::result<bill> r(db->query<bill>(query::bill_meter == meter_id));

            for (auto i = r.begin(); i != r.end(); ++i) {
                try {
                    // Convert bill_unit_used to double and add to total
                    total_units_used += std::stod(i->get_bill_unit_used());
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid bill_unit_used format for bill ID " << i->get_bill_id() << ": " << e.what() << std::endl;
                }
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error summing bill units used: " << e.what() << std::endl;
        }

        return total_units_used;
    }


    static double sumBillCostUsed(const std::shared_ptr<odb::mysql::database> & db, const int & meter_id) {
        double total_bill_cost = 0.0;

        try {
            odb::transaction t(db->begin());
            odb::result<bill> r(db->query<bill>(query::bill_meter == meter_id));

            for (auto i = r.begin(); i != r.end(); ++i) {
                try {
                    // Convert bill_cost to double and add to total
                    total_bill_cost += std::stod(i->get_bill_cost());
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid bill_cost format for bill ID " << i->get_bill_id() << ": " << e.what() << std::endl;
                }
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error summing bill units used: " << e.what() << std::endl;
        }

        return total_bill_cost;
    }

};
