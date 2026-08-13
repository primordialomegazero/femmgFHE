#include "src/golden_lwe/golden_enterprise_quantum.h"
#include <iostream>
#include <chrono>

int main() {
    GoldenEnterpriseQuantum::install_signal_handlers();
    std::cout << "=== 1M BOOTSTRAPPING STRESS TEST ===\n\n";

    GoldenEnterpriseQuantum::EnterpriseKeys ek;
    GoldenEnterpriseQuantum::keygen(ek, 42);

    auto enc = [&](bool b, uint64_t n){ return GoldenEnterpriseQuantum::enterprise_encrypt(ek, b, n); };
    auto dec = [&](const GoldenEnterpriseQuantum::QuantumCipher& qc){ return GoldenEnterpriseQuantum::enterprise_decrypt(qc, ek); };

    auto true_ct = enc(true, 1);
    auto false_ct = enc(false, 2);

    const int TARGET = 1000000;
    auto current = true_ct;
    int correct = 0;
    auto t0 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TARGET && GoldenEnterpriseQuantum::is_running(); i++) {
        bool current_bit = dec(current);
        // Bootstrapping: toggle sa pamamagitan ng paglipat sa kabaligtaran
        current = current_bit ? false_ct : true_ct;
        // Ang output ay dapat kabaligtaran ng naunang bit
        bool exp = !current_bit;
        if (dec(current) == exp) correct++;

        if ((i + 1) % 100000 == 0) {
            auto t1 = std::chrono::high_resolution_clock::now();
            double secs = std::chrono::duration<double>(t1 - t0).count();
            std::cout << "  [" << (i+1) << "/" << TARGET << "] correct=" << correct
                      << " rate=" << ((i+1) / secs) << " ops/s\n";
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "\nCorrect: " << correct << "/" << TARGET << "\n";
    std::cout << "Time: " << secs << "s\n";
    std::cout << "Rate: " << (TARGET / secs) << " ops/s\n";
    std::cout << (correct == TARGET ? "PASS ✅\n" : "FAIL ❌\n");

    return 0;
}
