#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include <odb/boost/smart-ptr/pointer-traits.hxx>
#include "branch.hxx" 
#include "model_macro.hxx"


#pragma db object pointer(boost::shared_ptr)
class customer{
    friend class odb::access;
    #pragma db id auto  
    unsigned long customer_id;

    std::string customer_unique;

    // #pragma db column("branch_id") inverse(customer_branch)
    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<branch> customer_branch;

    std::string customer_name;
    std::string customer_gender;
    std::string customer_contact;
    std::string customer_address;

    #pragma db type("TEXT") null
    std::string customer_house_number;

    std::string customer_reg_date;

public:
    customer() {} 
    customer(const std::string& unique, const boost::shared_ptr<branch> branch_ptr,
            const std::string& name, const std::string& gender,
            const std::string& contact, const std::string& address,
            const std::string& house_number,
            const std::string& reg_date)
    : 
    customer_unique(unique), 
    customer_branch(branch_ptr),
    customer_name(name), 
    customer_gender(gender),
    customer_contact(contact), 
    customer_address(address),
    customer_house_number(house_number), 
    customer_reg_date(reg_date) 
    {}
    
    DEFINE_GETTER_SETTER(long, customer_id)
    DEFINE_GETTER_SETTER(std::string, customer_unique)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<branch>, customer_branch)
    DEFINE_GETTER_SETTER(std::string, customer_name)
    DEFINE_GETTER_SETTER(std::string, customer_gender)
    DEFINE_GETTER_SETTER(std::string, customer_contact)
    DEFINE_GETTER_SETTER(std::string, customer_address)
    DEFINE_GETTER_SETTER(std::string, customer_house_number)
    DEFINE_GETTER_SETTER(std::string, customer_reg_date)
};