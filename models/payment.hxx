#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include "bill.hxx" 
#include "customer.hxx" 
#include "model_macro.hxx"

#pragma db object pointer(boost::shared_ptr)
class payments{
    friend class odb::access;
    #pragma db id auto  
    unsigned long pay_id;

    std::string pay_unique;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<bill> pay_bill;

    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<customer> pay_customer;

    std::string pay_method;
    std::string pay_type;
    std::string pay_amount;
    std::string pay_reg_date;

public:
    payments(){}
    payments(
        const std::string & unique,
        const boost::shared_ptr<bill> bill_ptr,
        const boost::shared_ptr<customer> customer_ptr,
        const std::string & method,
        const std::string & type,
        const std::string & amount,
        const std::string & reg_date
    ) :
        pay_unique(unique),
        pay_bill(bill_ptr),
        pay_customer(customer_ptr),
        pay_method(method),
        pay_type(type),
        pay_amount(amount),
        pay_reg_date(reg_date)
    {}

    DEFINE_GETTER_SETTER(unsigned long, pay_id)
    DEFINE_GETTER_SETTER(std::string, pay_unique)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<bill>, pay_bill)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<customer>, pay_customer)
    DEFINE_GETTER_SETTER(std::string, pay_method)
    DEFINE_GETTER_SETTER(std::string, pay_type)
    DEFINE_GETTER_SETTER(std::string, pay_amount)
    DEFINE_GETTER_SETTER(std::string, pay_reg_date)
};