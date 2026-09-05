// ============================================
// φ-POWER ARITHMETIC — CKKS na may φ
// Add, Sub, Mul ng φ-powers sa CKKS
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
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;

    cout << "========================================\n";
    cout << "  φ-POWER ARITHMETIC — CKKS na may φ\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: φ-power addition
    // ============================================
    cout << "  TEST 1: φ-power addition\n\n";

    // φ⁵ + φ⁴ = φ⁶
    vector<double> phi5(8, pow(PHI, 5));
    vector<double> phi4(8, pow(PHI, 4));
    
    Plaintext pt5 = cc->MakeCKKSPackedPlaintext(phi5);
    Plaintext pt4 = cc->MakeCKKSPackedPlaintext(phi4);
    
    auto ct5 = cc->Encrypt(keyPair.publicKey, pt5);
    auto ct4 = cc->Encrypt(keyPair.publicKey, pt4);
    
    auto ct_sum = cc->EvalAdd(ct5, ct4);
    
    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_sum, &pt_out);
    pt_out->SetLength(8);
    auto res_sum = pt_out->GetCKKSPackedValue();
    
    cout << "  φ⁵ + φ⁴ = " << res_sum[0].real() << "\n";
    cout << "  Expected φ⁶ = " << pow(PHI, 6) << "\n";
    cout << "  Match: " << (abs(res_sum[0].real() - pow(PHI, 6)) < 0.001 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: φ-power multiplication (ct × ct)
    // ============================================
    cout << "  TEST 2: φ-power multiplication (ct × ct)\n\n";

    // φ³ × φ² = φ⁵
    vector<double> phi3(8, pow(PHI, 3));
    vector<double> phi2(8, pow(PHI, 2));
    
    Plaintext pt3 = cc->MakeCKKSPackedPlaintext(phi3);
    Plaintext pt2 = cc->MakeCKKSPackedPlaintext(phi2);
    
    auto ct3 = cc->Encrypt(keyPair.publicKey, pt3);
    auto ct2 = cc->Encrypt(keyPair.publicKey, pt2);
    
    auto ct_mul = cc->EvalMult(ct3, ct2);
    
    Plaintext pt_out_mul;
    cc->Decrypt(keyPair.secretKey, ct_mul, &pt_out_mul);
    pt_out_mul->SetLength(8);
    auto res_mul = pt_out_mul->GetCKKSPackedValue();
    
    cout << "  φ³ × φ² = " << res_mul[0].real() << "\n";
    cout << "  Expected φ⁵ = " << pow(PHI, 5) << "\n";
    cout << "  Match: " << (abs(res_mul[0].real() - pow(PHI, 5)) < 0.01 ? "✅" : "❌") << "\n\n";
    cout << "  Level after mult: " << ct_mul->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: φ-power sa exponent space (EvalAdd)
    // ============================================
    cout << "  TEST 3: φ-power sa exponent space\n\n";
    cout << "  Sa exponent space: φ³ × φ² = φ⁵\n";
    cout << "  = EvalAdd(3, 2) = 5 — walang EvalMult!\n\n";

    vector<double> exp3(8, 3.0);
    vector<double> exp2(8, 2.0);
    
    Plaintext pt_exp3 = cc->MakeCKKSPackedPlaintext(exp3);
    Plaintext pt_exp2 = cc->MakeCKKSPackedPlaintext(exp2);
    
    auto ct_exp3 = cc->Encrypt(keyPair.publicKey, pt_exp3);
    auto ct_exp2 = cc->Encrypt(keyPair.publicKey, pt_exp2);
    
    auto ct_exp_add = cc->EvalAdd(ct_exp3, ct_exp2);
    
    Plaintext pt_exp_out;
    cc->Decrypt(keyPair.secretKey, ct_exp_add, &pt_exp_out);
    pt_exp_out->SetLength(8);
    auto res_exp = pt_exp_out->GetCKKSPackedValue();
    
    cout << "  Exponent sum: " << res_exp[0].real() << "\n";
    cout << "  φ^exponent = " << pow(PHI, res_exp[0].real()) << "\n";
    cout << "  Expected φ⁵ = " << pow(PHI, 5) << "\n";
    cout << "  Match: " << (abs(pow(PHI, res_exp[0].real()) - pow(PHI, 5)) < 0.01 ? "✅" : "❌") << "\n\n";
    cout << "  Level after EvalAdd: " << ct_exp_add->GetLevel() << "\n\n";
    cout << "  KEY: EvalAdd ay Level 0 — walang depth reduction!\n";

    return 0;
}
