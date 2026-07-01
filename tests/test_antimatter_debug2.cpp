#include "../src/security/anti_matter_v2.h"
#include <iostream>

int main() {
    antimatter::AntiMatterV2 am;
    
    std::cout << "Sending 100 rapid requests to 10.0.0.200..." << std::endl;
    int blocked = 0;
    for (int i = 0; i < 100; i++) {
        bool ok = am.allow("10.0.0.200");
        if (!ok) blocked++;
        if (i < 5 || i >= 95 || !ok) {
            std::cout << "  Request " << i << ": " << (ok ? "ALLOWED" : "BLOCKED") << std::endl;
        }
    }
    std::cout << "Blocked: " << blocked << "/100" << std::endl;
    std::cout << "Clients tracked: " << am.blocked_count() << std::endl;
    
    return 0;
}
