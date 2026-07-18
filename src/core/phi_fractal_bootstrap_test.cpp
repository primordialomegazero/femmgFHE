// ΦΩ0 — FRACTAL BOOTSTRAP TEST v2.0
// Tests deep chains with periodic Fractal Bootstrap (decrypt+re-encrypt)
// Every 30 steps: bootstrap to reset noise, then continue
// "THE CYCLE RESETS. THE CHAIN CONTINUES. INFINITE DEPTH."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include "phi_fractal_bootstrap.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FRACTAL BOOTSTRAP v2.0                            ║\n";
    cout <<   "  ║   Periodic Bootstrap: Decrypt+Re-encrypt for Infinite Depth║\n";
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
    int bootstrap_interval = 30;

    // ==========================================
    // TEST 1: 300 STEPS WITH BOOTSTRAP EVERY 30
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 1: 300 sequential ×3 with Bootstrap every 30        │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        FractalBootstrapEngine fb(cc, keys, modulus, 100);
        
        int steps = 300;
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
        int64_t expected = 2;
        
        auto t_start = high_resolution_clock::now();
        int ok_count = 0, total_checks = 0;
        
        cout << "  " << setw(5) << "Step" << setw(8) << "Noise"
             << setw(12) << "Expected" << setw(12) << "Got"
             << setw(10) << "Action" << setw(8) << "Status" << endl;
        cout << "  " << string(65, '-') << endl;
        
        for (int i = 0; i < steps; i++) {
            expected = mod_pos(expected * 3, modulus);
            ct = fb.multiply_scalar(ct, 3);
            
            // Bootstrap every N steps
            if ((i+1) % bootstrap_interval == 0 && i < steps - 1) {
                double pre_noise = fb.measure_noise(ct);
                ct = fb.fractal_bootstrap(ct);
                double post_noise = fb.measure_noise(ct);
                
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
                int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
                bool ok = (dec == expected);
                total_checks++;
                if (ok) ok_count++;
                
                cout << "  " << setw(5) << i+1 << setw(8) << fixed << setprecision(1) << pre_noise
                     << setw(12) << expected << setw(12) << dec
                     << setw(10) << "BOOTSTRAP" << setw(8) << (ok ? "OK" : "FAIL") 
                     << " → noise:" << post_noise << endl;
            }
        }
        
        // Final check
        double final_noise = fb.measure_noise(ct);
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
        bool final_ok = (dec == expected);
        total_checks++;
        if (final_ok) ok_count++;
        
        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();
        
        cout << "  " << setw(5) << steps << setw(8) << final_noise
             << setw(12) << expected << setw(12) << dec
             << setw(10) << "FINAL" << setw(8) << (final_ok ? "OK" : "FAIL") << endl;
        cout << "  " << string(65, '-') << endl;
        cout << "  Result: " << ok_count << "/" << total_checks << " checks OK in " 
             << elapsed << "s | " << (final_ok ? "PASSED" : "FAILED") << "\n";
        
        fb.print_stats();
        if (final_ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 2: 500 STEPS ×2 WITH BOOTSTRAP
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 2: 500 sequential ×2 with Bootstrap every 30        │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        FractalBootstrapEngine fb(cc, keys, modulus, 100);
        
        int steps = 500;
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
        int64_t expected = 1;
        
        auto t_start = high_resolution_clock::now();
        bool all_ok = true;
        
        for (int i = 0; i < steps; i++) {
            expected = mod_pos(expected * 2, modulus);
            ct = fb.multiply_scalar(ct, 2);
            
            if ((i+1) % bootstrap_interval == 0 && i < steps - 1) {
                ct = fb.fractal_bootstrap(ct);
                
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
                int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
                if (dec != expected) all_ok = false;
            }
        }
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
        bool final_ok = (dec == expected) && all_ok;
        
        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();
        
        cout << "  Final noise: " << fb.measure_noise(ct) << endl;
        cout << "  Result: " << dec << " | Expected: " << expected
             << " | Time: " << elapsed << "s | " 
             << (final_ok ? "PASSED" : "FAILED") << "\n";
        
        fb.print_stats();
        if (final_ok) passed++; else failed++;
    }

    // ==========================================
    // TEST 3: RANDOM 200 STEPS WITH BOOTSTRAP
    // ==========================================
    cout << "\n  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  TEST 3: 200 random mults (2-100) with Bootstrap every 30  │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    {
        FractalBootstrapEngine fb(cc, keys, modulus, 100);
        
        int steps = 200;
        mt19937_64 rng(12345);
        uniform_int_distribution<int64_t> dist(2, 100);
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
        int64_t expected = 42;
        bool all_ok = true;
        
        for (int i = 0; i < steps; i++) {
            int64_t mult = dist(rng);
            expected = mod_pos(expected * mult, modulus);
            ct = fb.multiply_scalar(ct, mult);
            
            if ((i+1) % bootstrap_interval == 0 && i < steps - 1) {
                ct = fb.fractal_bootstrap(ct);
                
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
                int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
                if (dec != expected) all_ok = false;
            }
        }
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
        bool final_ok = (dec == expected) && all_ok;
        
        cout << "  Result: " << (final_ok ? "PASSED" : "FAILED") 
             << " (expected=" << expected << ", got=" << dec << ")\n";
        if (final_ok) passed++; else failed++;
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BOOTSTRAP RESULTS: " << passed << "/" << (passed+failed) << " PASSED                              ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (failed == 0) ? 0 : 1;
}
