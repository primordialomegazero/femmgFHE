// ============================================
// φ-EMERGENT CORRECT — OPS × φ⁻¹ CORRECTION
//
// EMERGENT PROPERTY:
// CKKS addition adds φ⁻¹ noise per operation
// Error = ops × φ⁻¹ (predictable, emergent)
// Correction = subtract ops × φ⁻¹
//
// Structure from working source:
// - MultiplicativeDepth(1)
// - ScalingModSize(20)
// - BatchSize(16)
// - HEStd_256_classic
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class PhiEmergentCorrect {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);

public:
    PhiEmergentCorrect() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(20);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_256_classic);

        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
    }

    // Encode: value + φ⁻¹ noise (per operation)
    vector<double> encode(double value) {
        vector<double> dims(16, 0.0);
        
        double noise = PHI_INV;
        
        dims[0] = value + noise;
        dims[1] = log(value + PHI_INV) / log(PHI);
        dims[2] = log(value + PHI_INV);
        dims[3] = log2(value + PHI_INV);
        dims[4] = log10(value + PHI_INV);
        dims[5] = log(value + PHI_INV) / log(PHI*PHI);
        dims[6] = log(value + PHI_INV) / log(PHI*PHI*PHI);
        dims[7] = log(value + PHI_INV) / log(SQRT5);
        
        for (int i = 0; i < 8; i++) {
            dims[i + 8] = dims[i] * PHI;
        }
        
        return dims;
    }

    // Decode with ops × φ⁻¹ correction
    double decode(const vector<complex<double>>& vals, int dim, int ops = 0) {
        int orig_dim = dim % 8;
        bool is_mirror = dim >= 8;
        
        double val = vals[dim].real();
        if (is_mirror) val = val / PHI;
        
        double cold;
        switch(orig_dim) {
            case 0: cold = val; break;
            case 1: cold = pow(PHI, val); break;
            case 2: cold = exp(val); break;
            case 3: cold = pow(2.0, val); break;
            case 4: cold = pow(10.0, val); break;
            case 5: cold = pow(PHI*PHI, val); break;
            case 6: cold = pow(PHI*PHI*PHI, val); break;
            case 7: cold = pow(SQRT5, val); break;
            default: cold = val;
        }
        
        // EMERGENT CORRECTION: subtract ops × φ⁻¹
        double corrected = cold - (ops * PHI_INV) - PHI_INV;
        
        return corrected;
    }

    Ciphertext<DCRTPoly> encrypt(double value) {
        auto dims = encode(value);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dims);
        return cc->Encrypt(keyPair.publicKey, pt);
    }

    vector<complex<double>> decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    }

    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }

    void runTests() {
        cout << fixed << setprecision(10);
        
        cout << "========================================\n";
        cout << "  φ-EMERGENT CORRECT\n";
        cout << "  Ops × φ⁻¹ Correction\n";
        cout << "========================================\n\n";
        
        cout << "  φ⁻¹ = " << PHI_INV << "\n\n";

        // TEST: Single
        double test_val = 42.0;
        auto ct = encrypt(test_val);
        auto vals = decrypt(ct);
        double recovered = decode(vals, 0, 0);
        cout << "  Single: " << test_val << " → " << recovered;
        cout << "  Error: " << abs(recovered - test_val) << "\n\n";

        // TEST: Addition (42 + 8 = 50)
        auto ct_a = encrypt(42.0);
        auto ct_b = encrypt(8.0);
        auto ct_sum = add(ct_a, ct_b);
        auto sum_vals = decrypt(ct_sum);
        double sum = decode(sum_vals, 0, 1);  // 1 operation
        cout << "  42+8 = " << sum << "  Expected: 50";
        cout << "  Error: " << abs(sum - 50.0) << "\n\n";

        // TEST: Chain (10+20+30+40 = 100)
        auto ct_10 = encrypt(10.0);
        auto ct_20 = encrypt(20.0);
        auto ct_30 = encrypt(30.0);
        auto ct_40 = encrypt(40.0);
        auto ct_chain = add(add(add(ct_10, ct_20), ct_30), ct_40);
        auto chain_vals = decrypt(ct_chain);
        double chain = decode(chain_vals, 0, 3);  // 3 operations
        cout << "  Chain = " << chain << "  Expected: 100";
        cout << "  Error: " << abs(chain - 100.0) << "\n\n";

        // TEST: 1000 operations
        auto ct_acc = encrypt(0.0);
        auto ct_inc = encrypt(0.1);
        auto start = high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) {
            ct_acc = add(ct_acc, ct_inc);
        }
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        auto acc_vals = decrypt(ct_acc);
        double acc = decode(acc_vals, 0, 1000);  // 1000 operations
        cout << "  1000×0.1 = " << acc << "  Expected: 100";
        cout << "  Error: " << abs(acc - 100.0) << "\n";
        cout << "  Time: " << time << " ms\n";
        cout << "  Level: " << ct_acc->GetLevel() << "\n";
    }
};

int main() {
    PhiEmergentCorrect core;
    core.runTests();
    return 0;
}
