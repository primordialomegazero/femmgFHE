#include "../src/core/banach_engine.h"
#include <iostream>
#include <chrono>

using namespace banach;

int main() {
    NDimBanachEngine engine;
    const int TOTAL = 10000;
    int errors = 0;
    
    std::cout << "10K Encrypt+Decrypt..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TOTAL; i++) {
        auto ct = engine.encrypt(i % 1000, 0);
        if (engine.decrypt(ct) != (i % 1000)) errors++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  8 Demon Gates — 10K Quick Test" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "  Ops:    " << TOTAL << std::endl;
    std::cout << "  Time:   " << ms << "ms" << std::endl;
    std::cout << "  TPS:    " << (TOTAL * 1000 / (ms ? ms : 1)) << std::endl;
    std::cout << "  Errors: " << errors << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    
    return (errors == 0) ? 0 : 1;
}
