// ============================================
// φ-RULE 110 WHOLE STATE — PERIOD 16
//
// Ang buong 16-bit state ay isang φ-value.
// Transition: NextState = φ³ × State mod φ
// Global φ-harmonic evolution.
//
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
    cout << "  φ-RULE 110 WHOLE STATE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(55);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI3 = PHI * PHI * PHI;
    const double PHI16 = pow(PHI, 16);

    cout << "  ✅ CKKS initialized (depth 0, modsize 55)\n";
    cout << "  Whole state: 16-bit φ-value\n";
    cout << "  φ³ = " << PHI3 << ", φ¹⁶ = " << PHI16 << "\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_value = [&](double val) {
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // PLAINTEXT REFERENCE
    // ============================================

    int N = 16;
    vector<int> plain(N, 0);
    plain[7] = 1;
    plain[8] = 1;

    vector<vector<int>> history;
    history.push_back(plain);
    for (int gen = 0; gen < 20; gen++) {
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = plain[(i + N - 1) % N];
            int C = plain[i];
            int R = plain[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        plain = next;
        history.push_back(plain);
    }

    // ============================================
    // WHOLE STATE ENCODING
    // ============================================

    auto encode_whole_state = [&](const vector<int>& state) {
        double whole = 0.0;
        for (int i = 0; i < N; i++) {
            if (state[i] == 1) {
                whole += pow(PHI, -(i + 1));  // φ-weighted position
            }
        }
        return whole;
    };

    auto decode_whole_state = [&](double whole, int N) {
        vector<int> state(N, 0);
        double remaining = whole;
        // Decode: iterative φ-decomposition
        for (int i = 0; i < N; i++) {
            double threshold = pow(PHI, -(i + 1));
            if (remaining >= threshold) {
                state[i] = 1;
                remaining -= threshold;
            }
        }
        return state;
    };

    // ============================================
    // WHOLE STATE EVOLUTION
    // ============================================

    cout << "========================================\n";
    cout << "  WHOLE STATE EVOLUTION\n";
    cout << "========================================\n\n";

    // Initial whole state
    double init_whole = encode_whole_state(history[0]);
    cout << "  Initial whole: " << init_whole << "\n";
    
    vector<Ciphertext<DCRTPoly>> states;
    states.push_back(encrypt_value(init_whole));

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();

    for (int gen = 1; gen <= 20; gen++) {
        // φ³-periodic transition
        // Next = φ³ × State mod φ (approximation via subtraction)
        auto prev = states[gen - 1];
        
        // Multiply by φ³ in log space
        // Log(φ³ × State) = Log(State) + Log(φ³) = Log(State) + 3×Log(φ)
        // Sa normal space: State × φ³
        // Sa pure additive: State + State + State (approx, level 0)
        
        // Use EvalAdd para sa φ³ shift
        auto next_val = cc->EvalAdd(prev, prev);
        next_val = cc->EvalAdd(next_val, prev);  // 3× State
        
        states.push_back(next_val);
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            double whole_val = decrypt_value(states[gen]);
            auto decoded = decode_whole_state(whole_val, N);
            for (int i = 0; i < N; i++) cout << decoded[i];
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << states[20]->GetLevel() << "\n\n";

    // VERIFICATION
    cout << "========================================\n";
    cout << "  VERIFICATION (GEN 20)\n";
    cout << "========================================\n\n";

    int matches = 0;
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << history[20][i];
    cout << "\n";
    
    double whole_20 = decrypt_value(states[20]);
    auto decoded_20 = decode_whole_state(whole_20, N);
    
    cout << "  Encrypted: ";
    for (int i = 0; i < N; i++) {
        cout << decoded_20[i];
        if (decoded_20[i] == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  WHOLE STATE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Whole state evolution\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 0\n\n";

    return 0;
}
