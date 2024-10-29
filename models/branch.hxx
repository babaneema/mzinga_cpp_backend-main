#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/shared_ptr.hpp>
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class branch{

private:
    friend class odb::access;
    #pragma db id auto  
    unsigned long branch_id;
    
    std::string branch_unique;
    std::string branch_name;
    std::string branch_reg_date;

public:
    branch() {}

    branch(
        const std::string& unique,
        const std::string& name,
        const std::string& reg_date
    ): 
    branch_unique(unique), 
    branch_name(name), 
    branch_reg_date(reg_date)

    {}

    DEFINE_GETTER_SETTER(long, branch_id)
    DEFINE_GETTER_SETTER(std::string, branch_unique)
    DEFINE_GETTER_SETTER(std::string, branch_name)
    DEFINE_GETTER_SETTER(std::string, branch_reg_date)

};