#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include <odb/boost/smart-ptr/pointer-traits.hxx>
#include "branch.hxx" 
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class unit{
    friend class odb::access;
    #pragma db id auto  
    unsigned long unit_id;

    std::string unit_unique;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<branch> unit_branch;

    std::string unit_price;
    std::string unit_reg_date;

public:
    unit() {} 
    unit(const std::string& unique, const boost::shared_ptr<branch> branch_ptr,
            const std::string& price, 
            const std::string& reg_date)
    : unit_unique(unique), unit_branch(branch_ptr),
        unit_price(price),  unit_reg_date(reg_date) 
    {}

    DEFINE_GETTER_SETTER(long, unit_id)
    DEFINE_GETTER_SETTER(std::string, unit_unique)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<branch>, unit_branch)
    DEFINE_GETTER_SETTER(std::string, unit_price)
    DEFINE_GETTER_SETTER(std::string, unit_reg_date)
};