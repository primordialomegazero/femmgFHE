// ============================================
// φ-PARAMETER SWEEP
// Hanapin ang tamang OpenFHE parameters
//
// Core mission:
// - Hanapin ang stable na parameters para sa φ-FHE
// - I-sweep ang depth, scaling mod, at first mod
// - Ang tamang parameters ang susi sa unbounded FHE
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace lbcrypto;
using namespace std;

const double PHI = 1.6180339887498948482;
const double INV_PHI = 1.0 / PHI;

void test_parameters(int depth, int scaling_mod, int first_mod) {
    cout << "Depth=" << depth << " Scaling=" << scaling_mod << " First=" << first_mod << ": ";
    
    try {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(depth);
        params.SetScalingModSize(scaling_mod);
        params.SetBatchSize(256);
        params.SetFirstModSize(first_mod);
        
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        auto slots = cc->GetEncodingParams()->GetBatchSize();
        
        auto make_ct = [&](double val) {
            vector<complex<double>> vec(slots, {0.0, 0.0});
            vec[0] = {val, 0.0};
            return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
        };
        
        auto decrypt_val = [&](auto ct) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            return pt->GetCKKSPackedValue()[0].real();
        };
        
        // Test: 5 multiplications na may φ-bootstrap
        auto x = make_ct(0.5);
        
        for (int i = 0; i < 5; i++) {
            // Multiply sa 2
            auto two = make_ct(2.0);
            x = cc->EvalMult(x, two);
            // φ-logistic bootstrap
            auto one_minus_x = cc->EvalSub(make_ct(1.0), x);
            x = cc->EvalMult(x, one_minus_x);
            x = cc->EvalMult(x, make_ct(PHI));
        }
        
        double result = decrypt_val(x);
        cout << "Final=" << result;
        cout << (abs(result) < 10 ? " STABLE ✓" : " UNSTABLE ✗") << "\n";
        
    } catch (const exception& e) {
        cout << "FAILED: " << e.what() << "\n";
    }
}

int main() {
    cout << "========================================\n";
    cout << "  φ-PARAMETER SWEEP\n";
    cout << "  Hanapin ang Tamang Parameters\n";
    cout << "========================================\n\n";
    
    // Sweep depth
    cout << "DEPTH SWEEP:\n";
    cout << "============\n\n";
    for (int depth : {10, 15, 20, 30, 40, 50}) {
        test_parameters(depth, 50, 60);
    }
    cout << "\n";
    
    // Sweep scaling mod size
    cout << "SCALING MOD SWEEP:\n";
    cout << "==================\n\n";
    for (int scaling : {30, 40, 50, 59, 60}) {
        test_parameters(30, scaling, 60);
    }
    cout << "\n";
    
    // Sweep first mod size
    cout << "FIRST MOD SWEEP:\n";
    cout << "================\n\n";
    for (int first : {50, 55, 60, 65, 70}) {
        test_parameters(30, 50, first);
    }
    cout << "\n";
    
    // Best combination
    cout << "BEST COMBINATIONS:\n";
    cout << "==================\n\n";
    
    test_parameters(30, 59, 60);
    test_parameters(40, 59, 60);
    test_parameters(50, 59, 65);
    test_parameters(30, 60, 60);
    
    cout << "\n========================================\n";
    cout << "  φ-PARAMETER SWEEP COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
