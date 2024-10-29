#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <odb/pgsql/database.hxx>
#include <odb/transaction.hxx>
#include <odb/query.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/result.hxx>
#include <memory>
#include "../models/checkin.hxx"
#include "../models/checkin-odb.hxx"
#include <boost/json.hpp>
#include "employeeController.hxx"

#include <odb/mysql/tracer.hxx>
#include <odb/tracer.hxx>

class CheckinController {
public:
    typedef odb::query<checkin> query;
    typedef odb::result<checkin> result;

    static boost::json::object checkin_to_json(const std::shared_ptr<checkin> checkin) {
        if(checkin){
            return {
                {"checkin_id", checkin->id()},
                {"checkin_unique", checkin->unique()},
                {"checkin_employee_id", checkin->employee_id()},
                {"checkin_time_in", checkin->time_in()},
                {"checkin_time_out", checkin->time_out()},
                {"checkin_reg_date", checkin->reg_date()}
            };
        }
        return {};
    }

    static boost::json::array checkin_to_json(const std::vector<std::shared_ptr<checkin>> checkins) {
        boost::json::array json_array;
        for (const auto& checkin : checkins) {
            json_array.emplace_back(checkin_to_json(checkin));
        }
        return json_array;
    }

    static auto getAllCheckins(std::shared_ptr<odb::mysql::database> & db) {
        std::vector<std::shared_ptr<checkin>> checkins;
        try {
            odb::transaction t(db->begin());
            odb::result<checkin> r(db->query<checkin>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                checkins.emplace_back(std::make_shared<checkin>(*i));
            }
            t.commit();

        } catch (const std::exception& e) {
            std::cout << "Error retrieving checkins: " << e.what() << std::endl;
        }
        
        return checkins;
    }

    static auto getAllCheckinByUuid(std::shared_ptr<odb::mysql::database>  & db, const std::string& uuid) {
        std::shared_ptr<checkin> result_checkin;
        try {
            odb::transaction t(db->begin());
            checkin * raw_check = db->query_one<checkin>(odb::query<checkin>::checkin_unique == uuid);

            if (raw_check) {
                result_checkin = std::shared_ptr<checkin>(raw_check);
            } 

            t.commit();

        } catch (const std::exception& e) {
            std::cerr << "Error fetching branch by UUID: " << e.what() << std::endl;
        }

        return result_checkin;
    }

     static auto getAllCheckinByEmployee(std::shared_ptr<odb::mysql::database>  & db, const int& id) {
        std::vector<std::shared_ptr<checkin>> result_checkin;
        try {
            odb::transaction t(db->begin());
            odb::result<checkin> r(db->query<checkin>(odb::query<checkin>::checkin_employee == id));
            for (auto i = r.begin(); i != r.end(); ++i) {
                result_checkin.emplace_back(std::make_shared<checkin>(*i));
            }

            t.commit();

        } catch (const std::exception& e) {
            std::cerr << "Error fetching branch by UUID: " << e.what() << std::endl;
        }

        return result_checkin;
    }

    static bool getCheckinEmployees(std::shared_ptr<odb::mysql::database>& db) {
        try {
            std::vector<std::shared_ptr<checkin>> checkins;
            odb::transaction t(db->begin());

            
        } catch (const odb::exception& e) {
            std::cerr << "ODB Error: " << e.what() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error retrieving checkins: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

};