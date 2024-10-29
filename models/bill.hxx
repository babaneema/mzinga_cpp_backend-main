#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include "unit.hxx" 
#include "meter.hxx" 
#include "customer.hxx" 
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class bill{
    friend class odb::access;
    #pragma db id auto  
    unsigned long bill_id;

    std::string bill_unique;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<customer> bill_customer;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<meter> bill_meter;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<unit> bill_unit;

    std::string bill_unit_used;
    std::string bill_cost;
    std::string bill_reg_date;

public:
    bill(){}
    bill(
        const std::string & unique,
        const boost::shared_ptr<customer> customer_ptr,
        const boost::shared_ptr<meter> meter_ptr,
        const boost::shared_ptr<unit> unit_ptr,
        const std::string & unit_used,
        const std::string & cost,
        const std::string & reg_date
    ):
    bill_unique(unique),
    bill_customer(customer_ptr),
    bill_meter(meter_ptr),
    bill_unit(unit_ptr),
    bill_unit_used(unit_used),
    bill_cost(cost),
    bill_reg_date(reg_date)
    {}

    DEFINE_GETTER_SETTER(unsigned long, bill_id)
    DEFINE_GETTER_SETTER(std::string, bill_unique)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<customer>, bill_customer)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<meter>, bill_meter)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<unit>, bill_unit)
    DEFINE_GETTER_SETTER(std::string, bill_unit_used)
    DEFINE_GETTER_SETTER(std::string, bill_cost)
    DEFINE_GETTER_SETTER(std::string, bill_reg_date)
};