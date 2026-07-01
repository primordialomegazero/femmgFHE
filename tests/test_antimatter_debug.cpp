#include "../src/security/anti_matter_v2.h"
#include <iostream>

using namespace antimatter;

int main() {
    AntiMatterV2 am;
    
    std::cout << "Debug: Sending 50 rapid requests..." << std::endl;
    for (int i = 0; i < 50; i++) {
        bool allowed = am.allow("10.0.0.200");
        if (!allowed) {
            std::cout << "BLOCKED at request " << (i+1) << std::endl;
        }
    }
    std::cout << "Done. Check if any were blocked." << std::endl;
    
    // Check state
    std::cout << "Total tracked clients: " << am.blocked_count() << std::endl;
    
    return 0;
}
