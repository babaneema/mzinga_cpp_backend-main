// // database_manager.hpp
// #pragma once
// #include <map>
// #include <memory>
// #include <string>
// #include <mutex>
// #include <stdexcept>
// #include <odb/mysql/database.hxx>

// namespace database {

// class DatabaseManager {
// private:
//     static DatabaseManager* instance_;
//     static std::mutex mutex_;
    
//     std::map<std::string, std::shared_ptr<odb::mysql::database>> db_connections_;
//     std::mutex connections_mutex_;

//     // Private constructor for singleton
//     DatabaseManager() = default;

//     // Connection helper
//     std::shared_ptr<odb::mysql::database> connect_db(
//         const std::string& username, 
//         const std::string& password, 
//         const std::string& database_name);

// public:
//     DatabaseManager(const DatabaseManager&) = delete;
//     DatabaseManager& operator=(const DatabaseManager&) = delete;

//     static DatabaseManager& getInstance() {
//         std::lock_guard<std::mutex> lock(mutex_);
//         if (instance_ == nullptr) {
//             instance_ = new DatabaseManager();
//         }
//         return *instance_;
//     }

//     void load_connections();
    
//     std::shared_ptr<odb::mysql::database> get_connection_by_company(
//         const std::string& name);

//     // Add method to check connection health
//     bool verify_connection(const std::string& company_name);

//     // Cleanup method
//     void cleanup() {
//         std::lock_guard<std::mutex> lock(connections_mutex_);
//         db_connections_.clear();
//     }

//     ~DatabaseManager() {
//         cleanup();
//     }
// };

// // Implementation
// class DatabaseConnectionError : public std::runtime_error {
// public:
//     explicit DatabaseConnectionError(const std::string& message) 
//         : std::runtime_error(message) {}
// };

// inline std::shared_ptr<odb::mysql::database> DatabaseManager::connect_db(
//     const std::string& username, 
//     const std::string& password, 
//     const std::string& database_name) {
//     try {
//         return std::make_shared<odb::mysql::database>(
//             username, password, database_name);
//     } catch (const std::exception& e) {
//         throw DatabaseConnectionError(
//             "Failed to connect to database: " + std::string(e.what()));
//     }
// }

// inline void DatabaseManager::load_connections() {
//     std::lock_guard<std::mutex> lock(connections_mutex_);
    
//     try {
//         auto admin_db = connect_db("username", "password", "admin");
//         db_connections_["admin"] = admin_db;

//         auto db_companies = CompanyController::getAllCompanies(admin_db);
        
//         for (const auto& db_c : db_companies) {
//             std::string c_name = db_c->get_company();
//             std::string c_username = db_c->get_database_username();
//             std::string c_password = db_c->get_database_password();
//             std::string c_database = db_c->get_database_name();
            
//             db_connections_[c_name] = connect_db(
//                 c_username, c_password, c_database);
//         }
//     } catch (const std::exception& e) {
//         throw DatabaseConnectionError(
//             "Failed to load connections: " + std::string(e.what()));
//     }
// }

// inline std::shared_ptr<odb::mysql::database> DatabaseManager::get_connection_by_company(
//     const std::string& name) {
//     std::lock_guard<std::mutex> lock(connections_mutex_);
    
//     auto it = db_connections_.find(name);
//     if (it != db_connections_.end()) {
//         return it->second;
//     }
    
//     throw DatabaseConnectionError(
//         "Database connection not found for company: " + name);
// }

// inline bool DatabaseManager::verify_connection(const std::string& company_name) {
//     try {
//         auto db = get_connection_by_company(company_name);
//         // Perform a simple query to verify connection
//         db->query<int>("SELECT 1");
//         return true;
//     } catch (...) {
//         return false;
//     }
// }

// } // namespace database



#pragma once
#include <iostream>
#include <memory>
#include <map>
#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/database.hxx>
#include "controllers/companiesController.hxx"
#include "core/helpers.hxx"

using namespace std;

namespace database {
    using namespace odb::core;

    static std::map<std::string, std::shared_ptr<odb::mysql::database>> db_connections;

    // Function to connect to the database
    std::shared_ptr<odb::mysql::database> connect_db(const std::string& username, const std::string& password, const std::string& database_name) {
        std::shared_ptr<odb::mysql::database> db(new odb::mysql::database(username, password,database_name));
        return db;
    }

    // Function to load all database connections into the map
    void load_connections() {
        auto admin_db = connect_db("username","password", "admin");
        db_connections["admin"] = admin_db;
        auto db_companies =  CompanyController::getAllCompanies(admin_db);

        for (const auto& db_c : db_companies) {
            std::string c_name      = db_c->get_company();
            std::string c_username  = db_c->get_database_username();
            std::string c_passord   = db_c->get_database_password();
            std::string c_database  = db_c->get_database_name();
            db_connections[c_name] = connect_db(c_username,c_passord,c_database);
        }
    }

    std::shared_ptr<odb::mysql::database> get_connection_by_company(const std::string & name){
        auto it = db_connections.find(name);
        if (it != db_connections.end()) {
            return it->second;
        } else {
            std::string message = "Try to access database name which not available in the map "+name;
            logger("get_connection_by_company(const std::string & name)", message );
            return nullptr;
        }
    }

}


// #pragma once
// #include <iostream>
// #include <memory>
// #include <odb/database.hxx>
// #include <odb/transaction.hxx>
// #include <odb/pgsql/database.hxx>
// #include <odb/mysql/database.hxx> 

// using namespace std;

// namespace database {
//     using namespace odb::core;

//     // Local host  
//     std::shared_ptr<odb::mysql::database>  init_db() {
//         std::shared_ptr<odb::mysql::database> db(new odb::mysql::database("mzinga", "mzingamaji", "mzinga_system"));
//         return db;
//     } 

    

//     // Server settings This works because database and username share the same name
//     // std::shared_ptr<odb::mysql::database>  init_db() {
//     //     std::shared_ptr<odb::mysql::database> db(new odb::mysql::database(
//     //         "sql_mzingamaji_c",      // Database name
//     //         "NEs578aPybpeTe6P",      // User password
//     //         "sql_mzingamaji_c",     // Username
//     //         "45.32.22.61" // Replace with actual IP
//     //     ));
//     //     return db;
//     // }

// }