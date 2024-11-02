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
#include "../models/employee.hxx" 
#include "../models/employee-odb.hxx"
#include <boost/json.hpp>
#include <boost/shared_ptr.hpp>
#include "core/safe_json.hxx"
#include "core/helpers.hxx"

class EmployeeController {
public:
    typedef odb::query<employee> query;
    typedef odb::result<employee> result;

    static auto getAllEmployees(const std::shared_ptr<odb::mysql::database>  & db) {
        std::vector<boost::shared_ptr<employee>> employees;
        try {
            odb::transaction t(db->begin());
            odb::result<employee> r(db->query<employee>());
            for (auto i = r.begin(); i != r.end(); ++i) {
                employees.emplace_back(boost::make_shared<employee>(*i));
            }
            t.commit();
            return employees;

        } catch (const std::exception& e) {
            std::cerr << "Error fetching employees: " << e.what() << std::endl;
            return employees;
        }
    }

    static bool createEmployee(std::shared_ptr<odb::mysql::database> & db, boost::shared_ptr<employee> employee_d){
        try{
            odb::transaction t(db->begin());
            db->persist(employee_d);
            t.commit();
            return true;
        }catch (const std::exception& e) {
            logger("EmployeeController::createEmployee", e.what());
            std::cerr << "Error creating employee: " << e.what() << std::endl;
            return false;
        }
    }

    static boost::shared_ptr<employee> getEmployeeByUiid(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid){
        try{
            odb::transaction t(db->begin());
            boost::shared_ptr<employee> employee_data (
                db->query_one<employee> ( query::employee_unique == uuid));
            t.commit();
            return employee_data;

        }catch (const std::exception& e) {
            logger("EmployeeController::getEmployeeByUiid", e.what());
            std::cerr << "Error fetching employee by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static boost::shared_ptr<employee> getEmployeeByContact(std::shared_ptr<odb::mysql::database> & db, const std::string & contact){
        try{
            odb::transaction t(db->begin());
            boost::shared_ptr<employee> employee_data (
                db->query_one<employee> ( query::employee_contact == contact));
            t.commit();
            return employee_data;

        }catch (const std::exception& e) {
            logger("EmployeeController::getEmployeeByContact", e.what());
            std::cerr << "Error fetching employee by UUID: " << e.what() << std::endl;
            return nullptr;
        }
    }

    static auto updateEmployee(std::shared_ptr<odb::mysql::database> & db, const boost::json::value & values){
        try{
            const boost::json::object& jsonBody = values.as_object();

            std::string employee_uuid = safe_get_value<std::string>(jsonBody, "employee_unique", "Employee UUID");
            std::string employee_name = safe_get_value<std::string>(jsonBody, "employee_name", "Employee Name");
            std::string employee_gender = safe_get_value<std::string>(jsonBody, "employee_gender", "Employee Gender");
            std::string employee_contact = safe_get_value<std::string>(jsonBody, "employee_contact", "Employee Contact");
            std::string employee_address = safe_get_value<std::string>(jsonBody, "employee_address", "Employee Address");

            auto employee_d = getEmployeeByUiid(db, employee_uuid);
            if(!employee_d){
                return false;
            }

            employee_d->set_employee_name(employee_name);
            employee_d->set_employee_gender(employee_gender);
            employee_d->set_employee_contact(employee_contact);
            employee_d->set_employee_address(employee_address);

            odb::transaction t(db->begin());
            db->update(employee_d);
            t.commit();
            return true;

        }catch (const std::exception& e) {
            logger("EmployeeController::updateEmployee", e.what());
            std::cerr << "Error updating employee: " << e.what() << std::endl;
            return false;
        }
    }

    static auto deleteEmployee(std::shared_ptr<odb::mysql::database> & db, const std::string & uuid){
        try{
            auto employee_d = getEmployeeByUiid(db, uuid);
            if(!employee_d){  
                std::cout << "Could not find employee" << std::endl;
                return false;
            }

            odb::transaction t(db->begin());
            db->erase(employee_d);
            t.commit();
            return true;

        }catch (const std::exception& e) {
            logger("EmployeeController::deleteEmployee", e.what());
            std::cerr << "Error deleting employee: " << e.what() << std::endl;
            return false;
        }
    }
};
