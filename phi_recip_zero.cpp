// ============================================
// φ-BASED RECIPROCAL — ZERO LEVEL
// 1/x para sa Lucas numbers via φ-powers
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-BASED RECIPROCAL — ZERO LEVEL\n";
    cout << "  1/x para sa Lucas numbers\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 1;
    double PHI = 1.6180339887498948482;

    // Lucas at Fibonacci tables
    vector<long long> L = {2, 1};
    vector<long long> F = {0, 1};
    for (int i = 2; i <= 40; i++) {
        L.push_back(L[i-1] + L[i-2]);
        F.push_back(F[i-1] + F[i-2]);
    }

    cout << "========================================\n";
    cout << "  LUCAS RECIPROCALS\n";
    cout << "========================================\n\n";

    cout << "Key identities:\n";
    cout << "  1/φ = φ - 1\n";
    cout << "  1/φ² = 2 - φ\n";
    cout << "  1/φ³ = 2φ - 3\n\n";

    cout << "LUCAS RECIPROCAL TABLE:\n";
    cout << "n | L_n | 1/L_n | φ-form | Exact?\n";
    cout << "--|-----|-------|--------|-------\n";

    for (int n = 0; n <= 10; n++) {
        double reciprocal = 1.0 / L[n];
        
        // Find φ-form: a + bφ
        // For Lucas numbers: L_n = φ^n + (-1/φ)^n
        // 1/L_n ay may specific form
        
        cout << setw(2) << n << " | "
             << setw(4) << L[n] << " | "
             << setw(10) << fixed << setprecision(4) << reciprocal << " | ";
        
        // Try to express as a + bφ
        // For small Lucas numbers, may exact form
        if (n == 0) { // L_0 = 2
            cout << "1/2" << " | " << "✅";
        } else if (n == 1) { // L_1 = 1
            cout << "1" << " | " << "✅";
        } else if (n == 2) { // L_2 = 3
            cout << "≈ 0.382φ - 0.118" << " | " << "≈";
        } else {
            cout << "φ^-" << n << " form" << " | " << "≈";
        }
        cout << "\n";
    }

    cout << "\n========================================\n";
    cout << "  ZERO-LEVEL RECIPROCAL TEST\n";
    cout << "========================================\n\n";

    // Test: 1/L_4 = 1/7
    // L_4 = 7, L_4² = L_8 + 2 = 49
    // 1/7 ay hindi exact sa φ-form, pero may approximation

    cout << "TEST: 1/7 gamit ang φ-powers\n";
    cout << "L_4 = 7\n\n";

    // 1/7 ≈ φ^-3 = 0.2361
    // 7 × 0.2361 = 1.6527 ≠ 1
    // Kailangan ng better approximation

    // Try: 1/7 = (φ² - φ)/5
    // φ² = 2.618, φ = 1.618
    // (2.618 - 1.618)/5 = 1/5 = 0.2
    // 7 × 0.2 = 1.4 ≠ 1

    // Better: 1/7 = (3φ - 4)/5
    // 3φ = 4.854, 3φ - 4 = 0.854
    // 0.854/5 = 0.1708
    // 7 × 0.1708 = 1.196 ≠ 1

    // Exact: 1/7 ay walang simpleng φ-form
    // Pero may continued fraction approximation

    cout << "1/7 = 0.142857...\n";
    cout << "φ-form approximation: 1/7 ≈ (5 - 3φ)/3\n";
    cout << "(5 - 3φ)/3 = (5 - 4.854)/3 = 0.146/3 = 0.0487\n";
    cout << "7 × 0.0487 = 0.341 ≠ 1\n\n";

    cout << "========================================\n";
    cout << "  BETTER APPROACH: NEWTON + ZERO-LEVEL\n";
    cout << "========================================\n\n";

    cout << "Hybrid approach:\n";
    cout << "1. Newton's method para sa 1/x\n";
    cout << "2. Pero bawat Newton step ay zero-level\n";
    cout << "3. Kung kaya nating gawin ang Newton\n";
    cout << "   na zero-level, walang level consumption\n\n";

    cout << "CHALLENGE:\n";
    cout << "Ang Newton's method ay nangangailangan ng\n";
    cout << "EvalMult (ct_a × ct_x) — 1 level per step.\n";
    cout << "Para sa zero-level, kailangan ng\n";
    cout << "alternative multiplication.\n\n";

    cout << "========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n";
    cout << "  Ang reciprocal ng arbitrary value ay\n";
    cout << "  HINDI zero-level sa CKKS.\n";
    cout << "  Pero ang reciprocal ng φ-powers ay\n";
    cout << "  zero-level (addition lang).\n";
    cout << "  Kung ang computation ay nasa φ-domain,\n";
    cout << "  ang reciprocals ay libre.\n";
    cout << "========================================\n";

    return 0;
}
