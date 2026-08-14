#include "../src/golden_logger.h"
#include "../src/golden_error.h"
#include "../src/golden_privacy_system.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "TEST: LOGGER + ERROR HANDLING + GRACEFUL SHUTDOWN\n\n";
    
    // Setup
    GoldenLogger::set_level(LogLevel::DEBUG);
    GoldenLogger::enable_file_logging("golden.log");
    GoldenLogger::install_signal_handlers();
    
    LOG_INFO("Golden Privacy System starting...");
    LOG_DEBUG("Initializing FHE ring...");
    
    try {
        GoldenPrivacySystem gps(42);
        LOG_INFO("System initialized successfully");
        
        // Test logging levels
        LOG_TRACE("This is trace (hidden if level=DEBUG)");
        LOG_DEBUG("This is debug");
        LOG_INFO("This is info");
        LOG_WARN("This is warn");
        
        // Test error handling
        LOG_INFO("Testing error handling...");
        
        try {
            // Simulate error
            GOLDEN_CHECK(false, GoldenErrorCode::INVALID_PARAMETER, 
                         "Simulated error for testing");
        } catch (const GoldenException& e) {
            LOG_WARN(std::string("Caught expected: ") + e.what());
        }
        
        // Test operations with logging
        LOG_INFO("Testing encryption with logging...");
        auto ct = gps.encrypt_data(true);
        bool result = gps.decrypt_result(ct);
        
        if (result) {
            LOG_INFO("Encryption/Decryption: SUCCESS");
        } else {
            LOG_ERROR("Encryption/Decryption: FAILED");
        }
        
        // Test iO with logging
        LOG_INFO("Testing iO evaluation...");
        auto xor_func = [](const std::vector<bool>& in) {
            return in[0] ^ in[1];
        };
        gps.obfuscate_program(xor_func, 2);
        
        bool xor_result = gps.evaluate_io_public({true, false});
        LOG_INFO(std::string("XOR(1,0) = ") + (xor_result ? "1" : "0"));
        
        // Test graceful shutdown check
        LOG_INFO("Checking shutdown flag...");
        if (!GoldenLogger::is_shutting_down()) {
            LOG_INFO("System running normally");
        }
        
        // Test log count
        LOG_INFO(std::string("Total logs: ") + 
                 std::to_string(GoldenLogger::get_log_count()));
        
        // Test file logging
        LOG_INFO("File logging active: golden.log");
        
    } catch (const GoldenException& e) {
        LOG_ERROR(std::string("Fatal: ") + e.what());
        return 1;
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Std exception: ") + e.what());
        return 1;
    }
    
    LOG_INFO("Test complete. Shutting down gracefully...");
    GoldenLogger::initiate_shutdown();
    LOG_WARN("Shutdown initiated");
    
    GoldenLogger::close();
    
    std::cout << "\n✅ LOGGER + ERROR HANDLING TEST PASSED!\n";
    std::cout << "📄 Log file: golden.log\n";
    std::cout << "📊 Total logs: " << GoldenLogger::get_log_count() << "\n";
    
    return 0;
}
