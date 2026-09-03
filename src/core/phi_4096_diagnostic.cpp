// ============================================
// φ-4096 DIAGNOSTIC — BAKIT PERFECT?
//
// Test 1: 4096 sequential
// Test 2: 4096 parallel
// Test 3: 4096 fractal 2-level (64×64)
// Test 4: 4096 fractal 3-level (16×16×16)
// Test 5: 4096 fractal 4-level (8×8×8×8)
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
    cout << "  φ-4096 DIAGNOSTIC\n";
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

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n\n";

    auto encrypt_batch = [&](vector<double>& vals) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_batch = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        vector<double> results(16);
        for (int i = 0; i < 16; i++) {
            results[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return results;
    };

    auto zero_adaptive_decode = [&](double val) {
        double mod_phi = fmod(val, PHI);
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 0;
        return (mod_phi < PHI / 2.0) ? 1 : 0;
    };

    // ============================================
    // TEST 1: SEQUENTIAL 4096
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: SEQUENTIAL 4096\n";
    cout << "========================================\n\n";

    {
        vector<double> vals(16, PHI);
        auto ct = encrypt_batch(vals);
        vector<double> add_vals(16, PHI_INV);
        auto ct_add = encrypt_batch(add_vals);
        
        int match = 0;
        auto start = high_resolution_clock::now();
        
        for (int i = 0; i < 4096; i++) {
            ct = cc->EvalAdd(ct, ct_add);
            auto dec = decrypt_batch(ct);
            int decoded = zero_adaptive_decode(dec[0]);
            int expected = (i + 1) % 2;
            match += (decoded == expected);
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        cout << "  Match: " << match << "/4096\n";
        cout << "  Time: " << time << " ms\n\n";
    }

    // ============================================
    // TEST 2: PARALLEL 4096 (256 BATCHES × 16)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 2: PARALLEL 4096\n";
    cout << "========================================\n\n";

    {
        int match = 0;
        auto start = high_resolution_clock::now();
        
        for (int batch = 0; batch < 256; batch++) {
            vector<double> vals(16, 0.0);
            for (int i = 0; i < 16; i++) {
                vals[i] = ((batch * 16 + i) % 2 == 0) ? PHI : PHI_INV;
            }
            auto ct = encrypt_batch(vals);
            
            vector<double> add_vals(16, PHI_INV);
            auto ct_add = encrypt_batch(add_vals);
            
            auto ct_result = cc->EvalAdd(ct, ct_add);
            auto dec = decrypt_batch(ct_result);
            
            for (int i = 0; i < 16; i++) {
                int decoded = zero_adaptive_decode(dec[i]);
                int expected = (batch * 16 + i + 1) % 2;
                match += (decoded == expected);
            }
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        cout << "  Match: " << match << "/4096\n";
        cout << "  Time: " << time << " ms\n\n";
    }

    // ============================================
    // TEST 3: FRACTAL 2-LEVEL (64×64)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 3: FRACTAL 2-LEVEL (64×64)\n";
    cout << "========================================\n\n";

    {
        int match = 0;
        auto start = high_resolution_clock::now();
        
        for (int outer = 0; outer < 64; outer++) {
            for (int inner = 0; inner < 64; inner++) {
                vector<double> vals(16, 0.0);
                for (int i = 0; i < 16; i++) {
                    vals[i] = ((outer + inner + i) % 2 == 0) ? PHI : PHI_INV;
                }
                auto ct = encrypt_batch(vals);
                
                vector<double> add_vals(16, PHI_INV);
                auto ct_add = encrypt_batch(add_vals);
                
                auto ct_result = cc->EvalAdd(ct, ct_add);
                auto dec = decrypt_batch(ct_result);
                
                for (int i = 0; i < 16; i++) {
                    int decoded = zero_adaptive_decode(dec[i]);
                    int expected = (outer + inner + i + 1) % 2;
                    match += (decoded == expected);
                }
            }
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        cout << "  Match: " << match << "/4096\n";
        cout << "  Time: " << time << " ms\n\n";
    }

    // ============================================
    // TEST 4: FRACTAL 3-LEVEL (16×16×16)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 4: FRACTAL 3-LEVEL (16×16×16)\n";
    cout << "========================================\n\n";

    {
        int match = 0;
        auto start = high_resolution_clock::now();
        
        for (int level3 = 0; level3 < 16; level3++) {
            for (int level2 = 0; level2 < 16; level2++) {
                for (int level1 = 0; level1 < 16; level1++) {
                    vector<double> vals(16, 0.0);
                    for (int i = 0; i < 16; i++) {
                        vals[i] = ((level3 + level2 + level1 + i) % 2 == 0) ? PHI : PHI_INV;
                    }
                    auto ct = encrypt_batch(vals);
                    
                    vector<double> add_vals(16, PHI_INV);
                    auto ct_add = encrypt_batch(add_vals);
                    
                    auto ct_result = cc->EvalAdd(ct, ct_add);
                    auto dec = decrypt_batch(ct_result);
                    
                    for (int i = 0; i < 16; i++) {
                        int decoded = zero_adaptive_decode(dec[i]);
                        int expected = (level3 + level2 + level1 + i + 1) % 2;
                        match += (decoded == expected);
                    }
                }
            }
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        cout << "  Match: " << match << "/4096\n";
        cout << "  Time: " << time << " ms\n\n";
    }

    // ============================================
    // TEST 5: FRACTAL 4-LEVEL (8×8×8×8)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 5: FRACTAL 4-LEVEL (8×8×8×8)\n";
    cout << "========================================\n\n";

    {
        int match = 0;
        auto start = high_resolution_clock::now();
        
        for (int l4 = 0; l4 < 8; l4++) {
            for (int l3 = 0; l3 < 8; l3++) {
                for (int l2 = 0; l2 < 8; l2++) {
                    for (int l1 = 0; l1 < 8; l1++) {
                        vector<double> vals(16, 0.0);
                        for (int i = 0; i < 16; i++) {
                            vals[i] = ((l4 + l3 + l2 + l1 + i) % 2 == 0) ? PHI : PHI_INV;
                        }
                        auto ct = encrypt_batch(vals);
                        
                        vector<double> add_vals(16, PHI_INV);
                        auto ct_add = encrypt_batch(add_vals);
                        
                        auto ct_result = cc->EvalAdd(ct, ct_add);
                        auto dec = decrypt_batch(ct_result);
                        
                        for (int i = 0; i < 16; i++) {
                            int decoded = zero_adaptive_decode(dec[i]);
                            int expected = (l4 + l3 + l2 + l1 + i + 1) % 2;
                            match += (decoded == expected);
                        }
                    }
                }
            }
        }
        
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        cout << "  Match: " << match << "/4096\n";
        cout << "  Time: " << time << " ms\n\n";
    }

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  DIAGNOSTIC COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Lahat ng tests: 4096 operations\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
