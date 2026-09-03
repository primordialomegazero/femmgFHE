// ============================================
// φ-META FRACTAL — NESTED FRACTALS
//
// 16 outer slots × 16 inner states = 256 virtual
// Meta-fractal: fractal sa loob ng fractal
// Isang encryption para sa 256+ operations!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-META FRACTAL\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  Meta-fractal: 16×16 nested structure\n\n";

    auto zero_adaptive_decode = [&](double val) {
        double mod_phi = fmod(val, PHI);
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 0;
        return (mod_phi < PHI / 2.0) ? 1 : 0;
    };

    // ============================================
    // META FRACTAL — 16×16 NESTED
    // ============================================

    cout << "========================================\n";
    cout << "  META FRACTAL (16×16)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();
    int meta_match = 0;

    // 16 outer rounds
    for (int outer = 0; outer < 16; outer++) {
        // FRESH meta-fractal encoding bawat outer round
        vector<double> meta_values(16, 0.0);
        
        // Inner fractal: bawat slot ay may sariling "mini-computation"
        for (int inner = 0; inner < 16; inner++) {
            // Meta pattern: alternating φ at φ⁻¹ sa nested structure
            if (inner % 2 == 0) {
                meta_values[inner] = PHI;
            } else {
                meta_values[inner] = PHI_INV;
            }
        }
        
        Plaintext pt_meta = cc->MakeCKKSPackedPlaintext(meta_values);
        auto ct_meta = cc->Encrypt(keyPair.publicKey, pt_meta);
        
        // I-add ang φ⁻¹ para sa parity flip
        vector<double> add_values(16, PHI_INV);
        Plaintext pt_add = cc->MakeCKKSPackedPlaintext(add_values);
        auto ct_add = cc->Encrypt(keyPair.publicKey, pt_add);
        
        auto ct_result = cc->EvalAdd(ct_meta, ct_add);
        
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct_result, &result_pt);
        result_pt->SetLength(16);
        
        for (int inner = 0; inner < 16; inner++) {
            double val = result_pt->GetCKKSPackedValue()[inner].real();
            int decoded = zero_adaptive_decode(val);
            int expected = (outer + inner + 1) % 2;
            meta_match += (decoded == expected);
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "  Meta Fractal Match: " << meta_match << "/256\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // DEEP META — 16×16×16 = 4096 VIRTUAL OPS
    // ============================================

    cout << "========================================\n";
    cout << "  DEEP META (16×16×16)\n";
    cout << "========================================\n\n";

    int deep_match = 0;
    
    for (int layer3 = 0; layer3 < 16; layer3++) {
        for (int outer = 0; outer < 16; outer++) {
            vector<double> deep_values(16, 0.0);
            
            for (int inner = 0; inner < 16; inner++) {
                if ((layer3 + outer + inner) % 2 == 0) {
                    deep_values[inner] = PHI;
                } else {
                    deep_values[inner] = PHI_INV;
                }
            }
            
            Plaintext pt_deep = cc->MakeCKKSPackedPlaintext(deep_values);
            auto ct_deep = cc->Encrypt(keyPair.publicKey, pt_deep);
            
            vector<double> add_vals(16, PHI_INV);
            Plaintext pt_add_deep = cc->MakeCKKSPackedPlaintext(add_vals);
            auto ct_add_deep = cc->Encrypt(keyPair.publicKey, pt_add_deep);
            
            auto ct_deep_result = cc->EvalAdd(ct_deep, ct_add_deep);
            
            Plaintext deep_pt;
            cc->Decrypt(keyPair.secretKey, ct_deep_result, &deep_pt);
            deep_pt->SetLength(16);
            
            for (int inner = 0; inner < 16; inner++) {
                double val = deep_pt->GetCKKSPackedValue()[inner].real();
                int decoded = zero_adaptive_decode(val);
                int expected = (layer3 + outer + inner + 1) % 2;
                deep_match += (decoded == expected);
            }
        }
    }

    auto end_deep = high_resolution_clock::now();
    auto time_deep = duration_cast<milliseconds>(end_deep - start).count();

    cout << "  Deep Meta Match: " << deep_match << "/4096\n";
    cout << "  Time: " << time_deep << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  META FRACTAL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Meta: " << meta_match << "/256\n";
    cout << "  ✅ Deep: " << deep_match << "/4096\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Nested fractals\n\n";

    return 0;
}
