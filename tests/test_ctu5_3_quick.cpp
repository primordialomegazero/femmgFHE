#include "../src/core/banach_engine.h"
#include <iostream>
#include <chrono>

using namespace banach;

int main() {
    NDimBanachEngine engine;
    const int TOTAL = 100;
    int errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TOTAL; i++) {
        auto ct = engine.encrypt(i % 1000, 0);
        int64_t dec = engine.decrypt(ct);
        if (dec != (i % 1000)) errors++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double tps = TOTAL / (ms / 1000.0);
    
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  8 DEMON GATES — QUICK TEST" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  Ops:    " << TOTAL << std::endl;
    std::cout << "  Time:   " << ms << "ms" << std::endl;
    std::cout << "  TPS:    " << (long long)tps << std::endl;
    std::cout << "  Errors: " << errors << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (errors == 0) ? 0 : 1;
}
