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
#include "../models/bill.hxx" 
#include "../models/bill-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"

class BillController {
public: 
    typedef odb::query<bill> query;
    typedef odb::result<bill> result;

    static auto getAllBills(const std::shared_ptr<odb::mysql::database> & db) {
        std::vector<boost::shared_ptr<bill>> bills;
        try {
            odb::transaction t(db->begin());
            odb::result<bill> r(db->query<bill>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                bills.emplace_back(boost::make_shared<bill>(*i));
            }
            t.commit();
            return bills;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching bills: " << e.what() << std::endl;
            return bills;
        }
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
            // odb::result<bill> r(db->query<bill>());
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

};
