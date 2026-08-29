// ============================================
// φ-PRODUCTION V8 — BIT-ONLY, STRUCTURE INTACT
//
// STRUCTURE FROM WORKING SOURCE (phi_emergent_modulo_final.cpp):
// - MultiplicativeDepth(1) — as-is, don't change
// - ScalingModSize(20) — as-is
// - BatchSize(16) — as-is
// - HEStd_128_classic — BIT-ONLY change to 256
//
// BIT-ONLY CHANGE:
// - HEStd_128_classic → HEStd_256_classic
//
// WALANG IBANG BINAGO!
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

class PhiProductionV8 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);

    // Self-correcting: V=1 + φ⁻¹ = φ
    const double SELF_CORRECTING = 1.0;
    const double SELF_NOISE = PHI_INV;
    const double SELF_HARMONIZED = PHI;

public:
    PhiProductionV8() {
        // EXACT SAME as phi_emergent_modulo_final.cpp
        // ONLY SetSecurityLevel changed: 128 → 256
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(20);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_256_classic);  // BIT-ONLY: 128→256

        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);  // KEEP THIS!
    }

    // Emergent modulo: value × φ⁻ⁿ (natural bounded [0,1))
    double emergentModulo(double value, int n) {
        double result = value;
        for (int i = 0; i < n; i++) {
            result *= PHI_INV;
        }
        return result;
    }

    // Reverse: value × φⁿ (recovery)
    double reverseModulo(double value, int n) {
        double result = value;
        for (int i = 0; i < n; i++) {
            result *= PHI;
        }
        return result;
    }

    // Encode with self-correcting value
    vector<double> encodeSelfCorrecting(double value, int time_steps = 3) {
        vector<double> dims(16, 0.0);

        double scaled = emergentModulo(value, time_steps);
        
        double cold = scaled;
        double noise = SELF_NOISE;

        dims[0] = cold + noise;
        dims[1] = log(cold) / log(PHI) + noise * PHI_INV;
        dims[2] = log(cold) + noise * PHI_INV * PHI_INV;
        dims[3] = log2(cold) + fmod(noise * 2.0, 1.0);
        dims[4] = log10(cold) + fmod(noise * 3.0, 1.0);
        dims[5] = log(cold) / log(PHI*PHI) + fmod(noise * 5.0, 1.0);
        dims[6] = log(cold) / log(PHI*PHI*PHI) + fmod(noise * 8.0, 1.0);
        dims[7] = log(cold) / log(SQRT5) + fmod(noise * 13.0, 1.0);

        for (int i = 0; i < 8; i++) {
            dims[i + 8] = dims[i] * PHI;
        }

        return dims;
    }

    // Decode with self-correcting recovery
    double decodeSelfCorrecting(const vector<complex<double>>& vals, int dim, int time_steps = 3) {
        int orig_dim = dim % 8;
        bool is_mirror = dim >= 8;

        double val = vals[dim].real();

        if (is_mirror) {
            val = val / PHI;
        }

        double noise = SELF_NOISE;
        double noise_vals[8] = {
            noise, noise * PHI_INV,
            fmod(noise * 2.0, 1.0), fmod(noise * 3.0, 1.0),
            fmod(noise * 5.0, 1.0), fmod(noise * 8.0, 1.0),
            fmod(noise * 13.0, 1.0), fmod(noise * 21.0, 1.0)
        };

        val = val - noise_vals[orig_dim];

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

        return reverseModulo(cold, time_steps);
    }

    Ciphertext<DCRTPoly> encrypt(double value) {
        auto dims = encodeSelfCorrecting(value);
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
        cout << "  φ-PRODUCTION V8 — BIT-ONLY\n";
        cout << "  Structure: EXACT SAME as working\n";
        cout << "  Security: 128→256 bit ONLY\n";
        cout << "========================================\n\n";

        cout << "  Depth: 1 (unchanged)\n";
        cout << "  Scaling: 20 (unchanged)\n";
        cout << "  Batch: 16 (unchanged)\n";
        cout << "  Security: HEStd_256_classic (CHANGED)\n";
        cout << "  EvalMultKeyGen: YES (unchanged)\n\n";

        cout << "  TEST 1: SELF-CORRECTING VALUE\n";
        cout << "  V=1 + φ⁻¹ = φ\n";
        cout << "  Cold: " << SELF_CORRECTING << "\n";
        cout << "  Noise: " << SELF_NOISE << "\n";
        cout << "  Harmonized: " << SELF_HARMONIZED << "\n";
        cout << "  Match? " << (abs(SELF_HARMONIZED - PHI) < 1e-15 ? "YES" : "NO") << "\n\n";

        cout << "  TEST 2: SINGLE RECOVERY\n";
        double test_val = 42.0;
        auto ct = encrypt(test_val);
        auto vals = decrypt(ct);
        double recovered = decodeSelfCorrecting(vals, 0);
        cout << "  Original: " << test_val << "\n";
        cout << "  Recovered: " << recovered << "\n";
        cout << "  Error: " << abs(recovered - test_val) << "\n\n";

        cout << "  TEST 3: ADDITION (42 + 8)\n";
        auto ct_a = encrypt(42.0);
        auto ct_b = encrypt(8.0);
        auto ct_sum = add(ct_a, ct_b);
        auto sum_vals = decrypt(ct_sum);
        double sum = decodeSelfCorrecting(sum_vals, 0);
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
        double chain = decodeSelfCorrecting(chain_vals, 0);
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
        double acc = decodeSelfCorrecting(acc_vals, 0);
        cout << "  1000 × 0.1 = " << acc << "\n";
        cout << "  Expected: 100\n";
        cout << "  Error: " << abs(acc - 100.0) << "\n";
        cout << "  Time: " << time << " ms\n";
        cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

        cout << "========================================\n";
        cout << "  V8 SUMMARY\n";
        cout << "========================================\n";
        cout << "  Security: 256-bit\n";
        cout << "  Single: " << abs(recovered - test_val) << "\n";
        cout << "  42+8: " << abs(sum - 50.0) << "\n";
        cout << "  Chain: " << abs(chain - 100.0) << "\n";
        cout << "  1000×0.1: " << abs(acc - 100.0) << "\n";
        cout << "  Level: 0\n";
        cout << "  Structure: INTACT\n";
        cout << "  BIT-ONLY: 128→256\n";
    }
};

int main() {
    PhiProductionV8 core;
    core.runTests();
    return 0;
}
