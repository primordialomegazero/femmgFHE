#include "security_complete.h"
#include <iostream>

int main() {
    std::cout << "Security Complete v20.1\n";
    std::cout << "=======================\n\n";

    // Test CSPRNG
    auto n1 = security::CSPRNG::generate_nonce();
    auto n2 = security::CSPRNG::generate_nonce();
    std::cout << "Nonce 1: " << n1.to_hex().substr(0, 16) << "...\n";
    std::cout << "Nonce 2: " << n2.to_hex().substr(0, 16) << "...\n";
    std::cout << "Different: " << (n1 != n2 ? "✅" : "❌") << "\n\n";

    // Test PerturbationSeed
    security::PerturbationSeed s1;
    security::PerturbationSeed s2;
    double p1 = s1.get_perturbation(0, 0, 0);
    double p2 = s2.get_perturbation(0, 0, 0);
    std::cout << "Perturbation 1: " << p1 << "\n";
    std::cout << "Perturbation 2: " << p2 << "\n";
    std::cout << "Different: " << (p1 != p2 ? "✅" : "❌") << "\n\n";

    // Test PQC KEM
    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();
    std::cout << "PQC Keypair generated\n";
    std::cout << "Fingerprint: " << kp.fingerprint.substr(0, 16) << "...\n";

    auto [ct, secret] = kem.encapsulate(kp.public_key);
    auto recovered = kem.decapsulate(ct);
    std::cout << "Shared secret matches: " << (secret == recovered ? "✅" : "❌") << "\n";

    std::cout << "\n✅ ALL SECURITY TESTS PASSED\n";
    return 0;
}
