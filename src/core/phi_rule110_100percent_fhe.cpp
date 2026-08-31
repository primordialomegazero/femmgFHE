// ============================================
// φ-RULE 110 100% FHE — NO DECRYPTION
//
// Complete evolution na may linear sign
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-RULE 110 100% FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);
    parameters.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double EPSILON = pow(PHI, -6);
    const double LOWER = 5.0 * PHI - 7.0 - EPSILON;
    const double UPPER = 3.0 * PHI - 3.0 + EPSILON;

    // Positional encoding
    const double L_ZERO = pow(PHI, -4);
    const double L_ONE = pow(PHI, -1);
    const double C_ZERO = pow(PHI, -3);
    const double C_ONE = pow(PHI, 0);
    const double R_ZERO = pow(PHI, -3);
    const double R_ONE = pow(PHI, 0);

    // State encoding
    const double V_ZERO = pow(PHI, -5);
    const double V_ONE = pow(PHI, -2);

    cout << "✅ CKKS initialized (depth 1)\n";
    cout << "  Band: [" << LOWER << ", " << UPPER << "]\n";
    cout << "  Sign: 0.5 * p(x) + 0.5\n\n";

    // Helpers
    auto encrypt_value = [&](double val) {
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // PURE FHE RULE 110 TRANSITION
    // ============================================
    
    auto rule110_transition = [&](const Ciphertext<DCRTPoly>& l_ct,
                                   const Ciphertext<DCRTPoly>& c_ct,
                                   const Ciphertext<DCRTPoly>& r_ct) {
        // Sum
        auto sum1 = cc->EvalAdd(l_ct, c_ct);
        auto sum_ct = cc->EvalAdd(sum1, r_ct);
        
        // Band polynomial
        auto diff_lower = cc->EvalSub(sum_ct, LOWER);
        auto diff_upper = cc->EvalSub(UPPER, sum_ct);
        auto p_ct = cc->EvalMult(diff_lower, diff_upper);
        
        // Linear sign: 0.5 * p + 0.5
        auto sign_ct = cc->EvalMult(p_ct, 0.5);
        sign_ct = cc->EvalAdd(sign_ct, 0.5);
        
        // Ang output ay sign_ct (hindi na binary!)
        // Para sa next generation, kailangan nating i-convert
        // sa positional encoding
        
        return sign_ct;  // Returns encrypted score
    };
    
    // ============================================
    // TEST: FULL EVOLUTION (20 GENERATIONS)
    // ============================================
    
    int N = 16;
    vector<int> initial(N, 0);
    initial[7] = 1;
    initial[8] = 1;
    
    // Reference plaintext evolution
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};
    vector<vector<int>> plain_history;
    vector<int> current_plain = initial;
    plain_history.push_back(current_plain);
    
    for (int gen = 0; gen < 20; gen++) {
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = current_plain[(i + N - 1) % N];
            int C = current_plain[i];
            int R = current_plain[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        current_plain = next;
        plain_history.push_back(current_plain);
    }
    
    // FHE evolution (with encrypted states)
    vector<Ciphertext<DCRTPoly>> curr_states(N);
    for (int i = 0; i < N; i++) {
        curr_states[i] = encrypt_value(initial[i] ? V_ONE : V_ZERO);
    }
    
    cout << "FHE Evolution (20 generations):\n";
    cout << "================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    cout << "Gen  0: ";
    for (int i = 0; i < N; i++) cout << initial[i];
    cout << "\n";
    
    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next_states(N);
        
        for (int i = 0; i < N; i++) {
            // Get neighbors
            auto l_ct = curr_states[(i + N - 1) % N];
            auto c_ct = curr_states[i];
            auto r_ct = curr_states[(i + 1) % N];
            
            // Convert states to positional encoding
            // NOTE: Ito ay approximation - kailangan ng mas magandang conversion
            auto l_pos = l_ct;  // Placeholder
            auto c_pos = c_ct;  // Placeholder
            auto r_pos = r_ct;  // Placeholder
            
            // Apply transition
            auto result = rule110_transition(l_pos, c_pos, r_pos);
            
            // Convert result back to state encoding
            // score > 0.5 → V_ONE, score < 0.5 → V_ZERO
            // Sa pure FHE: V = V_ZERO + score * (V_ONE - V_ZERO)
            auto state_diff = encrypt_value(V_ONE - V_ZERO);
            auto scaled = cc->EvalMult(result, state_diff);
            auto next_state = cc->EvalAdd(scaled, V_ZERO);
            
            next_states[i] = next_state;
        }
        
        curr_states = next_states;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                double val = decrypt_value(curr_states[i]);
                int bit = (abs(val - V_ONE) < abs(val - V_ZERO)) ? 1 : 0;
                cout << bit;
            }
            cout << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << curr_states[0]->GetLevel() << "\n\n";
    
    // Verification
    int matches = 0;
    cout << "Verification (Gen 20):\n";
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << plain_history[20][i];
    cout << "\n  Encrypted: ";
    for (int i = 0; i < N; i++) {
        double val = decrypt_value(curr_states[i]);
        int bit = (abs(val - V_ONE) < abs(val - V_ZERO)) ? 1 : 0;
        cout << bit;
        if (bit == plain_history[20][i]) matches++;
    }
    cout << "\n\n  Match: " << matches << "/" << N << "\n";
    
    return 0;
}
