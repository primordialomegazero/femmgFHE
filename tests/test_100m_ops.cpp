#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>
#include <chrono>

int main() {
    FEmmgFHE fhe;
    uint64_t key = 0x9E3779B97F4A7C15ULL;
    
    const long long TOTAL = 100000000LL;
    auto ct = fhe.encrypt(2, key);
    int64_t expected = 2;
    long long errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (long long i = 1; i <= TOTAL; i++) {
        auto ct2 = fhe.encrypt(2, key);
        if (i % 2 == 1) { ct = fhe.add(ct, ct2); expected += 2; }
        else { ct = fhe.multiply(ct, ct2); expected *= 2; }
        
        if (i % 10000000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            if (fhe.decrypt(ct) != expected) errors++;
            std::cout << "[" << (i/1000000) << "M] TPS: " << (i/elapsed) << " Errors: " << errors << std::endl;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "\n✅ 100M ops in " << secs << "s — " << (TOTAL/secs) << " TPS — " << errors << " errors" << std::endl;
    return errors == 0 ? 0 : 1;
}
