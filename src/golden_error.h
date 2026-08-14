#pragma once
#include <exception>
#include <string>
#include "golden_logger.h"

// ============================================
// GOLDEN ERROR HANDLING
// ============================================

enum class GoldenErrorCode {
    SUCCESS = 0,
    RING_NOT_INITIALIZED = 100,
    INVALID_INPUT = 101,
    INVALID_PARAMETER = 102,
    ENCRYPTION_FAILED = 200,
    DECRYPTION_FAILED = 201,
    NAND_FAILED = 202,
    BOOTSTRAP_FAILED = 203,
    OBFUSCATION_FAILED = 300,
    EVALUATION_FAILED = 301,
    CIRCUIT_FAILED = 302,
    QUANTUM_FAILED = 400,
    PRNG_FAILED = 500,
    LUCAS_FAILED = 501,
    NOISE_FAILED = 502,
    SHUTDOWN_INITIATED = 900,
    UNKNOWN_ERROR = 999
};

class GoldenException : public std::exception {
private:
    GoldenErrorCode code;
    std::string message;
    std::string full_message;
    
public:
    GoldenException(GoldenErrorCode error_code, const std::string& msg, 
                    const char* file = "", int line = 0)
        : code(error_code), message(msg) {
        std::stringstream ss;
        ss << "GoldenException [Code: " << static_cast<int>(code) << "]";
        if (file && file[0]) {
            ss << " [" << file << ":" << line << "]";
        }
        ss << " - " << msg;
        full_message = ss.str();
        
        GoldenLogger::error(full_message);
    }
    
    const char* what() const noexcept override {
        return full_message.c_str();
    }
    
    GoldenErrorCode get_code() const {
        return code;
    }
    
    std::string get_message() const {
        return message;
    }
};

// Error checking macros
#define GOLDEN_CHECK(condition, code, msg) \
    if (!(condition)) throw GoldenException(code, msg, __FILE__, __LINE__)

#define GOLDEN_CHECK_NOT_SHUTDOWN() \
    if (GoldenLogger::is_shutting_down()) \
        throw GoldenException(GoldenErrorCode::SHUTDOWN_INITIATED, \
                              "System is shutting down", __FILE__, __LINE__)

// Safe wrapper
#define GOLDEN_TRY_BEGIN() try {
#define GOLDEN_TRY_END() } catch (const GoldenException& e) { \
    LOG_ERROR(std::string("Caught: ") + e.what()); \
} catch (const std::exception& e) { \
    LOG_ERROR(std::string("Std exception: ") + e.what()); \
} catch (...) { \
    LOG_ERROR("Unknown exception"); \
}
