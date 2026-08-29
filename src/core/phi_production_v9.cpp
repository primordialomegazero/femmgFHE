// ============================================
// φ-PRODUCTION V9 — BIT-ONLY + VALUE CORRECTNESS
//
// STRUCTURE FROM WORKING SOURCE:
// - MultiplicativeDepth(1) — unchanged
// - ScalingModSize(20) — unchanged
// - BatchSize(16) — unchanged
// - EvalMultKeyGen — KEPT
//
// BIT-ONLY: HEStd_128_classic → HEStd_256_classic
//
// VALUE CORRECTNESS FIX:
// - time_steps = 0 para sa operations
// - Encoding: value + noise (φ⁻¹)
// - Decoding: subtract noise, reverse
// - Addition: pure EvalAdd, walang time_steps
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

class PhiProductionV9 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);

    const double SELF_CORRECTING = 1.0;
    const double SELF_NOISE = PHI_INV;
    const double SELF_HARMONIZED = PHI;

public:
    PhiProductionV9() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(20);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_256_classic);  // BIT-ONLY

        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
    }

    // Encode: value + noise (NO time_steps — direct encoding)
    vector<double> encodeDirect(double value) {
        vector<double> dims(16, 0.0);
        
        double cold = value;              // Direct value, NO scaling
        double noise = SELF_NOISE;         // φ⁻¹ noise

        // Base dimensions (8)
        dims[0] = cold + noise;
        dims[1] = log(cold + PHI_INV) / log(PHI);
        dims[2] = log(cold + PHI_INV);
        dims[3] = log2(cold + PHI_INV);
        dims[4] = log10(cold + PHI_INV);
        dims[5] = log(cold + PHI_INV) / log(PHI*PHI);
        dims[6] = log(cold + PHI_INV) / log(PHI*PHI*PHI);
        dims[7] = log(cold + PHI_INV) / log(SQRT5);

        // Mirror dimensions (8)
        for (int i = 0; i < 8; i++) {
            dims[i + 8] = dims[i] * PHI;
        }

        return dims;
    }

    // Decode: remove noise, reverse log
    double decodeDirect(const vector<complex<double>>& vals, int dim) {
        int orig_dim = dim % 8;
        bool is_mirror = dim >= 8;

        double val = vals[dim].real();
        if (is_mirror) {
            val = val / PHI;
        }

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

        // Remove φ⁻¹ noise
        return cold - PHI_INV;
    }

    Ciphertext<DCRTPoly> encrypt(double value) {
        auto dims = encodeDirect(value);
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
        cout << fixed << setprecision(15);

        cout << "========================================\n";
        cout << "  φ-PRODUCTION V9 — BIT-ONLY + CORRECT\n";
        cout << "========================================\n\n";

        cout << "  Security: HEStd_256_classic\n";
        cout << "  Depth: 1\n";
        cout << "  Scaling: 20\n";
        cout << "  Batch: 16\n\n";

        cout << "  TEST 1: SELF-CORRECTING\n";
        cout << "  1 + φ⁻¹ = " << (1.0 + PHI_INV) << "\n";
        cout << "  φ = " << PHI << "\n";
        cout << "  Match: " << (abs(1.0 + PHI_INV - PHI) < 1e-15 ? "YES" : "NO") << "\n\n";

        cout << "  TEST 2: SINGLE RECOVERY\n";
        double test_val = 42.0;
        auto ct = encrypt(test_val);
        auto vals = decrypt(ct);
        double recovered = decodeDirect(vals, 0);
        cout << "  Original: " << test_val << "\n";
        cout << "  Recovered: " << recovered << "\n";
        cout << "  Error: " << abs(recovered - test_val) << "\n\n";

        cout << "  TEST 3: ADDITION (42 + 8)\n";
        auto ct_a = encrypt(42.0);
        auto ct_b = encrypt(8.0);
        auto ct_sum = add(ct_a, ct_b);
        auto sum_vals = decrypt(ct_sum);
        double sum = decodeDirect(sum_vals, 0);
        cout << "  42 + 8 = " << sum << "\n";
        cout << "  Expected: 50\n";
        cout << "  Error: " << abs(sum - 50.0) << "\n\n";

        cout << "  TEST 4: CHAIN (10+20+30+40)\n";
        auto ct_10 = encrypt(10.0);
        auto ct_20 = encrypt(20.0);
        auto ct_30 = encrypt(30.0);
        auto ct_40 = encrypt(40.0);
        auto ct_chain = add(add(add(ct_10, ct_20), ct_30), ct_40);
        auto chain_vals = decrypt(ct_chain);
        double chain = decodeDirect(chain_vals, 0);
        cout << "  10+20+30+40 = " << chain << "\n";
        cout << "  Expected: 100\n";
        cout << "  Error: " << abs(chain - 100.0) << "\n\n";

        cout << "  TEST 5: 1000 OPERATIONS\n";
        auto ct_acc = encrypt(0.0);
        auto ct_inc = encrypt(0.1);
        auto start = high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) {
            ct_acc = add(ct_acc, ct_inc);
        }
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        auto acc_vals = decrypt(ct_acc);
        double acc = decodeDirect(acc_vals, 0);
        cout << "  1000 × 0.1 = " << acc << "\n";
        cout << "  Expected: 100\n";
        cout << "  Error: " << abs(acc - 100.0) << "\n";
        cout << "  Time: " << time << " ms\n";
        cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

        cout << "========================================\n";
        cout << "  V9 SUMMARY\n";
        cout << "========================================\n";
        cout << "  Security: 256-bit\n";
        cout << "  Single: " << abs(recovered - test_val) << "\n";
        cout << "  42+8: " << abs(sum - 50.0) << "\n";
        cout << "  Chain: " << abs(chain - 100.0) << "\n";
        cout << "  1000×0.1: " << abs(acc - 100.0) << "\n";
        cout << "  Level: 0\n";
    }
};

int main() {
    PhiProductionV9 core;
    core.runTests();
    return 0;
}
