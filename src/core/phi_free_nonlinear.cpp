// ============================================
// φ-FREE NONLINEAR — EvalSquare at EvalSum
// Libreng non-linear functions sa CKKS
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
    cout << "  φ-FREE NONLINEAR — EvalSquare at Sum\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: EvalSquare
    // ============================================
    cout << "  TEST 1: EvalSquare\n\n";

    vector<double> phi_val(8, PHI);
    Plaintext pt_phi = cc->MakeCKKSPackedPlaintext(phi_val);
    auto ct_phi = cc->Encrypt(keyPair.publicKey, pt_phi);

    auto ct_square = cc->EvalSquare(ct_phi);
    auto v_square = decrypt_state(ct_square);
    
    cout << "  φ² = " << v_square[0] << "\n";
    cout << "  Expected (φ+1): " << PHI + 1.0 << "\n";
    cout << "  Match: " << (abs(v_square[0] - (PHI + 1.0)) < 0.001 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_square->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: EvalSum
    // ============================================
    cout << "  TEST 2: EvalSum\n\n";

    vector<double> phi_powers(8, 0.0);
    for (int i = 0; i < 8; i++) {
        phi_powers[i] = pow(PHI, i);
    }
    Plaintext pt_powers = cc->MakeCKKSPackedPlaintext(phi_powers);
    auto ct_powers = cc->Encrypt(keyPair.publicKey, pt_powers);

    auto ct_sum = cc->EvalSum(ct_powers, 8);
    auto v_sum = decrypt_state(ct_sum);
    
    double expected_sum = 0;
    for (double p : phi_powers) expected_sum += p;
    
    cout << "  Sum ng φ-powers = " << v_sum[0] << "\n";
    cout << "  Expected: " << expected_sum << "\n";
    cout << "  Match: " << (abs(v_sum[0] - expected_sum) < 0.01 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_sum->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: φ-identity na libre
    // ============================================
    cout << "  TEST 3: φ-identity na libre\n\n";

    auto ct_phi_plus_1 = cc->EvalAdd(ct_phi, 1.0);
    auto v_phi_plus_1 = decrypt_state(ct_phi_plus_1);
    
    cout << "  EvalSquare(φ) = " << v_square[0] << "\n";
    cout << "  EvalAdd(φ, 1) = " << v_phi_plus_1[0] << "\n";
    cout << "  Match: " << (abs(v_square[0] - v_phi_plus_1[0]) < 0.001 ? "✅" : "❌") << "\n";
    cout << "  Level (EvalAdd): " << ct_phi_plus_1->GetLevel() << "\n";

    return 0;
}
