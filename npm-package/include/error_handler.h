/*
 * FEmmg-FHE v22.0.0 — Error Handler
 * 
 * Secure error handling:
 * - No stack traces in responses
 * - Generic error messages (no info leakage)
 * - HTTP status codes
 * - Graceful degradation
 */

#pragma once
#include <string>
#include <map>

namespace error_handler {

enum class ErrorCode {
    OK = 0,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    RATE_LIMITED = 429,
    INTERNAL_ERROR = 500,
    SERVICE_UNAVAILABLE = 503,
    
    // Custom FHE errors
    ENCRYPTION_FAILED = 1001,
    DECRYPTION_FAILED = 1002,
    INVALID_CIPHERTEXT = 1003,
    SESSION_EXPIRED = 1004,
    AUTH_FAILED = 1005,
    BURST_DETECTED = 1006,
};

struct ErrorResponse {
    int http_code;
    std::string error_code;
    std::string message;       // Generic! No internal details.
    bool retry_allowed;
};

class ErrorHandler {
public:
    // ═══ GET ERROR RESPONSE ═══
    static ErrorResponse get(ErrorCode code) {
        switch (code) {
            case ErrorCode::OK:
                return {200, "OK", "Success", true};
            
            case ErrorCode::BAD_REQUEST:
                return {400, "BAD_REQUEST", "Invalid request format", false};
            
            case ErrorCode::UNAUTHORIZED:
                return {401, "UNAUTHORIZED", "Authentication required", true};
            
            case ErrorCode::FORBIDDEN:
                return {403, "FORBIDDEN", "Access denied", false};
            
            case ErrorCode::NOT_FOUND:
                return {404, "NOT_FOUND", "Resource not found", false};
            
            case ErrorCode::RATE_LIMITED:
                return {429, "RATE_LIMITED", "Too many requests. Slow down.", true};
            
            case ErrorCode::INTERNAL_ERROR:
                return {500, "INTERNAL_ERROR", "An error occurred. Please try again.", true};
            
            case ErrorCode::SERVICE_UNAVAILABLE:
                return {503, "SERVICE_UNAVAILABLE", "Service temporarily unavailable", true};
            
            // FHE-specific errors (generic messages)
            case ErrorCode::ENCRYPTION_FAILED:
                return {500, "OPERATION_FAILED", "Operation could not be completed", true};
            
            case ErrorCode::DECRYPTION_FAILED:
                return {400, "OPERATION_FAILED", "Invalid ciphertext or key", false};
            
            case ErrorCode::INVALID_CIPHERTEXT:
                return {400, "INVALID_DATA", "Malformed ciphertext", false};
            
            case ErrorCode::SESSION_EXPIRED:
                return {401, "SESSION_EXPIRED", "Session expired. Please login again.", true};
            
            case ErrorCode::AUTH_FAILED:
                return {401, "AUTH_FAILED", "Invalid credentials", true};
            
            case ErrorCode::BURST_DETECTED:
                return {429, "RATE_LIMITED", "Request rate exceeded", true};
            
            default:
                return {500, "UNKNOWN", "An unexpected error occurred", true};
        }
    }
    
    // ═══ JSON RESPONSE ═══
    static std::string to_json(ErrorCode code) {
        auto err = get(code);
        return "{"
               "\"status\":\"error\","
               "\"http_code\":" + std::to_string(err.http_code) + ","
               "\"error_code\":\"" + err.error_code + "\","
               "\"message\":\"" + err.message + "\","
               "\"retry\":" + (err.retry_allowed ? "true" : "false") +
               "}";
    }
    
    // ═══ SUCCESS RESPONSE ═══
    static std::string success_json(const std::string& data) {
        return "{\"status\":\"ok\",\"data\":" + data + "}";
    }
};

} // namespace error_handler
