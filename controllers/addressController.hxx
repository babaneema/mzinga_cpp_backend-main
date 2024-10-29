#pragma once
#include <boost/uuid/uuid.hpp>
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
#include "../models/address.hxx" 
#include "../models/address-odb.hxx"
#include <boost/json.hpp>

class AddressController {
public:
    typedef odb::query<address> query;
    typedef odb::result<address> result;
    typedef std::shared_ptr<odb::mysql::database> Db;

    static boost::json::object address_to_json(const std::shared_ptr<address> address) {
        if(address){
            return {
                {"address_id",          address->id()},
                {"address_unique",      address->unique()},
                {"address_name",        address->name()},
                {"address_reg_date",    address->reg_date()}
            };
        }
        return {};
    }

    static boost::json::array address_to_json(const std::vector<std::shared_ptr<address>> addresses) {
        boost::json::array json_array;
        for (const auto& address : addresses) {
            json_array.emplace_back(address_to_json(address));
        }
        return json_array;
    }

    static auto getAllAddress(const Db  & db) {
        std::vector<std::shared_ptr<address>> addresses;
        try {
            odb::transaction t(db->begin());
            odb::result<address> r(db->query<address>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                addresses.emplace_back(std::make_shared<address>(*i));
            }
            t.commit();
        }catch (const std::exception& e) {
            std::cerr << "Error creating branch: " << e.what() << std::endl;
           
        }
        return addresses; 
     
    }

    static bool createAddress(Db & db, const boost::json::object& address_data) {
        try {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string addressUnique = boost::uuids::to_string(uuid);

            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::ostringstream dateStream;
            dateStream << std::put_time(std::localtime(&now_c), "%Y-%m-%d");
            std::string addressRegDate = dateStream.str();

            std::string addressName = address_data.at("address_name").as_string().c_str();

            std::shared_ptr<address> newAddress = std::make_shared<address>(addressUnique, addressName, addressRegDate);

            odb::transaction t(db->begin());
            db->persist(*newAddress);
            t.commit();

            return true; 
        } catch (const std::exception& e) {
            std::cerr << "Error creating branch: " << e.what() << std::endl;
            return false; 
        }
    }
};