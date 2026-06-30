#include "security_complete_fixed.h"
#include "pqc_simple.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  FEmmg-FHE SECURITY COMPLETE v20.1          ║\n";
    std::cout << "║  FIXED: Perturbation + PQC KEM              ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    bool all_pass = true;

    // Test 1: CSPRNG
    std::cout << "Test 1: CSPRNG\n";
    std::cout << "--------------\n";
    auto n1 = security::CSPRNG::generate_nonce();
    auto n2 = security::CSPRNG::generate_nonce();
    bool nonce_ok = (n1 != n2);
    std::cout << "  Nonces different: " << (nonce_ok ? "✅" : "❌") << "\n\n";
    if(!nonce_ok) all_pass = false;

    // Test 2: Perturbation
    std::cout << "Test 2: Perturbation Seed\n";
    std::cout << "--------------------------\n";
    security::PerturbationSeed s1, s2, s3;
    double p1 = s1.get_perturbation(0, 0, 0);
    double p2 = s2.get_perturbation(0, 0, 0);
    double p3 = s3.get_perturbation(0, 0, 0);

    bool pert_ok = (p1 != p2) && (p2 != p3) && (p1 != 0);
    std::cout << "  p1: " << std::fixed << std::setprecision(6) << p1 << "\n";
    std::cout << "  p2: " << p2 << "\n";
    std::cout << "  p3: " << p3 << "\n";
    std::cout << "  All different: " << (pert_ok ? "✅" : "❌") << "\n\n";
    if(!pert_ok) all_pass = false;

    // Test 3: PQC KEM
    std::cout << "Test 3: PQC KEM\n";
    std::cout << "---------------\n";
    pqc_simple::SimpleKEM kem;
    auto kp = kem.generate();
    auto [ct, secret] = kem.encapsulate(kp.public_key);
    auto recovered = kem.decapsulate(ct);

    bool pqc_ok = (secret == recovered);
    std::cout << "  Secret matches: " << (pqc_ok ? "✅" : "❌") << "\n\n";
    if(!pqc_ok) all_pass = false;

    // Summary
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  RESULTS                                    ║\n";
    std::cout << "╠══════════════════════════════════════════════╣\n";
    std::cout << "║  CSPRNG:       " << (nonce_ok ? "✅ PASS" : "❌ FAIL") << "                    ║\n";
    std::cout << "║  Perturbation: " << (pert_ok ? "✅ PASS" : "❌ FAIL") << "                    ║\n";
    std::cout << "║  PQC KEM:      " << (pqc_ok ? "✅ PASS" : "❌ FAIL") << "                    ║\n";
    std::cout << "╠══════════════════════════════════════════════╣\n";
    std::cout << "║  OVERALL:      " << (all_pass ? "✅ ALL PASSED" : "❌ HAS FAILURES") << "          ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";

    return all_pass ? 0 : 1;
}
