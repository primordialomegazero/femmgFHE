#include "security_working.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  FEmmg-FHE SECURITY v20.1 - WORKING        ║\n";
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

    // Test 2: Perturbation (NOW WORKING!)
    std::cout << "Test 2: Perturbation Seed\n";
    std::cout << "--------------------------\n";
    security::PerturbationSeed s1, s2, s3;
    double p1 = s1.get_perturbation(0, 0, 0);
    double p2 = s2.get_perturbation(0, 0, 0);
    double p3 = s3.get_perturbation(0, 0, 0);

    std::cout << "  p1: " << std::fixed << std::setprecision(6) << p1 << "\n";
    std::cout << "  p2: " << p2 << "\n";
    std::cout << "  p3: " << p3 << "\n";

    bool pert_ok = (p1 != p2) && (p2 != p3) && (p1 != 0);
    std::cout << "  All different: " << (pert_ok ? "✅" : "❌") << "\n\n";
    if(!pert_ok) all_pass = false;

    // Check dimension variation
    double p00 = s1.get_perturbation(0, 0, 0);
    double p01 = s1.get_perturbation(0, 0, 1);
    double p10 = s1.get_perturbation(0, 1, 0);
    double p11 = s1.get_perturbation(1, 0, 0);

    std::cout << "  p(0,0,0): " << p00 << "\n";
    std::cout << "  p(0,0,1): " << p01 << "\n";
    std::cout << "  p(0,1,0): " << p10 << "\n";
    std::cout << "  p(1,0,0): " << p11 << "\n";

    bool dim_ok = (p00 != p01) && (p00 != p10) && (p00 != p11);
    std::cout << "  Dimensions vary: " << (dim_ok ? "✅" : "❌") << "\n\n";
    if(!dim_ok) all_pass = false;

    // Test 3: PQC KEM
    std::cout << "Test 3: PQC KEM\n";
    std::cout << "---------------\n";
    security::SimpleKEM kem;
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

    return 0;
}
