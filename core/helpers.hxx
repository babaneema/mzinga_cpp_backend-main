#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
#include <sstream>
#include <iomanip>



std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream dateStream;
    dateStream << std::put_time(std::localtime(&now_c), "%Y-%m-%d");
    return dateStream.str();
}

// Hashing with Boost's SHA-256
std::string hashPassword(const std::string& password) {
    try {
        // Generate a salt
        boost::uuids::random_generator gen;
        boost::uuids::uuid salt = gen();
        std::string salt_str = boost::uuids::to_string(salt);

        // Combine password with salt
        std::string salted_password = password + salt_str;

        // Hash the salted password using Boost.Hash
        boost::hash<std::string> string_hash;
        std::size_t hash_value = string_hash(salted_password);

        // Convert the hash to a hex string
        std::stringstream hash_stream;
        hash_stream << std::hex << hash_value;

        // Store the salt with the hash (you can separate them for better storage)
        return salt_str + ":" + hash_stream.str();
    } catch (const std::exception& e) {
        std::cerr << "Error hashing password: " << e.what() << std::endl;
        return "";
    }
}

// Verify password (rehash with the same salt and compare)
bool verifyPassword(const std::string& password, const std::string& hash) {
    try {
        // Split the hash and salt (assuming it's stored in the format "salt:hash")
        std::vector<std::string> parts;
        boost::split(parts, hash, boost::is_any_of(":"));
        if (parts.size() != 2) {
            std::cerr << "Invalid hash format." << std::endl;
            return false;
        }

        std::string salt = parts[0];
        std::string stored_hash = parts[1];

        // Rehash the provided password with the extracted salt
        std::string salted_password = password + salt;
        boost::hash<std::string> string_hash;
        std::size_t hash_value = string_hash(salted_password);

        // Convert the hash to hex and compare with the stored hash
        std::stringstream hash_stream;
        hash_stream << std::hex << hash_value;

        return hash_stream.str() == stored_hash;
    } catch (const std::exception& e) {
        std::cerr << "Error verifying password: " << e.what() << std::endl;
        return false;
    }
}


std::string formatedValue(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

std::string formatedValue(float value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}