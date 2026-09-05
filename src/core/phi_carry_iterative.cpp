// ============================================
// φ-CARRY ITERATIVE — Pure FHE carry
// Paulit-ulit na rotate+add hanggang converge
// Walang decrypt, walang EvalMult
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, -1, -2});

    const double PHI = 1.6180339887498948482;
    const int MAX_POWER = 7;
    const int VEC_SIZE = 15;

    auto encrypt_bits = [&](const vector<double>& bits) {
        vector<double> v(16, 0.0);
        for (size_t i = 0; i < bits.size(); i++) v[i] = bits[i];
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(VEC_SIZE);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < VEC_SIZE; i++) out.push_back(res[i].real());
        return out;
    };

    auto value_from_bits = [&](const vector<double>& bits) {
        double val = 0;
        for (size_t i = 0; i < bits.size(); i++) {
            if (abs(bits[i]) > 0.5) {
                val += bits[i] * pow(PHI, (int)i - MAX_POWER);
            }
        }
        return val;
    };

    cout << "========================================\n";
    cout << "  φ-CARRY ITERATIVE — Pure FHE carry\n";
    cout << "========================================\n\n";
    cout << "  Paulit-ulit na rotate+add\n";
    cout << "  Hanggang mag-converge\n\n";

    // Initial: φ³ + φ² = 6.854
    vector<double> state(16, 0.0);
    state[10] = 1.0;  // φ³
    state[9] = 1.0;   // φ²

    auto ct = encrypt_bits(state);
    
    cout << "  Initial: φ³ + φ² = " << value_from_bits(decrypt_bits(ct)) << "\n";
    cout << "  Target: φ⁴ = " << pow(PHI, 4) << "\n\n";

    // Iterative carry propagation
    auto start = high_resolution_clock::now();
    
    for (int iter = 0; iter < 10; iter++) {
        // I-rotate by +1 at -1 para sa carry
        auto ct_rot_fwd = cc->EvalRotate(ct, 1);   // shift pababa
        auto ct_rot_bwd = cc->EvalRotate(ct, -1);  // shift pataas
        
        // Carry: φ^i + φ^(i-1) = φ^(i+1)
        // Sa vector: ang carry ay naglilipat ng value mula sa position i papunta sa i+1
        ct = cc->EvalAdd(ct, ct_rot_fwd);
        
        // Bawasan ang original positions na nag-carry
        ct = cc->EvalSub(ct, ct_rot_bwd);
        
        if (iter < 5 || iter == 9) {
            auto v = decrypt_bits(ct);
            cout << "  Iter " << iter << ": value = " << value_from_bits(v) << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_bits(ct);
    cout << "\n  Final value: " << value_from_bits(v_final) << "\n";
    cout << "  Target: " << pow(PHI, 4) << "\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
