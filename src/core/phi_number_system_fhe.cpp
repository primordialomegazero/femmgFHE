// ============================================
// φ-NUMBER SYSTEM FHE — 100 iterations
// Binary φ-power vector encoding
// Carry: 2φ^i = φ^(i+1) + φ^(i-2)
// Walang EvalMult, walang decrypt
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
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, 4, 5, -1, -2, -3, -4, -5});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    const int MAX_POWER = 7;  // φ^-7 hanggang φ^7
    const int VEC_SIZE = 15;  // 2*7+1 = 15

    // Decompose integer sa binary φ-power vector
    auto decompose = [&](int x) {
        vector<double> bits(VEC_SIZE, 0.0);
        double remaining = x;
        
        for (int p = MAX_POWER; p >= -MAX_POWER; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                bits[p + MAX_POWER] = 1.0;
                remaining -= phi_p;
            }
        }
        return bits;
    };

    auto encrypt_bits = [&](const vector<double>& bits) {
        vector<double> v(16, 0.0);
        for (size_t i = 0; i < bits.size() && i < 16; i++) {
            v[i] = bits[i];
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(VEC_SIZE);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < VEC_SIZE; i++) {
            out.push_back(res[i].real());
        }
        return out;
    };

    auto value_from_bits = [&](const vector<double>& bits) {
        double val = 0;
        for (size_t i = 0; i < bits.size(); i++) {
            if (bits[i] > 0.5) {
                val += pow(PHI, (int)i - MAX_POWER);
            }
        }
        return val;
    };

    cout << "========================================\n";
    cout << "  φ-NUMBER SYSTEM FHE — 100 iterations\n";
    cout << "========================================\n\n";
    cout << "  Binary φ-power vector encoding\n";
    cout << "  Carry: 2φ^i = φ^(i+1) + φ^(i-2)\n\n";

    // ============================================
    // TEST 1: Decomposition at addition
    // ============================================
    cout << "  TEST 1: 5 + 3 = 8\n\n";

    auto bits_5 = decompose(5);
    auto bits_3 = decompose(3);

    cout << "  5 = ";
    for (size_t i = 0; i < bits_5.size(); i++) {
        if (bits_5[i] > 0.5) cout << "φ^" << (int)i - MAX_POWER << " ";
    }
    cout << "\n";
    
    cout << "  3 = ";
    for (size_t i = 0; i < bits_3.size(); i++) {
        if (bits_3[i] > 0.5) cout << "φ^" << (int)i - MAX_POWER << " ";
    }
    cout << "\n\n";

    auto ct_5 = encrypt_bits(bits_5);
    auto ct_3 = encrypt_bits(bits_3);

    auto ct_sum = cc->EvalAdd(ct_5, ct_3);
    auto sum_bits = decrypt_bits(ct_sum);

    cout << "  After EvalAdd (bago carry):\n  [";
    for (size_t i = 0; i < sum_bits.size(); i++) {
        cout << setw(3) << sum_bits[i];
    }
    cout << "]\n\n";

    double val_raw = value_from_bits(sum_bits);
    cout << "  Value (bago carry): " << val_raw << "\n";
    cout << "  Expected: 8\n\n";

    // ============================================
    // TEST 2: Sequence ng additions
    // ============================================
    cout << "  TEST 2: Sequence ng additions\n\n";

    auto ct_accum = encrypt_bits(bits_5);
    
    cout << "    Start: 5\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10; i++) {
        ct_accum = cc->EvalAdd(ct_accum, ct_3);
        auto acc_bits = decrypt_bits(ct_accum);
        double acc_val = value_from_bits(acc_bits);
        cout << "    +3 → " << acc_val << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Level: " << ct_accum->GetLevel() << "\n";

    return 0;
}
