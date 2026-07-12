// ΦΩ0 — UK×UK NOVEL STRATEGIES
// Strategy A: Pre-stabilization before UK×UK
// Strategy B: Staggered multipliers (alternating ×2, ×3)
// Strategy C: Noise budgeting (save noise budget for later)
// Strategy D: Burst reset (reset only when noise > threshold)
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — UK×UK NOVEL STRATEGIES                 ║\n";
    cout <<   "║  Pre-stabilize, Stagger, Budget, Burst        ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(1073643521);
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

    auto get_noise = [&](auto& ct) { return ct->GetNoiseScaleDeg(); };

    // ============================================
    // STRATEGY A: PRE-STABILIZATION
    // 5 rounds of ZANS BEFORE UK×UK, then UK×UK, then 3× reset
    // ============================================
    cout << "=== STRATEGY A: PRE-STABILIZATION ===\n";
    cout << "  (5× ZANS before UK×UK + UK×UK + 3× reset)\n";
    {
        vector<int64_t> two_v = {2};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        
        int64_t val = 2;
        int steps = 0;
        bool ok = true;
        
        for(int i = 0; i < 35 && ok; i++) {
            // PRE-STABILIZE: 5 rounds of ZANS
            for(int j = 0; j < 5; j++) {
                ct = cc->EvalAdd(ct, anchor0);
            }
            
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // 3× reset
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
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << get_noise(ct) << "\n\n";
    }

    // ============================================
    // STRATEGY B: STAGGERED MULTIPLIERS
    // Alternate between ×2 and ×3 to "confuse" noise pattern
    // ============================================
    cout << "=== STRATEGY B: STAGGERED MULTIPLIERS ===\n";
    cout << "  (Alternate ×2, ×3, ×2, ×3 with 3× reset each)\n";
    {
        vector<int64_t> start_v = {2};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_v));
        auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
        auto three_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{3}));
        
        int64_t val = 2;
        int steps = 0;
        bool ok = true;
        
        for(int i = 0; i < 30 && ok; i++) {
            auto mult_ct = (i % 2 == 0) ? two_ct : three_ct;
            int mult_val = (i % 2 == 0) ? 2 : 3;
            
            ct = cc->EvalMult(ct, mult_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // 3× reset
            for(int r = 0; r < 3; r++) {
                auto reset = zero_ct;
                reset = cc->EvalAdd(reset, ct);
                reset = cc->EvalAdd(reset, anchor0);
                ct = reset;
            }
            
            steps++;
            val *= mult_val;
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << get_noise(ct) << "\n\n";
    }

    // ============================================
    // STRATEGY C: NOISE BUDGETING
    // Only reset when noise exceeds threshold (save computation)
    // ============================================
    cout << "=== STRATEGY C: NOISE BUDGETING ===\n";
    cout << "  (Reset only when noise > 2.0, otherwise skip reset)\n";
    {
        vector<int64_t> two_v = {2};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        
        int64_t val = 2;
        int steps = 0;
        bool ok = true;
        int resets_done = 0;
        int resets_skipped = 0;
        
        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // Budget check: only reset if noise > 2.0
            if(get_noise(ct) > 2.0) {
                for(int r = 0; r < 3; r++) {
                    auto reset = zero_ct;
                    reset = cc->EvalAdd(reset, ct);
                    reset = cc->EvalAdd(reset, anchor0);
                    ct = reset;
                }
                resets_done++;
            } else {
                resets_skipped++;
            }
            
            steps++;
            val *= 2;
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << get_noise(ct);
        cout << " | Resets: " << resets_done << " done, " << resets_skipped << " skipped\n\n";
    }

    // ============================================
    // STRATEGY D: BURST RESET
    // Run without reset, then do MASSIVE reset (10 rounds) when needed
    // ============================================
    cout << "=== STRATEGY D: BURST RESET ===\n";
    cout << "  (No reset for 3 steps, then 10× burst reset)\n";
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
            
            // Every 3rd step: BURST 10× reset
            if(i % 3 == 2) {
                for(int r = 0; r < 10; r++) {
                    auto reset = zero_ct;
                    reset = cc->EvalAdd(reset, ct);
                    reset = cc->EvalAdd(reset, anchor0);
                    ct = reset;
                }
            }
            
            steps++;
            val *= 2;
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << get_noise(ct) << "\n\n";
    }

    // ============================================
    // STRATEGY E: ADAPTIVE RESET (THE HYBRID KILLER)
    // Measure noise, choose optimal reset rounds (1-5)
    // ============================================
    cout << "=== STRATEGY E: ADAPTIVE RESET ===\n";
    cout << "  (1× reset if noise=1, 3× if noise=2, 5× if noise>2)\n";
    {
        vector<int64_t> two_v = {2};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_v));
        
        int64_t val = 2;
        int steps = 0;
        bool ok = true;
        int adaptive_rounds[5] = {0};
        
        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // Adaptive: choose reset rounds based on noise level
            double n = get_noise(ct);
            int reset_rounds;
            if(n <= 1.0) reset_rounds = 1;
            else if(n <= 2.0) reset_rounds = 3;
            else reset_rounds = 5;
            
            for(int r = 0; r < reset_rounds; r++) {
                auto reset = zero_ct;
                reset = cc->EvalAdd(reset, ct);
                reset = cc->EvalAdd(reset, anchor0);
                ct = reset;
            }
            
            if(reset_rounds < 5) adaptive_rounds[reset_rounds]++;
            else adaptive_rounds[4]++;
            
            steps++;
            val *= 2;
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << get_noise(ct);
        cout << " | 1×:" << adaptive_rounds[1] << " 3×:" << adaptive_rounds[3] << " 5×:" << adaptive_rounds[4] << "\n\n";
    }

    // ============================================
    // FINAL COMPARISON
    // ============================================
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  STRATEGY COMPARISON                          ║\n";
    cout <<   "╠════════════════════════════════════════════════╣\n";
    cout <<   "║  A: Pre-stabilize — adds overhead             ║\n";
    cout <<   "║  B: Staggered — tests pattern resistance      ║\n";
    cout <<   "║  C: Budgeting — saves computation             ║\n";
    cout <<   "║  D: Burst — tests delayed reset               ║\n";
    cout <<   "║  E: Adaptive — optimal per-step reset         ║\n";
    cout <<   "║  Baseline: 27 steps (3× fixed reset)          ║\n";
    cout <<   "╚════════════════════════════════════════════════╝\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
