// ΦΩ0 — RING DIM 32768 with AUTO modulus selection
// OpenFHE auto-selects compatible modulus for ring dim 32768
// "THE SYSTEM KNOWS. LET IT CHOOSE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 32768;
    
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — RING DIM 32768: AUTO MODULUS SELECTION            ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    auto t_setup_start = high_resolution_clock::now();
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(100);
    params.SetRingDim(ring_dim);
    // NO plaintext modulus set — let OpenFHE choose compatible one
    
    cout << "  Generating crypto context (ring dim 32768)... " << flush;
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cout << "OK" << endl;
    
    // Get the auto-selected modulus
    auto crypto_params = cc->GetCryptoParameters();
    auto element_params = crypto_params->GetElementParams();
    auto modulus_obj = element_params->GetModulus();
    int64_t modulus = modulus_obj.ConvertToInt();
    int64_t half_mod = modulus / 2;
    
    cout << "  Auto-selected modulus: " << modulus << " (" << (int)log2(modulus) << " bits)" << endl;
    cout << "  Half modulus: " << half_mod << endl;
    
    cout << "  Generating keys... " << flush;
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cout << "OK" << endl;
    
    ZANSAnchorPool pool(cc, keys, 20);
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    
    auto t_setup_end = high_resolution_clock::now();
    double setup_time = duration_cast<seconds>(t_setup_end - t_setup_start).count();
    cout << "  Setup time: " << setup_time << "s\n\n";

    // ==========================================
    // TEST: Sequential ×2
    // ==========================================
    cout << "  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  SEQUENTIAL ×2 — Finding max depth (ring dim 32768)        │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    int64_t expected = 1;
    int max_steps = 500;
    int last_ok = 0;
    int checkpoint = 50;
    
    auto t_start = high_resolution_clock::now();
    
    cout << "  " << setw(6) << "Step" << setw(10) << "Noise"
         << setw(12) << "Time" << setw(8) << "Status" << endl;
    cout << "  " + string(45, '-') << endl;
    
    for (int step = 0; step < max_steps; step++) {
        expected = mod_pos(expected * 2, modulus);
        
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
        ct = cc->EvalMult(ct, ct_mult);
        
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        
        if ((step + 1) % checkpoint == 0) {
            double noise = ct->GetNoiseScaleDeg();
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec == expected);
            if (ok) last_ok = step + 1;
            
            auto t_now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(t_now - t_start).count();
            
            cout << "  " << setw(6) << step+1 
                 << setw(10) << fixed << setprecision(1) << noise
                 << setw(12) << fixed << setprecision(1) << elapsed
                 << setw(8) << (ok ? "OK" : "FAIL") << endl;
            
            if (!ok) {
                cout << "\n  *** FAILED at step " << step+1 
                     << " | Noise: " << noise 
                     << " | Max steps: " << last_ok << endl;
                break;
            }
        }
        
        if (step == max_steps - 1) last_ok = max_steps;
    }
    
    auto t_end = high_resolution_clock::now();
    double total_time = duration_cast<seconds>(t_end - t_start).count();
    
    cout << "  " + string(45, '-') << endl;
    cout << "  Max sequential ×2: " << last_ok << "/" << max_steps 
         << " in " << total_time << "s\n";
    cout << "  Improvement: " << (double)last_ok / 31.0 << "× over ring dim 4096\n\n";

    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RING DIM 32768: " << setw(4) << last_ok << " steps without bootstrap              ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
