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
#include "../models/companies.hxx" 
#include "../models/companies-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "core/safe_json.hxx"


class CompanyController {
public:
    typedef odb::query<companies> query;
    typedef odb::result<companies> result;

    // Get all companies
    static auto getAllCompanies(const std::shared_ptr<odb::mysql::database> &db) {
        std::vector<boost::shared_ptr<companies>> companiesList;
        try {
            odb::transaction t(db->begin());
            odb::result<companies> r(db->query<companies>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                companiesList.emplace_back(boost::make_shared<companies>(*i));
            }
            t.commit();
            return companiesList;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching companies: " << e.what() << std::endl;
            return companiesList;
        }
    }

    // Get company by ID
    static boost::shared_ptr<companies> getCompanyById(std::shared_ptr<odb::mysql::database> &db, const unsigned long &id) {
        try {
            boost::shared_ptr<companies> company_data = boost::make_shared<companies>();
            odb::transaction t(db->begin());
            db->load(id, *company_data);
            t.commit();
            return company_data;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching company by ID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    // Create a new company
    static bool createCompany(std::shared_ptr<odb::mysql::database> &db, boost::shared_ptr<companies> company_data) {
        try {
            odb::transaction t(db->begin());
            db->persist(company_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error creating company: " << e.what() << std::endl;
            return false;
        }
    }

    // Get company by name
    static boost::shared_ptr<companies> getCompanyByName(std::shared_ptr<odb::mysql::database> &db, const std::string &companyName) {
        try {
            odb::transaction t(db->begin());
            boost::shared_ptr<companies> company_data(db->query_one<companies>(query::company == companyName));
            t.commit();
            return company_data;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching company by name: " << e.what() << std::endl;
            return nullptr;
        }
    }

    // Update a company's information
    static bool updateCompany(std::shared_ptr<odb::mysql::database> &db, const boost::json::value &values) {
        try {
            const boost::json::object& jsonBody = values.as_object();

            std::string companyName = safe_get_value<std::string>(jsonBody, "company", "Company Name");
            std::string dbUsername = safe_get_value<std::string>(jsonBody, "database_username", "Database Username");
            std::string dbPassword = safe_get_value<std::string>(jsonBody, "database_password", "Database Password");
            std::string dbName = safe_get_value<std::string>(jsonBody, "database_name", "Database Name");

            auto company_data = getCompanyByName(db, companyName);
            if (!company_data) {
                return false; // Company not found
            }

            company_data->set_database_username(dbUsername);
            company_data->set_database_password(dbPassword);
            company_data->set_database_name(dbName);

            odb::transaction t(db->begin());
            db->update(company_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error updating company: " << e.what() << std::endl;
            return false;
        }
    }

    // Delete a company by name
    static bool deleteCompany(std::shared_ptr<odb::mysql::database> &db, const std::string &companyName) {
        try {
            auto company_data = getCompanyByName(db, companyName);
            if (!company_data) {
                std::cout << "Could not find company" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(company_data);
            t.commit();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error deleting company: " << e.what() << std::endl;
            return false;
        }
    }
};
