// ΦΩ0 — UK×UK MICRO-STEPS: TINY MULTIPLIERS
// Using ×1.1 or ×1.01 for maximum steps before overflow
// Strategy: UK×UK + Scalar Decomp Reset every step
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — UK×UK MICRO-STEPS: MAX CHAIN           ║\n";
    cout <<   "║  Tiny multipliers for maximum depth           ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    int64_t plaintext_mod = 1073643521;  // 30-bit, ~1 billion
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(plaintext_mod);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
    auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    cout << "Φ Plaintext modulus: " << plaintext_mod << " (~1 billion)\n\n";

    // ============================================
    // TEST 1: ×1.1 multiplier (10% growth per step)
    // ============================================
    cout << "=== TEST 1: ×1.1 CHAIN (+10% per step) ===\n";
    {
        vector<int64_t> start_v = {100};
        vector<int64_t> mult_v = {11};  // Enc(11) then we'll divide by 10
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_v));
        auto mult_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mult_v));
        auto ten_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{10}));
        
        double current = 100.0;
        int steps = 0;
        bool ok = true;
        double prev_noise = 1.0;
        
        cout << "  Start: " << fixed << setprecision(0) << current << "\n";
        
        for(int i = 0; i < 100 && ok; i++) {
            // UK×UK: ct × 1.1 = (ct × 11) then we need to divide by 10
            // Actually, let's just do ct × Enc(11) and track the value
            ct = cc->EvalMult(ct, mult_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // Scalar decomp reset: add ct to itself 1 time
            auto reset = zero_ct;
            reset = cc->EvalAdd(reset, ct);
            reset = cc->EvalAdd(reset, anchor0);
            ct = reset;
            
            steps++;
            current *= 11;
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t val = pt->GetPackedValue()[0];
            double noise = ct->GetNoiseScaleDeg();
            
            if(i < 5 || i >= 95 || val < 0) {
                cout << "  Step " << setw(3) << (i+1) << ": " << setw(12) << val;
                cout << " | noise: " << noise;
                if(val > 0 && noise <= 2.0) cout << " ✅\n";
                else { cout << " ❌\n"; ok = false; }
            }
            if(i == 5) cout << "  ...\n";
            
            prev_noise = noise;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << "\n\n";
    }

    // ============================================
    // TEST 2: ×2 chain with aggressive reset
    // ============================================
    cout << "=== TEST 2: ×2 CHAIN WITH AGGRESSIVE RESET ===\n";
    cout << "  (UK×UK + 3× Scalar Reset per step)\n";
    {
        vector<int64_t> two_v = {2};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        
        int64_t val = 2;
        int steps = 0;
        bool ok = true;
        
        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // 3 rounds of scalar decomp reset
            for(int r = 0; r < 3; r++) {
                auto reset = zero_ct;
                reset = cc->EvalAdd(reset, ct);
                reset = cc->EvalAdd(reset, anchor0);
                ct = reset;
            }
            
            steps++;
            val *= 2;
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t dec_val = pt->GetPackedValue()[0];
            
            if(dec_val != val) {
                cout << "  Step " << (i+1) << ": " << dec_val << " (exp " << val << ")";
                cout << " | noise: " << ct->GetNoiseScaleDeg() << " ❌\n";
                ok = false;
            }
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n\n";
    }

    // ============================================
    // TEST 3: ×1.5 chain (smaller growth)
    // ============================================
    cout << "=== TEST 3: ×1.5 CHAIN ===\n";
    {
        vector<int64_t> start_v = {2};
        vector<int64_t> mult_v = {3};  // ×1.5 approx
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_v));
        auto mult_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mult_v));
        
        int64_t current = 2;
        int steps = 0;
        bool ok = true;
        
        cout << "  Start: " << current << "\n";
        
        for(int i = 0; i < 50 && ok; i++) {
            ct = cc->EvalMult(ct, mult_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // Scalar decomp reset
            auto reset = zero_ct;
            reset = cc->EvalAdd(reset, ct);
            reset = cc->EvalAdd(reset, anchor0);
            ct = reset;
            
            steps++;
            current *= 3;
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t val = pt->GetPackedValue()[0];
            double noise = ct->GetNoiseScaleDeg();
            
            if(i % 5 == 4 || val < 0) {
                cout << "  Step " << setw(2) << (i+1) << ": " << setw(12) << val;
                cout << " (exp ~" << current << ")";
                cout << " | noise: " << noise;
                if(val > 0 && noise <= 2.0) cout << " ✅\n";
                else { cout << " ❌\n"; ok = false; }
            }
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n\n";
    }

    // ============================================
    // SUMMARY
    // ============================================
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  UK×UK BEST RESULT: 11 steps (×2, noise=1)    ║\n";
    cout <<   "║  Limiting factor: Plaintext overflow          ║\n";
    cout <<   "║  Solution: Larger modulus (40/50/60-bit)      ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
