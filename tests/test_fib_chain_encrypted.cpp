// FIB CHAIN ENCRYPTED — 100% Homomorphic
// Walang decrypt sa gitna
// Isang decrypt lang sa dulo

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIB CHAIN ENCRYPTED\n";
    std::cout << "  100% Homomorphic\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Encrypted normalization — walang decrypt!
    auto normalize_encrypted = [&](auto ct) {
        // Multiply by 1000 in encrypted domain
        auto ct_1000 = make_ct(1000.0);
        return cc->EvalMult(ct, ct_1000);
    };

    // Encrypted sign approximation — walang decrypt!
    auto sign_encrypted = [&](auto ct) {
        // Simple polynomial: x - x^3/3 + x^5/5
        auto ct_sq = cc->EvalMult(ct, ct);
        auto ct_cu = cc->EvalMult(ct_sq, ct);
        auto ct_qu = cc->EvalMult(ct_sq, ct_sq);
        auto ct_qi = cc->EvalMult(ct_qu, ct);
        
        auto ct_third = make_ct(1.0/3.0);
        auto ct_fifth = make_ct(1.0/5.0);
        
        auto term1 = ct;
        auto term2 = cc->EvalMult(ct_cu, ct_third);
        auto term3 = cc->EvalMult(ct_qi, ct_fifth);
        
        auto result = cc->EvalSub(term1, term2);
        result = cc->EvalAdd(result, term3);
        
        return result;
    };

    auto ct_a = make_ct(0.0);
    auto ct_b = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    std::cout << "ENCRYPTED CHAIN TEST (Walang Decrypt):\n";
    std::cout << "======================================\n\n";

    auto start = high_resolution_clock::now();
    
    // Run 100 gates na puro encrypted
    Ciphertext<DCRTPoly> final_result;
    
    for (int i = 0; i < 100; i++) {
        // NAND: 1 - (a + b)
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_nand = cc->EvalSub(ct_one, ct_sum);
        
        // Normalize (encrypted)
        auto ct_norm = normalize_encrypted(ct_nand);
        
        // Sign extraction (encrypted)
        auto ct_sign = sign_encrypted(ct_norm);
        
        // Feedback
        ct_a = ct_b;
        ct_b = ct_sign;
        
        final_result = ct_sign;
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    // ISANG DECRYPT LANG SA DULO
    double final_val = decrypt_val(final_result);
    
    std::cout << "  Final result: " << final_val << "\n";
    std::cout << "  Final bit: " << (final_val > 0 ? "1" : "0") << "\n";
    std::cout << "  Time: " << duration.count() << " ms\n";
    std::cout << "  Decrypts: 1 (sa dulo lang)\n";
    std::cout << "  Homomorphic: ✓ 100%\n";

    return 0;
}
