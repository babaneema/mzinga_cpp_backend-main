#pragma once
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <odb/mysql/database.hxx>
#include <odb/transaction.hxx>
#include <odb/query.hxx>
#include <odb/schema-catalog.hxx>
#include <odb/result.hxx>
#include <memory>
#include "../models/activity.hxx"
#include "../models/activity-odb.hxx"
#include <boost/json.hpp>

class ActivityController {
public:
    typedef odb::query<activity> query;
    typedef odb::result<activity> result;

    static boost::json::object activity_to_json(const std::shared_ptr<activity> activity) {
        if (activity) {
            return {
                {"activity_id", activity->id()},
                {"activity_unique", activity->unique()},
                {"activity_name", activity->name()},
                {"activity_price", activity->price()},
                {"activity_reg_date", activity->reg_date()}
            };
        }
        return {};
    }

    static boost::json::array activity_to_json(const std::vector<std::shared_ptr<activity>> activities) {
        boost::json::array json_array;
        for (const auto& activity : activities) {
            json_array.emplace_back(activity_to_json(activity));
        }
        return json_array;
    }

    static auto getAllActivities(std::shared_ptr<odb::mysql::database>& db) {
        std::vector<std::shared_ptr<activity>> activities;
        try {
            odb::transaction t(db->begin());
            odb::result<activity> r(db->query<activity>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                activities.emplace_back(std::make_shared<activity>(*i));
            }
            t.commit();
        } catch (const std::exception& e) {
            std::cout << "Error retrieving activities: " << e.what() << std::endl;
        }
        return activities;
    }

    static auto getActivityByUuid(std::shared_ptr<odb::mysql::database>& db, const std::string& uuid) {
        std::shared_ptr<activity> result_activity;
        try {
            odb::transaction t(db->begin());
            activity* raw_activity = db->query_one<activity>(odb::query<activity>::activity_unique == uuid);
            if (raw_activity) {
                result_activity = std::shared_ptr<activity>(raw_activity);
            }
            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error fetching activity by UUID: " << e.what() << std::endl;
        }
        return result_activity;
    }

    static auto getActivityById(std::shared_ptr<odb::mysql::database>& db, const unsigned long& id) {
        std::shared_ptr<activity> result_activity;
        try {
            odb::transaction t(db->begin());
            activity* raw_activity = db->query_one<activity>(odb::query<activity>::activity_id == id);
            if (raw_activity) {
                result_activity = std::shared_ptr<activity>(raw_activity);
            }
            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error fetching activity by ID: " << e.what() << std::endl;
        }
        return result_activity;
    }

    static bool createActivity(std::shared_ptr<odb::mysql::database>& db, const std::string& name, int price) {
        try {
            odb::transaction t(db->begin());
            
            boost::uuids::random_generator gen;
            boost::uuids::uuid uuid = gen();
            std::string unique = boost::uuids::to_string(uuid);

            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
            std::string reg_date = ss.str();

            activity new_activity(unique, name, price, reg_date);
            db->persist(new_activity);
            
            t.commit();
            return true;
        } catch (const odb::exception& e) {
            std::cerr << "ODB Error: " << e.what() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error creating activity: " << e.what() << std::endl;
            return false;
        }
    }

    static bool updateActivity(std::shared_ptr<odb::mysql::database>& db, const std::string& uuid, const std::string& name, int price) {
        try {
            // odb::transaction t(db->begin());
            
            // auto activity = getActivityByUuid(db, uuid);
            // if (!activity) {
            //     std::cerr << "Activity not found" << std::endl;
            //     return false;
            // }

            // activity->activity_name = name;
            // activity->activity_price = price;

            // db->update(*activity);
            
            // t.commit();
            return true;
        } catch (const odb::exception& e) {
            std::cerr << "ODB Error: " << e.what() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error updating activity: " << e.what() << std::endl;
            return false;
        }
    }

    static bool deleteActivity(std::shared_ptr<odb::mysql::database>& db, const std::string& uuid) {
        try {
            odb::transaction t(db->begin());
            
            auto activity = getActivityByUuid(db, uuid);
            if (!activity) {
                std::cerr << "Activity not found" << std::endl;
                return false;
            }

            db->erase(*activity);
            
            t.commit();
            return true;
        } catch (const odb::exception& e) {
            std::cerr << "ODB Error: " << e.what() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error deleting activity: " << e.what() << std::endl;
            return false;
        }
    }
};