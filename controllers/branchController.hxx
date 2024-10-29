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
#include "../models/branch.hxx" 
#include "../models/branch-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"

class BranchController {
public:
    typedef odb::query<branch> query;
    typedef odb::result<branch> result;

    static auto getAllBranches(const std::shared_ptr<odb::mysql::database>  & db) {
        std::vector<boost::shared_ptr<branch>> branches;
        try {
            odb::transaction t(db->begin());
            odb::result<branch> r(db->query<branch>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                branches.emplace_back(boost::make_shared<branch>(*i));
            }
            t.commit();
            return branches;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching branches: " << e.what() << std::endl;
            return branches;
        }
    }

    static boost::shared_ptr<branch> getBranchById(std::shared_ptr<odb::mysql::database>  & db, const int& id) {
        try {
            boost::shared_ptr<branch> branch_data = boost::make_shared<branch>();
            odb::transaction t(db->begin());
            db->load(id,*branch_data);
            t.commit();
            return branch_data;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching branch by UUID: " << e.what() << std::endl;
            return nullptr; 
        }
    }

    static bool createBranch(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<branch> branch_d){
        try{
            odb::transaction t(db->begin());
            db->persist(branch_d);
            t.commit();
            return true;
        }catch (const std::exception& e) {
            std::cerr << "Error : " << e.what() << std::endl;
            return false;
        }
    }

    static boost::shared_ptr<branch> getBranchByUiid(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid){
        try{
            odb::transaction t(db->begin());
            boost::shared_ptr<branch> branch_data (
                db->query_one<branch> ( query::branch_unique == uuid));
            t.commit();
            return branch_data;

        }catch (const std::exception& e) {
            std::cerr << "Error here: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static auto updateBranch(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values){
        try{
            const boost::json::object& jsonBody = values.as_object();

            std::string branch_uuid = safe_get_value<std::string>(jsonBody, "branch_unique", "Branch Uiid");
            std::string branch_name = safe_get_value<std::string>(jsonBody, "branch_name", "Branch Name");

            auto branch_d = getBranchByUiid(db, branch_uuid);
            if(!branch_d){
                // this should not happen. It is just safe guard
                return false;
            }

            branch_d->set_branch_name(branch_name);

            odb::transaction t(db->begin());
            db->update(branch_d);
            t.commit();
            return true;

        }catch (const std::exception& e) {
            std::cerr << "Error here: " << e.what() << std::endl;
            return false;
        }
    }

    static auto deleteBranch(std::shared_ptr<odb::mysql::database> & db, const std::string & uiid){
        try{
            auto branch_d = getBranchByUiid(db, uiid);
            if(!branch_d){  
                std::cout << "Could not find branch " <<endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase (branch_d);
            t.commit();
            return true;

        }catch (const std::exception& e) {
            std::cerr << "Error here: " << e.what() << std::endl;
            return false;
        }
    }
};
