#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include <odb/boost/smart-ptr/pointer-traits.hxx>
#include "branch.hxx" 
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class employee{
    friend class odb::access;
    #pragma db id auto  
    unsigned long employee_id;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<branch> employee_branch;

    std::string employee_unique;
    std::string employee_name;
    std::string employee_gender;
    std::string employee_contact ;
    std::string employee_address;
    std::string employee_password;
    std::string employee_reg_date;

public:
    employee(){}


    employee(
        const std::string& unique,
        const boost::shared_ptr<branch> branch_ptr,
        const std::string& name,
        const std::string& gender,
        const std::string& contact,
        const std::string& address,
        const std::string& password,
        const std::string& reg_date
    ) : employee_unique(unique),
        employee_branch(branch_ptr),
        employee_name(name),
        employee_gender(gender),
        employee_contact(contact),
        employee_address(address),
        employee_password(password),
        employee_reg_date(reg_date)
    {}

    DEFINE_GETTER_SETTER(unsigned long, employee_id)
    DEFINE_GETTER_SETTER(std::string, employee_unique)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<branch>, employee_branch)
    DEFINE_GETTER_SETTER(std::string, employee_name)
    DEFINE_GETTER_SETTER(std::string, employee_gender)
    DEFINE_GETTER_SETTER(std::string, employee_contact)
    DEFINE_GETTER_SETTER(std::string, employee_address)
    DEFINE_GETTER_SETTER(std::string, employee_password)
    DEFINE_GETTER_SETTER(std::string, employee_reg_date)
};