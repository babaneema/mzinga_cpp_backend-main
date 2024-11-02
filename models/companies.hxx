#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class companies{

private:
    friend class odb::access;
    #pragma db id auto  
    unsigned long company_id;
    
    std::string company;
    std::string database_username;
    std::string database_password;
    std::string database_name;

public:
    companies() {}

    companies(
        const std::string& company,
        const std::string& username,
        const std::string& password,
        const std::string& dbname
    ): 
    company(company), 
    database_username(username), 
    database_password(password),
    database_name(dbname)

    {}

    DEFINE_GETTER_SETTER(long, company_id)
    DEFINE_GETTER_SETTER(std::string, company)
    DEFINE_GETTER_SETTER(std::string, database_username)
    DEFINE_GETTER_SETTER(std::string, database_password)
    DEFINE_GETTER_SETTER(std::string, database_name)
};