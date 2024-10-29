#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include <odb/boost/smart-ptr/pointer-traits.hxx>
#include <odb/boost/optional/wrapper-traits.hxx>
#include "customer.hxx" 
#include "model_macro.hxx"

 enum class LockStatus {
    No,
    Yes
};

#pragma db object pointer(boost::shared_ptr)
class meter{ 
    friend class odb::access;
    #pragma db id auto  
    unsigned long meter_id;

    std::string meter_unique;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<customer> meter_customer;

    std::string meter_owner;
    std::string meter_number;
    std::string meter_intital_unit;
    std::string meter_joinging_price;

    #pragma db type("ENUM('Yes', 'No')") default("No") null
    std::string meter_lock;

    std::string meter_in_service;
    std::string meter_reg_date;

public:
    meter() {}

    meter(
        const std::string& unique,
        const boost::shared_ptr<customer>& customer_ptr,
        const std::string& owner,
        const std::string& number,
        const std::string& initial_unit,
        const std::string& joining_price,
        const  std::string& lock = "No",
        const std::string& in_service="No",
        const std::string& reg_date=""
    ) : meter_unique(unique),
        meter_customer(customer_ptr),
        meter_owner(owner),
        meter_number(number),
        meter_intital_unit(initial_unit),
        meter_joinging_price(joining_price),
        meter_lock(lock),
        meter_in_service(in_service),
        meter_reg_date(reg_date) 
    {}

    DEFINE_GETTER_SETTER(unsigned long, meter_id)
    DEFINE_GETTER_SETTER(std::string, meter_unique)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<customer>, meter_customer)
    DEFINE_GETTER_SETTER(std::string, meter_owner)
    DEFINE_GETTER_SETTER(std::string, meter_number)
    DEFINE_GETTER_SETTER(std::string, meter_intital_unit)
    DEFINE_GETTER_SETTER(std::string, meter_joinging_price)
    DEFINE_GETTER_SETTER(std::string, meter_lock)
    DEFINE_GETTER_SETTER(std::string, meter_in_service)
    DEFINE_GETTER_SETTER(std::string, meter_reg_date)

};