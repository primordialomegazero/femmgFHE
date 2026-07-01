#include "../src/security/audit_log.h"
#include <iostream>

using namespace audit_log;

int main() {
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  AUDIT LOGGER TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    AuditLogger logger;
    int passed = 0, total = 5;
    
    // Test 1: Log events
    std::cout << "\n1. LOG EVENTS: ";
    logger.log(EventType::AUTH_SUCCESS, "dan123", "login", "192.168.1.1", true);
    logger.log(EventType::AUTH_FAILURE, "hacker", "login", "10.0.0.99", false);
    logger.log(EventType::RATE_LIMIT, "spammer", "fhe_add", "203.0.113.5", false);
    logger.log(EventType::REQUEST, "dan123", "fhe_encrypt", "192.168.1.1", true);
    std::cout << logger.count() << " events logged ✅" << std::endl;
    if (logger.count() == 4) passed++;
    
    // Test 2: Client ID is hashed
    std::cout << "2. PRIVACY (hashed IDs): ";
    auto events = logger.get_recent(1);
    if (!events.empty()) {
        bool hashed = events[0].client_id != "dan123";  // Should be different!
        std::cout << (hashed ? "✅" : "❌") << " (" << events[0].client_id << ")" << std::endl;
        if (hashed) passed++;
    }
    
    // Test 3: Report generation
    std::cout << "3. REPORT: ";
    std::string report = logger.report();
    bool has_auth = report.find("AUTH_SUCCESS") != std::string::npos;
    bool has_fail = report.find("AUTH_FAILURE") != std::string::npos;
    std::cout << ((has_auth && has_fail) ? "✅" : "❌") << std::endl;
    if (has_auth && has_fail) passed++;
    
    // Test 4: JSON export
    std::cout << "4. JSON EXPORT: ";
    std::string json = logger.export_json();
    std::cout << (json.find("[{") == 0 ? "✅" : "❌") << " (" << json.size() << " bytes)" << std::endl;
    if (json.find("[{") == 0) passed++;
    
    // Test 5: Disabled mode
    std::cout << "5. DISABLED: ";
    logger.disable();
    logger.log(EventType::ERROR, "test", "test", "0.0.0.0", false);
    std::cout << (logger.count() == 4 ? "✅ (no new events)" : "❌") << std::endl;
    if (logger.count() == 4) passed++;
    
    std::cout << "\n═══════════════════════════════════════" << std::endl;
    std::cout << "  " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    return (passed == total) ? 0 : 1;
}
