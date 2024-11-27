#pragma once
#include <string>
#include <odb/core.hxx>
#include <odb/query.hxx>
#include <boost/shared_ptr.hpp>
#include "model_macro.hxx"
#include "employee.hxx"

#pragma db object pointer(boost::shared_ptr)
class Task{
    friend class odb::access;
    #pragma db id auto  
    unsigned long task_id;

    std::string task_unique;
    #pragma db type("BIGINT UNSIGNED")
    boost::shared_ptr<employee> task_employee;

    std::string task_item;
    std::string task_amount;
    std::string task_start;
    std::string task_end;
    std::string task_reg_date;
public:
    Task(
        const std::string & unique,
        const boost::shared_ptr<employee>& employee_ptr,
        const std::string & item,
        const std::string & amount,
        const std::string & start,
        const std::string & end,
        const std::string & reg_date
    )
    :
    task_unique(unique), 
    task_employee(employee_ptr),
    task_item(item), 
    task_amount(amount), 
    task_start(start), 
    task_end(end), 
    task_reg_date(reg_date)
    {}

    Task(){} 

    DEFINE_GETTER_SETTER(long, task_id)
    DEFINE_GETTER_SETTER(std::string, task_unique)
    DEFINE_NON_CONST_GETTER_SETTER(boost::shared_ptr<employee>, task_employee)
    DEFINE_GETTER_SETTER(std::string, task_item)
    DEFINE_GETTER_SETTER(std::string, task_amount)
    DEFINE_GETTER_SETTER(std::string, task_start)
    DEFINE_GETTER_SETTER(std::string, task_end)
    DEFINE_GETTER_SETTER(std::string, task_reg_date)
};