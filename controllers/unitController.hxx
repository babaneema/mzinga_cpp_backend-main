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
#include "../models/unit.hxx" 
#include "../models/unit-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"
#include "controllers/branchController.hxx"
#include <algorithm> // for std::sort

class UnitController {
public:
    typedef odb::query<unit> query;
    typedef odb::result<unit> result;

    // Fetch all units
    static auto getAllUnits(const std::shared_ptr<odb::mysql::database>  & db) {
        std::vector<boost::shared_ptr<unit>> units;
        try {
            odb::transaction t(db->begin());
            odb::result<unit> r(db->query<unit>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                units.emplace_back(boost::make_shared<unit>(*i));
            }
            t.commit();
            return units;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching units: " << e.what() << std::endl;
            return units;
        }
    }

    // Fetch unit by UUID
    static boost::shared_ptr<unit> getUnitByUuid(const std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<unit> unit_data(
                db->query_one<unit>(query::unit_unique == uuid));
            t.commit();
            return unit_data;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching unit by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    // Create a new unit
    static bool createUnit(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<unit> unit_d) {
        try {
            odb::transaction t(db->begin());
            db->persist(unit_d);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error creating unit: " << e.what() << std::endl;
            return false;
        }
    }

    // Update unit
    static bool updateUnit(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values) {
        try {
            const boost::json::object& jsonBody = values.as_object();

            std::string unit_uuid = safe_get_value<std::string>(jsonBody, "unit_unique", "Unit UUID");
            std::string price = safe_get_value<std::string>(jsonBody, "unit_price", "0.0");

            auto unit_d = getUnitByUuid(db, unit_uuid);
            if (!unit_d) {
                return false;
            }

            std::string unit_branch = safe_get_value<std::string>(jsonBody, "unit_branch", "Branch");

            auto branch_d = BranchController::getBranchByUiid(db, unit_branch);
            if(!branch_d){ 
               return false;
            }

            unit_d->set_unit_price(price);
            unit_d->set_unit_branch(branch_d);

            odb::transaction t(db->begin());
            db->update(unit_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error updating unit: " << e.what() << std::endl;
            return false;
        }
    }

    // Delete unit by UUID
    static bool deleteUnit(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid) {
        try {
            auto unit_d = getUnitByUuid(db, uuid);
            if (!unit_d) {
                std::cout << "Unit not found" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(unit_d);
            t.commit();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Error deleting unit: " << e.what() << std::endl;
            return false;
        }
    }

    // Fetch the last unit added for a specific branch
    static boost::shared_ptr<unit> getLastUnitByBranch(const std::shared_ptr<odb::mysql::database>& db, 
                                                   const boost::shared_ptr<branch>& branch_ptr) {
        std::vector<boost::shared_ptr<unit>> units;
        try {
            odb::transaction t(db->begin());

            // Using proper ODB query syntax
            typedef odb::query<unit> query;
            typedef odb::result<unit> result;

            // Creating the query
            result r(db->query<unit>(query::unit_branch == branch_ptr->get_branch_id()));

            boost::shared_ptr<unit> unit_data;
            for (auto i = r.begin(); i != r.end(); ++i) {
                units.emplace_back(boost::make_shared<unit>(*i));
            }
            std::sort(units.begin(), units.end(), [](const boost::shared_ptr<unit>& a, const boost::shared_ptr<unit>& b) {
                return a->get_unit_id() > b->get_unit_id(); // Descending order
            });

            t.commit();
           return !units.empty() ? units.front() : nullptr;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching last unit by branch: " << e.what() << std::endl;
            return nullptr;
        }
    }


};
