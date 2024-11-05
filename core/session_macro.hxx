#pragma once
#define CHECK_SESSION_AND_GET_EMPLOYEE(req, res, employee_d)                                    \
    auto session_uuid = ServerSession::check_session_in_request(req);                           \
    if (!session_uuid) {                                                                        \
        res.result(http::status::bad_request);                                                  \
        res.set(http::field::content_type, "application/json");                                 \
        res.body() = R"({"auth": "false", "permission": "false", "error": "Bad Request."})";    \
        res.prepare_payload();                                                                  \
        return;                                                                                 \
    }                                                                                           \
    auto session = ServerSession::get_session_by_uuid(*session_uuid);                           \
    if (!session) {                                                                             \
        res.result(http::status::bad_request);                                                  \
        res.set(http::field::content_type, "application/json");                                 \
        res.body() = R"({"auth": "false", "permission": "false", "error": "Bad Request."})";    \
        res.prepare_payload();                                                                  \
        return;                                                                                 \
    }                                                                                           \
    auto handle = database::get_connection_by_company(session->company);                        \
    employee_d = EmployeeController::getEmployeeByContact(handle, session->phone);              \
    if (!employee_d) {                                                                          \
        res.result(http::status::bad_request);                                                  \
        res.set(http::field::content_type, "application/json");                                 \
        res.body() = R"({"auth": "true", "permission": "false", "error": "Bad Request."})";     \
        res.prepare_payload();                                                                  \
        return;                                                                                 \
    }

