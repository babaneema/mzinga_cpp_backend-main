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
#include "../models/customer.hxx" 
#include "../models/customer-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"

class CustomerController{
    typedef odb::query<customer> query;
    typedef odb::result<customer> result;

public:
    static boost::shared_ptr<customer> getCustomerById(std::shared_ptr<odb::mysql::database> & db, const int & id){
        try{
            boost::shared_ptr<customer> customer_data = boost::make_shared<customer>();
            odb::transaction t(db->begin());
            db->load(id,*customer_data);
            t.commit();
            return customer_data;
        }catch (const std::exception& e) {
            std::cerr << "Error here: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static boost::shared_ptr<customer> getCustomerByUiid(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid){
        try{
            odb::transaction t(db->begin());
            boost::shared_ptr<customer> customer_data (
                db->query_one<customer> ( query::customer_unique == uuid));
            t.commit();
            return customer_data;

        }catch (const std::exception& e) {
            std::cerr << "Error here: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static bool createCustomer(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<customer> customer_d){
        try{
            odb::transaction t(db->begin());
            db->persist(customer_d);
            t.commit();
            return true;
        }catch (const std::exception& e) {
            std::cerr << "Error : " << e.what() << std::endl;
            return false;
        }
    }

    static auto getAllCustomers(const std::shared_ptr<odb::mysql::database>  & db) {
        std::vector<boost::shared_ptr<customer>> customers;
        try {
            odb::transaction t(db->begin());
            odb::result<customer> r(db->query<customer>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                customers.emplace_back(boost::make_shared<customer>(*i));
            }
            t.commit();
            return customers;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching customers: " << e.what() << std::endl;
            return customers;
        }
    }

    static auto updateCustomer(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values){
        try{
            const boost::json::object& jsonBody = values.as_object();

            std::string customer_uuid = safe_get_value<std::string>(jsonBody, "customer_unique", "Customer UUID");
            std::string customer_branch = safe_get_value<std::string>(jsonBody, "customer_branch", "Customer Name");
            std::string customer_name = safe_get_value<std::string>(jsonBody, "customer_name", "Customer Name");
            std::string customer_gender = safe_get_value<std::string>(jsonBody, "customer_gender", "Customer Gender");
            std::string customer_contact = safe_get_value<std::string>(jsonBody, "customer_contact", "Customer Contact");
            std::string customer_address = safe_get_value<std::string>(jsonBody, "customer_address", "Customer Address");
            std::string customer_house_number = safe_get_value<std::string>(jsonBody, "customer_house_number", "Customer House Number");

            auto customer_d = getCustomerByUiid(db, customer_uuid);
            if(!customer_d){
                return false;
            }

            auto branch_d = BranchController::getBranchByUiid(db, customer_branch);
            if(!branch_d){
                return false;
            }

            customer_d->set_customer_name(customer_name);
            customer_d->set_customer_branch(branch_d);
            customer_d->set_customer_gender(customer_gender);
            customer_d->set_customer_contact(customer_contact);
            customer_d->set_customer_address(customer_address);
            customer_d->set_customer_house_number(customer_house_number);

            odb::transaction t(db->begin());
            db->update(customer_d);
            t.commit();
            return true;

        }catch (const std::exception& e) {
            std::cerr << "Error updating customer: " << e.what() << std::endl;
            return false;
        }
    }

    static auto GetCustomerReport(
        std::shared_ptr<odb::mysql::database> & db, 
        const std::string & start_date,
        const std::string & end_date
    ){
        
        std::vector<boost::shared_ptr<customer>> customers;

        try {
            odb::transaction t(db->begin());
            result r(db->query<customer>(query::customer_reg_date >= start_date && query::customer_reg_date <= end_date));

            for (auto i = r.begin(); i != r.end(); ++i) {
                customers.emplace_back(boost::make_shared<customer>(*i));
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Could not Get customers by date: " << e.what() << std::endl;
        }

        return customers;

    }

    static auto deleteCustomer(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid){
        try{
            auto customer_d = getCustomerByUiid(db, uuid);
            if(!customer_d){  
                std::cout << "Could not find customer" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(customer_d);
            t.commit();
            return true;

        }catch (const std::exception& e) {
            std::cerr << "Error deleting customer: " << e.what() << std::endl;
            return false;
        }
    }

    // Function to search for a customer by name or any three letters of the name
    static std::vector<boost::shared_ptr<customer>> searchCustomerByName(
        std::shared_ptr<odb::mysql::database>& db, const std::string& name) {
        std::vector<boost::shared_ptr<customer>> customers;
        try {
            odb::transaction t(db->begin());

            // Query to find names with any three letters match
            std::string search_pattern = "%" + name + "%";
            odb::result<customer> r(db->query<customer>(query::customer_name.like(search_pattern)));

            for (auto i = r.begin(); i != r.end(); ++i) {
                customers.emplace_back(boost::make_shared<customer>(*i));
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error searching customer by name: " << e.what() << std::endl;
        }

        return customers;
    }

    // Function to search for a customer by phone number or any three digits of the phone number
    static std::vector<boost::shared_ptr<customer>> searchCustomerByPhone(
        std::shared_ptr<odb::mysql::database>& db, const std::string& phone) {
        std::vector<boost::shared_ptr<customer>> customers;
        try {
            odb::transaction t(db->begin());

            // Query to find phone numbers with any three digits match
            std::string search_pattern = "%" + phone + "%";
            odb::result<customer> r(db->query<customer>(query::customer_contact.like(search_pattern)));

            for (auto i = r.begin(); i != r.end(); ++i) {
                customers.emplace_back(boost::make_shared<customer>(*i));
            }

            t.commit();
        } catch (const std::exception& e) {
            std::cerr << "Error searching customer by phone: " << e.what() << std::endl;
        }

        return customers;
    }

};
