#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <boost/optional.hpp> 
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/beast/http.hpp>
#include "core/helpers.hxx"

namespace beast = boost::beast;
namespace http = beast::http;

class ServerSession {
public:
    struct Session {
        std::string phone;
        std::string company;
        std::string uuid;
        std::string time = getCurrentDate();
    };

private:
    static std::vector<Session> session_vector;

public:
    // Setter to add a new session and return UUID
    static std::string set_session(const std::string& phone, const std::string& company) {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        std::string uuid_str = boost::uuids::to_string(uuid);

        session_vector.push_back(Session{phone, company, uuid_str});
        return uuid_str;
    }

    // Getter to retrieve all sessions
    static const std::vector<Session>& get_sessions() {
        return session_vector;
    }

    // Function to check if a session with a specific UUID exists
    static bool session_exists(const std::string& uuid) {
        for (const auto& session : session_vector) {
            if (session.uuid == uuid) {
                return true;
            }
        }
        return false;
    }

    // Function to get a single session by UUID
    static boost::optional<Session> get_session_by_uuid(const std::string& uuid) {
        for (const auto& session : session_vector) {
            if (session.uuid == uuid) {
                return session;
            }
        }
        return boost::none; // Return std::nullopt if session not found
    }

    static boost::optional<std::string>  check_session_in_request(const http::request<http::string_body>& req) {
        auto session_cookie = req[http::field::cookie];
        if (session_cookie.empty() || session_cookie.find("uuid=") == std::string::npos) {
            return boost::none;
        }

        // Check if the cookie contains a UUID
        std::string uuid_token = "uuid=";
        auto pos = session_cookie.find(uuid_token);

        pos += uuid_token.size();
        auto end_pos = session_cookie.find(";", pos);
        std::string uuid = session_cookie.substr(pos, end_pos - pos);

        // Check if the UUID exists in the session vector
        if(session_exists(uuid)) return uuid;
        return boost::none;
    }

    static std::string set_session_in_response(http::response<http::string_body>& res, const std::string& phone, const std::string& company) {
        std::string uuid = set_session(phone,company);
        res.set(http::field::set_cookie, "uuid=" + uuid + "; HttpOnly; Path=/");
        return uuid;
    }

    static bool delete_session_by_uuid(const std::string& uuid) {
        auto it = std::remove_if(session_vector.begin(), session_vector.end(),
                                 [&uuid](const Session& session) {
                                     return session.uuid == uuid;
                                 });
        
        if (it != session_vector.end()) {
            session_vector.erase(it, session_vector.end());
            return true;  // Session deleted
        }
        return false;  // Session not found
    }
};

// Definition of the static session_vector
std::vector<ServerSession::Session> ServerSession::session_vector;

// int main() {
//     // Example usage
//     std::string uuid = ServerSession::set_session("123", "192.168.1.1", "CompanyA");
//     std::cout << "Session UUID: " << uuid << std::endl;

//     bool exists = ServerSession::session_exists(uuid);
//     std::cout << "Session exists: " << (exists ? "Yes" : "No") << std::endl;

//     auto session = ServerSession::get_session_by_uuid(uuid);
//     if (session) {
//         std::cout << "Session found: ID=" << session->id << ", IP=" << session->ip 
//                   << ", Company=" << session->company << ", UUID=" << session->uuid << std::endl;
//     } else {
//         std::cout << "Session not found" << std::endl;
//     }

//     return 0;
// }
