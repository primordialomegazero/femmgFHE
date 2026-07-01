/*
 * FEmmg-FHE v22.0.0 — Input Validator
 * 
 * Validates all incoming requests to prevent:
 * - Buffer overflow
 * - JSON injection
 * - Oversized payloads
 * - Malformed ciphertexts
 */

#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace input_validator {

constexpr size_t MAX_BODY_SIZE = 65536;      // 64KB max
constexpr size_t MAX_CIPHERTEXT_SIZE = 1024; // 1KB per ciphertext
constexpr int MAX_CLIENT_ID_LENGTH = 128;
constexpr int MAX_ACTION_LENGTH = 32;

struct ValidationResult {
    bool valid = true;
    std::string error;
    std::string sanitized_body;
};

class InputValidator {
public:
    // ═══ VALIDATE REQUEST ═══
    static ValidationResult validate(const std::string& raw_body, size_t received_bytes) {
        ValidationResult result;
        
        // Check 1: Size limit
        if (received_bytes > MAX_BODY_SIZE) {
            result.valid = false;
            result.error = "Request too large";
            return result;
        }
        
        // Check 2: Empty body
        if (raw_body.empty()) {
            result.valid = false;
            result.error = "Empty request";
            return result;
        }
        
        // Check 3: Must be valid JSON-like (starts with {)
        std::string trimmed = raw_body;
        // Trim whitespace
        size_t start = trimmed.find_first_not_of(" \t\r\n");
        if (start == std::string::npos || trimmed[start] != '{') {
            result.valid = false;
            result.error = "Invalid JSON format";
            return result;
        }
        
        // Check 4: No null bytes (injection attempt)
        if (trimmed.find('\0') != std::string::npos) {
            result.valid = false;
            result.error = "Null byte injection detected";
            return result;
        }
        
        // Check 5: Sanitize — remove control characters
        result.sanitized_body = sanitize(trimmed);
        
        return result;
    }
    
    // ═══ SANITIZE STRING ═══
    static std::string sanitize(const std::string& input) {
        std::string result;
        result.reserve(input.size());
        
        for (char c : input) {
            // Allow printable ASCII + newline + tab
            if ((c >= 32 && c <= 126) || c == '\n' || c == '\t') {
                result += c;
            }
        }
        
        // Truncate to max size
        if (result.size() > MAX_BODY_SIZE) {
            result.resize(MAX_BODY_SIZE);
        }
        
        return result;
    }
    
    // ═══ VALIDATE CLIENT ID ═══
    static bool valid_client_id(const std::string& id) {
        if (id.empty() || id.size() > MAX_CLIENT_ID_LENGTH) return false;
        
        for (char c : id) {
            if (!((c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') ||
                  c == '_' || c == '-' || c == '.')) {
                return false;
            }
        }
        return true;
    }
    
    // ═══ VALIDATE CIPHERTEXT SIZE ═══
    static bool valid_ciphertext_size(size_t size) {
        return size > 0 && size <= MAX_CIPHERTEXT_SIZE;
    }
};
} // namespace input_validator
