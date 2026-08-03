#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>

const double PHI = 1.6180339887498948482;

int main() {
    uint64_t seed = 1000;
    
    std::cout << "=== PHI HASH DISTRIBUTION DEBUG ===\n\n";
    
    // Test hash distribution for similar paths
    char path[256];
    uint64_t prev = 0;
    int collisions = 0;
    
    for (int i = 0; i < 20; i++) {
        snprintf(path, sizeof(path), "/home/user/file_%d.txt", i);
        
        double h = PHI;
        for (const char* p = path; *p; p++) {
            h = h * PHI + (double)(unsigned char)*p + seed * 0.0001;
            h = h - std::floor(h);
        }
        uint64_t hash = (uint64_t)(h * 1e18);
        int fd = (int)(hash & 0x7FFFFFFF);
        
        std::cout << "Path: " << path << "\n";
        std::cout << "  Full hash: " << hash << "\n";
        std::cout << "  FD (31-bit): " << fd << "\n";
        if (i > 0 && hash == prev) {
            std::cout << "  ⚠️ COLLISION with previous!\n";
            collisions++;
        }
        prev = hash;
    }
    
    std::cout << "\nCollisions in 20 paths: " << collisions << "\n";
    std::cout << "\n=== PROBLEM: The fractional part (h) may not vary enough ===\n";
    
    // Check: does h vary for different paths?
    std::cout << "\nHash values for first 10 paths:\n";
    for (int i = 0; i < 10; i++) {
        snprintf(path, sizeof(path), "/home/user/file_%d.txt", i);
        double h = PHI;
        for (const char* p = path; *p; p++) {
            h = h * PHI + (double)(unsigned char)*p + seed * 0.0001;
            h = h - std::floor(h);
        }
        std::cout << "  " << path << ": h=" << std::fixed << std::setprecision(15) << h << "\n";
    }
    
    return 0;
}
