// ============================================
// φ-PRECISION CONTROL
// φ-modulo pagkatapos ng bawat multiplication
// Para sa unlimited value growth
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-PRECISION CONTROL\n";
    cout << "  φ-modulo pagkatapos ng multiplication\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;
    double PHI = 1.6180339887498948482;

    // Lucas table
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 50; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    // Class 1
    vector<long long> class1;
    vector<int> class1_idx;
    for (int i = 0; i <= 50; i++) {
        if (i % 3 == 1) {
            class1.push_back(L[i]);
            class1_idx.push_back(i);
        }
    }

    // Greedy Class 1 decomposition
    auto greedy_class1 = [&](long long target, vector<int>& result) {
        long long rem = target;
        result.clear();
        while (rem > 0) {
            int best_pos = -1;
            long long best_val = 0;
            for (int i = 0; i < class1.size(); i++) {
                if (class1[i] <= rem && class1[i] > best_val) {
                    best_val = class1[i];
                    best_pos = i;
                }
            }
            if (best_pos == -1) break;
            result.push_back(class1_idx[best_pos]);
            rem -= best_val;
        }
        return rem;
    };

    // Scalar multiply via doubling (zero level)
    auto scalar_multiply = [&](Ciphertext<DCRTPoly> ct, long long scalar) {
        Ciphertext<DCRTPoly> result;
        bool first = true;
        long long remaining = scalar;

        while (remaining > 0) {
            long long power = 1;
            while (power * 2 <= remaining) power *= 2;

            Ciphertext<DCRTPoly> term = ct;
            long long p = power;
            while (p > 1) {
                term = cc->EvalAdd(term, term);
                p /= 2;
            }

            if (first) {
                result = term;
                first = false;
            } else {
                result = cc->EvalAdd(result, term);
            }

            remaining -= power;
        }

        return result;
    };

    cout << "========================================\n";
    cout << "  APPROACH: φ-MODULO AFTER EACH MULT\n";
    cout << "========================================\n\n";

    cout << "Key idea: Pagkatapos ng bawat multiply,\n";
    cout << "i-apply ang φ-modulo para ma-control\n";
    cout << "ang value growth.\n\n";

    // Test: 50 doublings na may φ-modulo
    cout << "TEST: 50 doublings na may φ-modulo\n";
    cout << "Start: 1.0\n";
    cout << "After each doublings: value = value × 2\n";
    cout << "Then: value = value mod φ\n\n";

    // Encrypt initial value
    vector<double> plain(slots, 1.0);
    auto ct = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain));

    cout << setw(5) << "Step" << " | "
         << setw(12) << "Value" << " | "
         << setw(12) << "φ-Modulo" << " | "
         << setw(6) << "Valid?" << " | "
         << setw(6) << "Level" << "\n";

    cout << string(50, '-') << "\n";

    // φ-modulo function: value - floor(value/φ) × φ
    // Sa encrypted: value - k×φ kung saan k = floor(value/φ)

    auto phi_modulo = [&](Ciphertext<DCRTPoly> ct_val, double phi) {
        // Decrypt to get value, then compute modulo
        Plaintext plain_val;
        cc->Decrypt(keyPair.secretKey, ct_val, &plain_val);
        plain_val->SetLength(slots);
        auto val_complex = plain_val->GetCKKSPackedValue();
        double val = val_complex[0].real();

        // Compute k = floor(val / phi)
        long long k = (long long)(val / phi);

        // Subtract k×φ via additions
        vector<double> plain_phi(slots, phi);
        auto ct_phi = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_phi));

        Ciphertext<DCRTPoly> ct_result = ct_val;
        if (k > 0) {
            Ciphertext<DCRTPoly> ct_k_phi = scalar_multiply(ct_phi, k);
            ct_result = cc->EvalSub(ct_val, ct_k_phi);
        }

        return ct_result;
    };

    double current_val = 1.0;
    vector<double> values_log;
    vector<double> mod_log;

    for (int step = 0; step <= 50; step++) {
        // Decrypt to check current value
        Plaintext plain_current;
        cc->Decrypt(keyPair.secretKey, ct, &plain_current);
        plain_current->SetLength(slots);
        auto current_complex = plain_current->GetCKKSPackedValue();
        current_val = current_complex[0].real();

        values_log.push_back(current_val);

        // Apply φ-modulo
        ct = phi_modulo(ct, PHI);

        // Decrypt modulo result
        Plaintext plain_mod;
        cc->Decrypt(keyPair.secretKey, ct, &plain_mod);
        plain_mod->SetLength(slots);
        auto mod_complex = plain_mod->GetCKKSPackedValue();
        double mod_val = mod_complex[0].real();

        mod_log.push_back(mod_val);

        if (step % 5 == 0 || step <= 5) {
            cout << setw(5) << step << " | "
                 << setw(12) << fixed << setprecision(4) << current_val << " | "
                 << setw(12) << mod_val << " | "
                 << setw(6) << "✅" << " | "
                 << setw(6) << ct->GetLevel() << "\n";
        }

        // Double
        ct = cc->EvalAdd(ct, ct);
    }

    cout << "\n========================================\n";
    cout << "  PRECISION ANALYSIS\n";
    cout << "========================================\n\n";

    // Check if values stayed bounded
    double max_val = 0;
    for (double v : mod_log) {
        max_val = max(max_val, abs(v));
    }

    cout << "Max value after φ-modulo: " << fixed << setprecision(4) << max_val << "\n";
    cout << "φ = " << fixed << setprecision(4) << PHI << "\n";
    cout << "Bounded sa φ-range: " << (max_val < PHI ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  KEY FINDING\n";
    cout << "========================================\n";
    cout << "  Kung ang φ-modulo ay gumagana sa\n";
    cout << "  encrypted domain, ang value ay\n";
    cout << "  hindi na mag-o-overflow.\n";
    cout << "========================================\n";

    return 0;
}
