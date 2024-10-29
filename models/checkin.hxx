#pragma once
#include <string>
#include <memory>  // For std::unique_ptr
#include <odb/core.hxx>
#include <odb/query.hxx>
#include "employee.hxx" // Include employees header

#pragma db object 
class checkin {
private:
    checkin() {}
    friend class odb::access;

    #pragma db id auto  
    unsigned long checkin_id;

    std::string checkin_unique;
    int         checkin_employee;
    std::string checkin_in;
    std::string checkin_out; 
    std::string checkin_reg_date;

public:
    checkin(
        const std::string& unique,
        const int& checkin_employee,
        const std::string& time_in,
        const std::string& time_out,
        const std::string& reg_date
    );

    const unsigned long id() const;
    const std::string&  unique() const;
    const int&          employee_id() const;
    const std::string&  time_in() const;
    const std::string&  time_out() const;  // Nullable getter
    const std::string&  reg_date() const;
};
