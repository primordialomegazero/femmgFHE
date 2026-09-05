// ============================================
// φ-META FUNCTIONS — Lahat ng Libre
// EvalAtIndex, EvalSum, EvalSquare
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    cc->EvalSumKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-META FUNCTIONS — Lahat ng Libre\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: EvalSquare na may φ-identity
    // ============================================
    cout << "  TEST 1: EvalSquare na may φ-identity\n\n";

    vector<double> phi_val(8, PHI);
    Plaintext pt_phi = cc->MakeCKKSPackedPlaintext(phi_val);
    auto ct_phi = cc->Encrypt(keyPair.publicKey, pt_phi);

    auto ct_sq = cc->EvalSquare(ct_phi);
    auto v_sq = decrypt_state(ct_sq);
    
    auto ct_add = cc->EvalAdd(ct_phi, 1.0);
    auto v_add = decrypt_state(ct_add);
    
    cout << "    EvalSquare(φ) = " << v_sq[0] << "\n";
    cout << "    EvalAdd(φ, 1) = " << v_add[0] << "\n";
    cout << "    Match: " << (abs(v_sq[0] - v_add[0]) < 0.001 ? "✅" : "❌") << "\n";
    cout << "    Level (Square): " << ct_sq->GetLevel() << "\n";
    cout << "    Level (Add): " << ct_add->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: EvalSum na may φ-powers
    // ============================================
    cout << "  TEST 2: EvalSum na may φ-powers\n\n";

    vector<double> phi_powers(8, 0.0);
    for (int i = 0; i < 8; i++) phi_powers[i] = pow(PHI, i);
    Plaintext pt_powers = cc->MakeCKKSPackedPlaintext(phi_powers);
    auto ct_powers = cc->Encrypt(keyPair.publicKey, pt_powers);

    auto ct_sum = cc->EvalSum(ct_powers, 8);
    auto v_sum = decrypt_state(ct_sum);
    
    double sum_phi = (pow(PHI, 8) - 1.0) / (PHI - 1.0);
    cout << "    EvalSum = " << v_sum[0] << "\n";
    cout << "    φ-formula = " << sum_phi << "\n";
    cout << "    Match: " << (abs(v_sum[0] - sum_phi) < 0.01 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct_sum->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: Fibonacci na libre via EvalAdd
    // ============================================
    cout << "  TEST 3: Fibonacci na libre via EvalAdd\n\n";
    cout << "  φ^(n+1) = φ^n + φ^(n-1) — Fibonacci!\n";
    cout << "  Ito ay EvalAdd lang — Level 0\n\n";

    // φ³ = φ² + φ¹
    vector<double> phi2(8, pow(PHI, 2));
    vector<double> phi1(8, pow(PHI, 1));
    Plaintext pt2 = cc->MakeCKKSPackedPlaintext(phi2);
    Plaintext pt1 = cc->MakeCKKSPackedPlaintext(phi1);
    auto ct2 = cc->Encrypt(keyPair.publicKey, pt2);
    auto ct1 = cc->Encrypt(keyPair.publicKey, pt1);
    
    auto ct3 = cc->EvalAdd(ct2, ct1);
    auto v3 = decrypt_state(ct3);
    
    cout << "    φ² + φ¹ = " << v3[0] << "\n";
    cout << "    Expected φ³ = " << pow(PHI, 3) << "\n";
    cout << "    Match: " << (abs(v3[0] - pow(PHI, 3)) < 0.001 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct3->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: φ-power chain — 10K na walang Mult
    // ============================================
    cout << "  TEST 4: φ-power chain — 10K na walang Mult\n\n";
    cout << "  Ang Fibonacci recurrence ay EvalAdd lang\n";
    cout << "  φ^n = φ^(n-1) + φ^(n-2)\n\n";

    auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(vector<double>(8, 1.0)));
    auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(vector<double>(8, PHI)));
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        auto ct_next = cc->EvalAdd(ct_a, ct_b);
        ct_a = ct_b;
        ct_b = ct_next;
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_chain = decrypt_state(ct_b);
    
    cout << "    After 10K Fibonacci steps:\n";
    cout << "    φ^10002 = " << v_chain[0] << "\n";
    cout << "    Expected: " << pow(PHI, 10002) << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (10000 * 1000.0) / time << "\n";
    cout << "    Level: " << ct_b->GetLevel() << "\n";

    return 0;
}
