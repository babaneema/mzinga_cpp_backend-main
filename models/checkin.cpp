#include "checkin.hxx"

checkin::checkin(
    const std::string&  unique,
    const int&          employee_id,
    const std::string&  time_in,
    const std::string&  time_out,
    const std::string&  reg_date
)
    : checkin_unique(unique),
      checkin_employee(checkin_employee),
      checkin_in(time_in),
      checkin_out(time_out),
      checkin_reg_date(reg_date) {}

// Getters implementation
const unsigned long checkin::id() const {
    return checkin_id;
}

const std::string& checkin::unique() const {
    return checkin_unique;
}

const int& checkin::employee_id() const {
    return checkin_employee;
}

const std::string& checkin::time_in() const {
    return checkin_in;
}

const std::string& checkin::time_out() const {
    return checkin_out;
}

const std::string& checkin::reg_date() const {
    return checkin_reg_date;
}