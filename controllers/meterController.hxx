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
#include "../models/meter.hxx" 
#include "../models/meter-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"
#include "../models/customer.hxx" 
#include "../models/customer-odb.hxx"

class MeterController {
public:
    typedef odb::query<meter> query;
    typedef odb::result<meter> result;

    static auto getAllMeter(const std::shared_ptr<odb::mysql::database>  & db) {
        std::vector<boost::shared_ptr<meter>> meters;
        try {
            odb::transaction t(db->begin());
            odb::result<meter> r(db->query<meter>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                meters.emplace_back(boost::make_shared<meter>(*i));
            }
            t.commit();
            return meters;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching meters: " << e.what() << std::endl;
            return meters;
        }
    }

    static auto getMeterByCustomerId(std::shared_ptr<odb::mysql::database> & db, const int & id) {
        std::vector<boost::shared_ptr<meter>> meters;
        try{
            odb::transaction t(db->begin());
            odb::result<meter> r(db->query<meter>(query::meter_customer == id));
            for (auto i = r.begin(); i != r.end(); ++i) {
                meters.emplace_back(boost::make_shared<meter>(*i));
            }
            t.commit();
            return meters;

        }catch (const std::exception& e) {
            std::cerr << "Error fetching meters by customer ID: " << e.what() << std::endl;
            return meters;
        }
    }

    static boost::shared_ptr<meter> getMeterByUiid(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<meter> meter_data (
                db->query_one<meter>(query::meter_unique == uuid));
            t.commit();
            return meter_data;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching meter by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static bool createMeter(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<meter> meter_d){
        try{
            odb::transaction t(db->begin());
            db->persist(meter_d);
            t.commit();
            return true;
        }catch (const std::exception& e) {
            std::cerr << "Error : " << e.what() << std::endl;
            return false;
        }
    }

    static auto updateMeter(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values) {
        try {
            const boost::json::object& jsonBody = values.as_object(); // 

            std::string meter_uuid = safe_get_value<std::string>(jsonBody, "meter_unique", "abcd");
            std::string meter_owner = safe_get_value<std::string>(jsonBody, "meter_owner", "Compamy");
            std::string meter_number = safe_get_value<std::string>(jsonBody, "meter_number", "00");
            std::string meter_initial_unit = safe_get_value<std::string>(jsonBody, "meter_initial_unit", "00");
            std::string meter_joinging_price = safe_get_value<std::string>(jsonBody, "meter_joinging_price", "0.00");
            std::string meter_lock = safe_get_value<std::string>(jsonBody, "meter_lock", "No");
            std::string meter_in_service = safe_get_value<std::string>(jsonBody, "meter_in_service", "No");

            auto meter_d = getMeterByUiid(db, meter_uuid);
            if (!meter_d) {
                return false;
            }

            meter_d->set_meter_owner(meter_owner);
            meter_d->set_meter_number(meter_number);
            meter_d->set_meter_intital_unit(meter_initial_unit);
            meter_d->set_meter_joinging_price(meter_joinging_price);
            meter_d->set_meter_lock(meter_lock);
            meter_d->set_meter_in_service(meter_in_service);

            odb::transaction t(db->begin());
            db->update(meter_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error updating meter: " << e.what() << std::endl;
            return false;
        }
    }

    static auto deleteMeter(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            auto meter_d = getMeterByUiid(db, uuid);
            if (!meter_d) {
                std::cout << "Could not find meter" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(meter_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error deleting meter: " << e.what() << std::endl;
            return false;
        }
    }

    // Function to search for a meter by meter number or any three digits of the meter number
    static std::vector<boost::shared_ptr<meter>> searchMeterByNumber(
        std::shared_ptr<odb::mysql::database>& db, const std::string& meter_number) {
        std::vector<boost::shared_ptr<meter>> meters;
        try {
            odb::transaction t(db->begin());

            // Query to find meter numbers with any three digits match
            std::string search_pattern = "%" + meter_number + "%";
            odb::result<meter> r(db->query<meter>(query::meter_number.like(search_pattern)));

            for (auto i = r.begin(); i != r.end(); ++i) {
                meters.emplace_back(boost::make_shared<meter>(*i));
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error searching meter by number: " << e.what() << std::endl;
        }

        return meters;
    }

     static std::vector<boost::shared_ptr<meter>> searchMeterByCustomerUiid(
        std::shared_ptr<odb::mysql::database>& db, boost::shared_ptr<customer> customer_d) {
        std::vector<boost::shared_ptr<meter>> meters;
        auto customer_id = customer_d->get_customer_id();
        try {
            odb::transaction t(db->begin());

            // Query to find meter numbers with any three digits match
            odb::result<meter> r(db->query<meter>(query::meter_customer == customer_id));

            for (auto i = r.begin(); i != r.end(); ++i) {
                meters.emplace_back(boost::make_shared<meter>(*i));
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error searching meter by number: " << e.what() << std::endl;
        }

        return meters;
    }

    static std::vector<boost::shared_ptr<meter>> getMetersByStatus(
        std::shared_ptr<odb::mysql::database>& db, const std::string& status) {
        std::vector<boost::shared_ptr<meter>> meters;
        try {
            odb::transaction t(db->begin());

            odb::result<meter> r;

            // Query based on the provided status
            if (status == "active") {
                r = db->query<meter>(query::meter_in_service == "No");
            } else if (status == "locked") {
                r = db->query<meter>(query::meter_lock == "Yes");
            } else if (status == "unlocked") {
                r = db->query<meter>(query::meter_lock == "No");
            } else if (status == "service") {
                r = db->query<meter>(query::meter_in_service == "No");
            } else {
                std::cerr << "Invalid status provided: " << status << std::endl;
                return meters; // Return empty if invalid status
            }

            // Populate the meters vector with the result
            for (auto i = r.begin(); i != r.end(); ++i) {
                meters.emplace_back(boost::make_shared<meter>(*i));
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error fetching meters by status: " << e.what() << std::endl;
        }

        return meters;
    }

};
