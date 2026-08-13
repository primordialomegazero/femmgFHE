#include "src/golden_lwe/golden_enterprise_quantum.h"
#include <iostream>

int main() {
    GoldenEnterpriseQuantum::EnterpriseKeys ek;
    GoldenEnterpriseQuantum::keygen(ek, 42);

    auto enc = [&](bool b, uint64_t n){ return GoldenEnterpriseQuantum::enterprise_encrypt(ek, b, n); };
    auto dec = [&](const GoldenEnterpriseQuantum::QuantumCipher& qc){ return GoldenEnterpriseQuantum::enterprise_decrypt(qc, ek); };

    auto true_ct = enc(true, 1);
    auto false_ct = enc(false, 2);

    std::cout << "dec(true_ct) = " << dec(true_ct) << " (expect 1)\n";
    std::cout << "dec(false_ct) = " << dec(false_ct) << " (expect 0)\n";

    // Test ang first 10 iterations
    auto current = true_ct;
    for (int i = 0; i < 10; i++) {
        bool current_bit = dec(current);
        current = current_bit ? false_ct : true_ct;
        bool exp = (i % 2 == 0);
        bool out = dec(current);
        std::cout << "i=" << i << " current_bit=" << current_bit
                  << " out=" << out << " exp=" << exp << "\n";
    }

    return 0;
}
