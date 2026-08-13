#include "src/golden_lwe/golden_enterprise_quantum.h"
#include <iostream>
#include <chrono>

int main() {
    GoldenEnterpriseQuantum::install_signal_handlers();
    std::cout << "=== ENTERPRISE QUANTUM FHE STRESS ===\n\n";

    GoldenEnterpriseQuantum::EnterpriseKeys ek;
    GoldenEnterpriseQuantum::keygen(ek, 42);

    auto enc = [&](bool b, uint64_t n){ return GoldenEnterpriseQuantum::enterprise_encrypt(ek, b, n); };
    auto dec = [&](const GoldenEnterpriseQuantum::QuantumCipher& qc){ return GoldenEnterpriseQuantum::enterprise_decrypt(qc, ek); };

    int pass = 0;
    for (int i=0; i<16; i++) {
        auto qc = enc(i % 2 == 1, i+1);
        if (dec(qc) == (i % 2 == 1)) pass++;
    }
    std::cout << "Encrypt/Decrypt: " << pass << "/16\n";

    std::vector<bool> bits(10000);
    for (int i = 0; i < 10000; i++) bits[i] = i % 2;
    auto t0 = std::chrono::high_resolution_clock::now();
    auto batch = GoldenEnterpriseQuantum::batch_enterprise_encrypt(ek, bits, 100000);
    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "Batch 10K: " << (10000 / secs) << " ops/s\n";

    std::cout << (pass == 16 ? "PASS ✅\n" : "FAIL ❌\n");
    return 0;
}
