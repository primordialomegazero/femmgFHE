#include "security_complete_fixed.h"
#include <iostream>

int main() {
    std::cout << "Security Complete v20.1 - FIXED\n";
    std::cout << "================================\n\n";

    // Test 1: CSPRNG
    auto n1 = security::CSPRNG::generate_nonce();
    auto n2 = security::CSPRNG::generate_nonce();
    std::cout << "Nonce 1: " << n1.to_hex().substr(0, 16) << "...\n";
    std::cout << "Nonce 2: " << n2.to_hex().substr(0, 16) << "...\n";
    std::cout << "Different: " << (n1 != n2 ? "✅" : "❌") << "\n\n";

    // Test 2: PerturbationSeed
    security::PerturbationSeed s1;
    security::PerturbationSeed s2;
    double p1 = s1.get_perturbation(0, 0, 0);
    double p2 = s2.get_perturbation(0, 0, 0);
    std::cout << "Perturbation 1: " << p1 << "\n";
    std::cout << "Perturbation 2: " << p2 << "\n";
    std::cout << "Different: " << (p1 != p2 ? "✅" : "❌") << "\n";
    std::cout << "Non-zero: " << (p1 != 0 ? "✅" : "❌") << "\n\n";

    // Test 3: PQC KEM
    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();
    std::cout << "PQC Keypair generated\n";
    std::cout << "Fingerprint: " << kp.fingerprint.substr(0, 16) << "...\n\n";

    auto [ct, secret] = kem.encapsulate(kp.public_key);
    auto recovered = kem.decapsulate(ct);

    bool pqc_ok = (secret == recovered);
    std::cout << "Shared secret matches: " << (pqc_ok ? "✅" : "❌") << "\n";
    if(!pqc_ok) {
        std::cout << "  Secret: ";
        for(int i=0; i<8; i++) printf("%02x", secret[i]);
        std::cout << "...\n  Recovered: ";
        for(int i=0; i<8; i++) printf("%02x", recovered[i]);
        std::cout << "...\n";
    }

    std::cout << "\n" << (p1 != 0 && p1 != p2 && pqc_ok ? "✅ ALL PASSED" : "❌ SOME FAILED") << "\n";
    return 0;
}
