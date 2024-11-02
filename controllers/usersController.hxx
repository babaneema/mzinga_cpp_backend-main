#pragma once
#include <odb/pgsql/database.hxx>
#include <odb/transaction.hxx>
#include <odb/query.hxx> 
#include <odb/schema-catalog.hxx>
#include <odb/result.hxx>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "../models/users.hxx" 
#include "../models/users-odb.hxx"
#include "core/safe_json.hxx"
#include "core/helpers.hxx"

class UserController {
public:
    typedef odb::query<users> query;
    typedef odb::result<users> result;

    // Get all users
    static auto getAllUsers(const std::shared_ptr<odb::mysql::database> &db) {
        std::vector<boost::shared_ptr<users>> usersList;
        try {
            odb::transaction t(db->begin());
            odb::result<users> r(db->query<users>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                usersList.emplace_back(boost::make_shared<users>(*i));
            }
            t.commit();
            return usersList;
        } catch (const std::exception& e) {
            logger("UserController::getAllUsers", e.what());
            std::cerr << "Error fetching users: " << e.what() << std::endl;
            return usersList;
        }
    }

    // Get user by ID
    static boost::shared_ptr<users> getUserById(std::shared_ptr<odb::mysql::database> &db, const unsigned long &id) {
        try {
            boost::shared_ptr<users> user_data = boost::make_shared<users>();
            odb::transaction t(db->begin());
            db->load(id, *user_data);
            t.commit();
            return user_data;
        } catch (const std::exception& e) {
            logger("UserController::getUserById", e.what());
            std::cerr << "Error fetching user by ID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    // Create a new user
    static bool createUser(std::shared_ptr<odb::mysql::database> &db, boost::shared_ptr<users> user_data) {
        try {
            odb::transaction t(db->begin());
            db->persist(user_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("UserController::createUser", e.what());
            std::cerr << "Error creating user: " << e.what() << std::endl;
            return false;
        }
    }

    // Get user by phone number
    static boost::shared_ptr<users> getUserByPhone(std::shared_ptr<odb::mysql::database> &db, const std::string &phone) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<users> user_data(db->query_one<users>(query::phone == phone));
            t.commit();
            return user_data;
        } catch (const std::exception& e) {
            logger("UserController::getUserByPhone", e.what());
            std::cerr << "Error fetching user by phone: " << e.what() << std::endl;
            return nullptr;
        }
    }

    // Update a user's information
    static bool updateUser(std::shared_ptr<odb::mysql::database> &db, const boost::json::value &values) {
        try {
            const boost::json::object& jsonBody = values.as_object();

            std::string phone = safe_get_value<std::string>(jsonBody, "phone", "Phone Number");
            std::string password = safe_get_value<std::string>(jsonBody, "password", "Password");
            std::string company = safe_get_value<std::string>(jsonBody, "company", "Company");
            std::string reg_date = safe_get_value<std::string>(jsonBody, "reg_date", "Registration Date");

            auto user_data = getUserByPhone(db, phone);
            if (!user_data) {
                return false; // User not found
            }

            user_data->set_password(password);
            user_data->set_company(company);
            user_data->set_reg_date(reg_date);

            odb::transaction t(db->begin());
            db->update(user_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("UserController::updateUser", e.what());
            std::cerr << "Error updating user: " << e.what() << std::endl;
            return false;
        }
    }

    // Delete a user by phone
    static bool deleteUser(std::shared_ptr<odb::mysql::database> &db, const std::string &phone) {
        try {
            auto user_data = getUserByPhone(db, phone);
            if (!user_data) {
                std::cout << "Could not find user" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(user_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("deleteUser::deleteUser", e.what());
            std::cerr << "Error deleting user: " << e.what() << std::endl;
            return false;
        }
    }
};
