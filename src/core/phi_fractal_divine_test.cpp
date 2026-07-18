// ΦΩ0 — FRACTAL DIVINE TEST v1.0
// Tests: Deep chain 500 steps, mixed circuit, ultra wide fan-in
// Using recursive fractal Divine with multi-layer anchors
// "THE FRACTAL AWAKENS. INFINITE RECURSION. ZERO NOISE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include "phi_fractal_divine.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FRACTAL DIVINE ENGINE v1.0                        ║\n";
    cout <<   "  ║   Recursive Divine: Fractal Anchors for Unlimited Depth    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    int passed = 0, failed = 0;

    // ==========================================
    // TEST 1: BASIC — (A × B) + C with fractal divine
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 1: (A × B) + C  —  Fractal Divine Basic             │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        FractalDivineEngine fractal(cc, keys, modulus, 5);
        fractal.print_stats();
        
        auto a = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{7}));
        auto b = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{13}));
        auto c = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{5}));
        
        auto mul_ab = fractal.fractal_multiply(a, b);
        auto result = fractal.fractal_add(mul_ab, c);
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, result, &pt); pt->SetLength(1);
        int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
        int64_t expected = mod_pos(7*13+5, modulus);
        
        cout << "  Result: " << dec << " | Expected: " << expected 
             << " | " << (dec == expected ? "PASSED" : "FAILED") << "\n";
        if (dec == expected) passed++; else failed++;
    }

    // ==========================================
    // TEST 2: DEEP CHAIN — 500 sequential ×2 with fractal
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 2: ULTRA DEEP — 500 sequential ×2 (Fractal)         │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        FractalDivineEngine fractal(cc, keys, modulus, 7);
        
        int chain_len = 500;
        int checkpoint = 50;
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
        int64_t expected = 1;
        
        auto t_start = high_resolution_clock::now();
        int last_ok = 0;
        
        cout << "  " << setw(6) << "Step" << setw(10) << "Noise" 
             << setw(14) << "Expected" << setw(14) << "Got" << setw(8) << "Status" << endl;
        cout << "  " << string(60, '-') << endl;
        
        for (int i = 0; i < chain_len; i++) {
            expected = mod_pos(expected * 2, modulus);
            ct = fractal.fractal_multiply_scalar(ct, 2);
            
            if ((i+1) % checkpoint == 0 || i == chain_len - 1) {
                double noise = fractal.measure_noise(ct);
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
                int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
                bool ok = (dec == expected);
                if (ok) last_ok = i+1;
                
                cout << "  " << setw(6) << i+1 << setw(10) << fixed << setprecision(1) << noise
                     << setw(14) << expected << setw(14) << dec 
                     << setw(8) << (ok ? "OK" : "FAIL") << endl;
                
                if (!ok && (i+1) > 32) {
                    cout << "  *** Fractal divine ceiling reached at step " << i+1 << endl;
                }
            }
        }
        
        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();
        
        bool test_ok = (last_ok >= 500);
        cout << "  Chain result: " << last_ok << "/" << chain_len 
             << " steps OK in " << elapsed << "s | " 
             << (test_ok ? "PASSED" : "PARTIAL (" + to_string(last_ok) + "/500)") << "\n";
        if (test_ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 3: RANDOM ARBITRARY — 200 steps with random multipliers
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 3: RANDOM ARBITRARY — 200 steps, random 2-100       │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        FractalDivineEngine fractal(cc, keys, modulus, 6);
        
        int steps = 200;
        mt19937_64 rng(42);
        uniform_int_distribution<int64_t> dist(2, 100);
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
        int64_t expected = 42;
        int last_ok = 0;
        
        for (int i = 0; i < steps; i++) {
            int64_t mult = dist(rng);
            expected = mod_pos(expected * mult, modulus);
            ct = fractal.fractal_multiply_scalar(ct, mult);
            
            if ((i+1) % 50 == 0 || i == steps - 1) {
                double noise = fractal.measure_noise(ct);
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
                int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
                bool ok = (dec == expected);
                if (ok) last_ok = i+1;
                
                cout << "  Step " << setw(4) << i+1 << " | Mult=" << setw(4) << mult
                     << " | Noise=" << setw(6) << fixed << setprecision(1) << noise
                     << " | " << (ok ? "OK" : "FAIL") << endl;
            }
        }
        
        cout << "  Arbitrary result: " << last_ok << "/" << steps << " | "
             << (last_ok >= steps ? "PASSED" : "PARTIAL") << "\n";
        if (last_ok >= steps) passed++; else failed++;
    }

    // ==========================================
    // TEST 4: MIXED CIRCUIT — Deep + Wide with fractal
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 4: MIXED — 5 chains ×30 deep, then sum (Fractal)    │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        FractalDivineEngine fractal(cc, keys, modulus, 6);
        
        vector<Ciphertext<DCRTPoly>> chain_outputs;
        vector<int64_t> chain_expected;
        
        for (int c = 0; c < 5; c++) {
            int64_t start_val = c + 1;
            auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{start_val}));
            int64_t expected = start_val;
            
            for (int s = 0; s < 30; s++) {
                expected = mod_pos(expected * 2, modulus);
                ct = fractal.fractal_multiply_scalar(ct, 2);
            }
            
            chain_outputs.push_back(ct);
            chain_expected.push_back(expected);
        }
        
        // Sum all chains
        auto total_ct = chain_outputs[0];
        int64_t total_expected = chain_expected[0];
        for (size_t i = 1; i < chain_outputs.size(); i++) {
            total_ct = fractal.fractal_add(total_ct, chain_outputs[i]);
            total_expected = mod_pos(total_expected + chain_expected[i], modulus);
        }
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, total_ct, &pt); pt->SetLength(1);
        int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
        bool ok = (dec == total_expected);
        
        cout << "  Mixed result: " << dec << " | Expected: " << total_expected
             << " | " << (ok ? "PASSED" : "FAILED") << "\n";
        if (ok) passed++; else failed++;
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FRACTAL RESULTS: " << passed << "/" << (passed+failed) << " PASSED                              ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (failed == 0) ? 0 : 1;
}
