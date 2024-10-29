#pragma once
#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <string>
#include <iostream>
#include <limits>
#include <cmath>

// Helper function to safely extract a string from a JSON value
std::string safe_get_string(const boost::json::value& value, const std::string& default_value = "") {
    if (value.is_string()) {
        return value.as_string().c_str();
    }
    return default_value;
}

// Helper function to safely extract an integer from a JSON value
boost::optional<int> safe_get_int(const boost::json::value& value) {
    if (value.is_int64()) {
        int64_t int_value = value.as_int64();
        if (int_value >= std::numeric_limits<int>::min() && int_value <= std::numeric_limits<int>::max()) {
            return static_cast<int>(int_value);
        }
    } else if (value.is_uint64()) {
        uint64_t uint_value = value.as_uint64();
        if (uint_value <= static_cast<uint64_t>(std::numeric_limits<int>::max())) {
            return static_cast<int>(uint_value);
        }
    } else if (value.is_double()) {
        double double_value = value.as_double();
        if (std::floor(double_value) == double_value && 
            double_value >= static_cast<double>(std::numeric_limits<int>::min()) &&
            double_value <= static_cast<double>(std::numeric_limits<int>::max())) {
            return static_cast<int>(double_value);
        }
    } else if (value.is_string()) {
        try {
            return std::stoi(value.as_string().c_str());
        } catch (const std::exception&) {
            // Conversion failed, return empty optional
        }
    }
    return boost::none;
}

// Function to safely extract a value from a JSON object
template<typename T>
T safe_get_value(const boost::json::object& obj, const std::string& key, const T& default_value) {
    if (obj.if_contains(key)) {
        if constexpr (std::is_same<T, std::string>::value) {
            return safe_get_string(obj.at(key), default_value);
        } else if constexpr (std::is_same<T, int>::value) {
            boost::optional<int> result = safe_get_int(obj.at(key));
            return result ? *result : default_value;
        }
    }
    return default_value;
}

// Example usage function
void parse_employee_json(const boost::json::value& parsedValue) {
    if (!parsedValue.is_object()) {
        std::cerr << "Error: Expected a JSON object" << std::endl;
        return;
    }

    const boost::json::object& jsonBody = parsedValue.as_object();

    std::string fullName = safe_get_value<std::string>(jsonBody, "employee_name", "");
    std::string gender = safe_get_value<std::string>(jsonBody, "employee_gender", "");
    std::string contact = safe_get_value<std::string>(jsonBody, "employee_contact", "");
    std::string address = safe_get_value<std::string>(jsonBody, "employee_address", "");
    std::string password = safe_get_value<std::string>(jsonBody, "employee_password", "");
    std::string branch = safe_get_value<std::string>(jsonBody, "employee_branch", "");
    int age = safe_get_value<int>(jsonBody, "employee_age", -1);

    // Use the extracted values
    std::cout << "Employee Name: " << fullName << std::endl;
    std::cout << "Employee Age: " << (age != -1 ? std::to_string(age) : "Not provided") << std::endl;
    // ... other fields
}