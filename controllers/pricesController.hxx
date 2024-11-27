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
#include "../models/prices.hxx" 
#include "../models/prices-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"
#include "core/helpers.hxx"

class PriceController {
public:
    typedef odb::query<Price> query;
    typedef odb::result<Price> result;

    static auto getAllPrices(const std::shared_ptr<odb::mysql::database> & db) {
        std::vector<boost::shared_ptr<Price>> prices;
        try {
            odb::transaction t(db->begin());
            result r(db->query<Price>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                prices.emplace_back(boost::make_shared<Price>(*i));
            }
            t.commit();
            return prices;

        } catch (const std::exception& e) {
            logger("PriceController::getAllPrices", e.what());
            std::cerr << "Error fetching prices: " << e.what() << std::endl;
            return prices;
        }
    }

    static boost::shared_ptr<Price> getPriceById(std::shared_ptr<odb::mysql::database> & db, const int& id) {
        try {
            boost::shared_ptr<Price> price_data = boost::make_shared<Price>();
            odb::transaction t(db->begin());
            db->load(id, *price_data);
            t.commit();
            return price_data;
        } catch (const std::exception& e) {
            logger("PriceController::getPriceById", e.what());
            std::cerr << "Error fetching price by ID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static bool createPrice(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<Price> price_data) {
        try {
            odb::transaction t(db->begin());
            db->persist(price_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("PriceController::createPrice", e.what());
            std::cerr << "Error creating price: " << e.what() << std::endl;
            return false;
        }
    }

    static boost::shared_ptr<Price> getPriceByUuid(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<Price> price_data(
                db->query_one<Price>(query::price_unique == uuid));
            t.commit();
            return price_data;
        } catch (const std::exception& e) {
            logger("PriceController::getPriceByUuid", e.what());
            std::cerr << "Error fetching price by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static bool updatePrice(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values) {
        try {
            const boost::json::object& jsonBody = values.as_object();

            std::string price_uuid = safe_get_value<std::string>(jsonBody, "price_unique", "Price UUID");
            std::string price_item = safe_get_value<std::string>(jsonBody, "price_item", "Price Item");
            std::string price_amount = safe_get_value<std::string>(jsonBody, "price_amount", "Price Amount");
            std::string price_reg_date = safe_get_value<std::string>(jsonBody, "price_reg_date", "Price Registration Date");

            auto price_data = getPriceByUuid(db, price_uuid);
            if (!price_data) {
                // Safeguard
                return false;
            }

            price_data->set_price_item(price_item);
            price_data->set_price_amount(price_amount);
            price_data->set_price_reg_date(price_reg_date);

            odb::transaction t(db->begin());
            db->update(price_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("PriceController::updatePrice", e.what());
            std::cerr << "Error updating price: " << e.what() << std::endl;
            return false;
        }
    }

    static bool deletePrice(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            auto price_data = getPriceByUuid(db, uuid);
            if (!price_data) {
                std::cout << "Could not find price" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(price_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            logger("PriceController::deletePrice", e.what());
            std::cerr << "Error deleting price: " << e.what() << std::endl;
            return false;
        }
    }
};
