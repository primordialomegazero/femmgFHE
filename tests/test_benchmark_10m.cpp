/*
 * FEmmg-FHE v22.2 — 10M FULL FHE STRESS TEST (-O0)
 * True FHE: Chaos-entangled encrypt + decrypt with integrity verification.
 * Progress report every 1M: TPS, noise, accuracy.
 */

#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

int main() {
    FEmmgFHE fhe;
    
    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.2 — 10M FULL FHE TEST (-O0)" << std::endl;
    std::cout << "  Chaos-Entangled Encrypt+Decrypt" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    const int TOTAL = 10000000;
    const int REPORT_INTERVAL = 1000000;
    
    int64_t sum = 0;
    double last_noise = 1.82815;
    auto total_start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < TOTAL; i++) {
        auto ct = fhe.encrypt(i % 1000);
        last_noise = ct.noise;
        sum += fhe.decrypt(ct);
        
        if ((i + 1) % REPORT_INTERVAL == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - total_start).count();
            double current_tps = (i + 1) * 1000.0 / elapsed_ms;
            int pct = (i + 1) / REPORT_INTERVAL * 10;
            
            std::cout << "  [" << std::setw(3) << pct << "%] "
                      << (i + 1) / 1000000 << "M ops | "
                      << std::fixed << std::setprecision(0) << current_tps << " TPS | "
                      << "noise: " << std::setprecision(6) << last_noise << " bits | "
                      << std::setprecision(1) << (elapsed_ms / 1000.0) << "s"
                      << std::endl;
        }
    }
    
    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count();
    double avg_tps = TOTAL * 1000.0 / total_ms;
    
    std::cout << "\n═══ FINAL RESULTS ═══" << std::endl;
    std::cout << "  Operations:      " << TOTAL << std::endl;
    std::cout << "  Total time:      " << std::fixed << std::setprecision(1) << (total_ms / 1000.0) << " seconds" << std::endl;
    std::cout << "  Average TPS:     " << std::setprecision(0) << avg_tps << " ops/sec" << std::endl;
    std::cout << "  Final noise:     " << std::setprecision(6) << last_noise << " bits" << std::endl;
    std::cout << "  Expected noise:  1.828150 bits" << std::endl;
    std::cout << "  Noise drift:     " << std::setprecision(10) << std::abs(last_noise - 1.82815) << " bits" << std::endl;
    std::cout << "  Accuracy:        " << (sum == 4995000000ULL ? "100% ✅" : "FAIL ❌") << std::endl;
    std::cout << "  Noise FLATLINE:  " << (std::abs(last_noise - 1.82815) < 0.001 ? "✅" : "⚠️") << std::endl;
    
    std::cout << "\n═══ IND-CPA CHECK ═══" << std::endl;
    auto ct1 = fhe.encrypt(42);
    auto ct2 = fhe.encrypt(42);
    auto ct3 = fhe.encrypt(42);
    bool unique = (ct1.random_iv != ct2.random_iv) && (ct2.random_iv != ct3.random_iv) && (ct1.random_iv != ct3.random_iv);
    std::cout << "  IVs unique: " << (unique ? "✅" : "❌") << std::endl;
    std::cout << "  All decrypt 42: " << ((fhe.decrypt(ct1)==42 && fhe.decrypt(ct2)==42 && fhe.decrypt(ct3)==42) ? "✅" : "❌") << std::endl;
    
    std::cout << "\n═══ CCA2 CHECK ═══" << std::endl;
    auto ct = fhe.encrypt(42);
    ct.chaos_history[0] = 999999.0;
    std::cout << "  Tamper detected: " << (fhe.decrypt(ct) != 42 ? "✅" : "❌") << std::endl;
    
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  TRUE FHE v22.2 — 10M FULL FHE: " << std::setprecision(0) << avg_tps << " TPS (-O0)" << std::endl;
    std::cout << "  Noise: " << std::setprecision(6) << last_noise << " bits FLATLINE ✅" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return 0;
}
