#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include <odb/boost/smart-ptr/pointer-traits.hxx>
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class users{
    friend class odb::access;
    #pragma db id auto  
    unsigned long id;

    std::string company;
    std::string phone;
    std::string password;
    std::string reg_date;

public:
    users() {} 
    users(
        const std::string& company, 
        const std::string& phone, 
        const std::string& password, 
        const std::string& reg_date
    ): 
    company(company),
    phone(phone),
    password(password),  
    reg_date(reg_date) 
    {}

    DEFINE_GETTER_SETTER(long, id)
    DEFINE_GETTER_SETTER(std::string, company)
    DEFINE_GETTER_SETTER(std::string, phone)
    DEFINE_GETTER_SETTER(std::string, password)
    DEFINE_GETTER_SETTER(std::string, reg_date)
};