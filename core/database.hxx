#pragma once
#include <iostream>
#include <memory>
#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/pgsql/database.hxx>
#include <odb/mysql/database.hxx> 

using namespace std;

namespace database {
    using namespace odb::core;

    // Local host  
    std::shared_ptr<odb::mysql::database>  init_db() {
        std::shared_ptr<odb::mysql::database> db(new odb::mysql::database("mzinga", "mzingamaji", "mzinga_system"));
        return db;
    } 

    

    // Server settings This works because database and username share the same name
    // std::shared_ptr<odb::mysql::database>  init_db() {
    //     std::shared_ptr<odb::mysql::database> db(new odb::mysql::database(
    //         "sql_mzingamaji_c",      // Database name
    //         "NEs578aPybpeTe6P",      // User password
    //         "sql_mzingamaji_c",     // Username
    //         "45.32.22.61" // Replace with actual IP
    //     ));
    //     return db;
    // }

}