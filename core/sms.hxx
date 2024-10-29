#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <boost/json.hpp>
#include <boost/optional.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include "core/safe_json.hxx"

namespace json = boost::json;

// Callback function for curl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch(std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}


std::string base64_encode(const std::string& input) {
    // Calculate the required size for the encoded output
    std::string output(boost::beast::detail::base64::encoded_size(input.size()), '\0');
    
    // Perform the encoding
    boost::beast::detail::base64::encode(&output[0], input.data(), input.size());
    
    return output;
}

void sendSingleSms(const std::string& number, const std::string& message) {
    const std::string api_key = "8c941787fcc3d9d4";
    const std::string secret_key = "MzgwOGQxMzVhZTk2YzIzMjYwYzM2NGU2NGI0M2VlYmE4ZDljYTRkOWIyYTNlZTE1ZmZkNzVjNzc2NDVhYTA0MQ==";

    json::object namba = {
        {"recipient_id", "1"},
        {"dest_addr", number}
    };

    json::object postData = {
        {"source_addr", "MZINGAMAJI"},
        {"encoding", 0},
        {"schedule_time", ""},
        {"message", message},
        {"recipients", json::array{namba}}
    };

    std::string url = "https://apisms.beem.africa/v1/send";

    CURL* curl = curl_easy_init();
    if(curl) {
        struct curl_slist* headers = NULL;
        std::string credentials = api_key + ":" + secret_key;
        std::string ecode_credentials = base64_encode(credentials);
        char* escaped_credentials = curl_easy_escape(curl, credentials.c_str(), 0);
        std::string auth = std::string("Authorization:Basic ") + ecode_credentials;
        // curl_free(escaped_credentials);
        
        headers = curl_slist_append(headers, auth.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string readBuffer;
        std::string postDataStr = json::serialize(postData);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postDataStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << readBuffer << std::endl;

            try {
                // Parsing JSON response and safely extracting values
                auto parsedResponse = json::parse(readBuffer);
                const json::object& jsonResponse = parsedResponse.as_object();

                // Safely accessing "status" using find() to avoid exceptions
                auto status_it = jsonResponse.find("status");
                if (status_it != jsonResponse.end() && status_it->value().is_string()) {
                    std::string status = status_it->value().as_string().c_str();
                    std::cout << "SMS send status: " << status << std::endl;
                } else {
                    std::cerr << "Status not found or is not a string." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing or accessing JSON: " << e.what() << std::endl;
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}


void sendManySms(const std::vector<std::string>& numbers, const std::string& message) {
    const std::string api_key = "8c941787fcc3d9d4";
    const std::string secret_key = "MzgwOGQxMzVhZTk2YzIzMjYwYzM2NGU2NGI0M2VlYmE4ZDljYTRkOWIyYTNlZTE1ZmZkNzVjNzc2NDVhYTA0MQ==";

    json::array recipients;
    for (const auto& number : numbers) {
        recipients.push_back(json::object{
            {"recipient_id", "1"},
            {"dest_addr", number}
        });
    }

    json::object postData = {
        {"source_addr", "MZINGAMAJI"},
        {"encoding", 0},
        {"schedule_time", ""},
        {"message", message},
        {"recipients", recipients}
    };

    std::string url = "https://apisms.beem.africa/v1/send";

    CURL* curl = curl_easy_init();
    if(curl) {
        struct curl_slist* headers = NULL;
        std::string credentials = api_key + ":" + secret_key;
        char* escaped_credentials = curl_easy_escape(curl, credentials.c_str(), 0);
        std::string auth = std::string("Authorization: Basic ") + escaped_credentials;
        curl_free(escaped_credentials);
        
        headers = curl_slist_append(headers, auth.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string readBuffer;
        std::string postDataStr = json::serialize(postData);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postDataStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << readBuffer << std::endl;

            // Parsing JSON response and safely extracting values
            auto parsedResponse = json::parse(readBuffer);
            const json::object& jsonResponse = parsedResponse.as_object();
            std::string status = safe_get_string(jsonResponse.at("status"), "unknown");
            std::cout << "Bulk SMS send status: " << status << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}