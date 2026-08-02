// ΦΩ0 — UK×UK DEEP: AGGRESSIVE ZANS STABILIZATION
// Strategy 1: ZANS before AND after UK×UK
// Strategy 2: Multiple ZANS rounds between UK×UK
// Strategy 3: Fibonacci-spaced ZANS stabilization
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
    cout <<   "║  ΦΩ0 — UK×UK DEEP ZANS STRATEGIES             ║\n";
    cout <<   "║  Testing multiple stabilization approaches    ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
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

    // Setup: Enc(2) as multiplier
    vector<int64_t> two_vec = {2};
    auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));

    cout << "Φ Testing UK×UK ×2 chain with different ZANS strategies\n";
    cout << "Φ Goal: Maximize steps before corruption\n\n";

    // ============================================
    // STRATEGY 1: Standard — ZANS only after UK×UK
    // ============================================
    cout << "=== STRATEGY 1: ZANS After UK×UK ===\n";
    {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
        int64_t val = 2;
        int steps = 0;
        bool ok = true;

        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);  // 1 ZANS after
            steps++;
            val *= 2;

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n";
    }

    // ============================================
    // STRATEGY 2: ZANS Before AND After UK×UK
    // ============================================
    cout << "=== STRATEGY 2: ZANS Before + After UK×UK ===\n";
    {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
        int64_t val = 2;
        int steps = 0;
        bool ok = true;

        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalAdd(ct, anchor0);  // ZANS before
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);  // ZANS after
            steps++;
            val *= 2;

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n";
    }

    // ============================================
    // STRATEGY 3: Multi-ZANS (5 rounds between UK×UK)
    // ============================================
    cout << "=== STRATEGY 3: 5× ZANS Between UK×UK ===\n";
    {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
        int64_t val = 2;
        int steps = 0;
        bool ok = true;

        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalMult(ct, two_ct);
            // 5 rounds of ZANS
            for(int j = 0; j < 5; j++) {
                ct = cc->EvalAdd(ct, anchor0);
            }
            steps++;
            val *= 2;

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n";
    }

    // ============================================
    // STRATEGY 4: Fibonacci-spaced ZANS (φ pattern)
    // ============================================
    cout << "=== STRATEGY 4: Fibonacci-Spaced ZANS ===\n";
    cout << "  (1 ZANS after step 1, 2 after step 2, 3 after step 3, 5 after step 4...)\n";
    {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
        int64_t val = 2;
        int steps = 0;
        bool ok = true;
        vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34};

        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalMult(ct, two_ct);
            // Fibonacci number of ZANS rounds
            int zans_rounds = (i < (int)fib.size()) ? fib[i] : fib.back();
            for(int j = 0; j < zans_rounds; j++) {
                ct = cc->EvalAdd(ct, anchor0);
            }
            steps++;
            val *= 2;

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n";
    }

    // ============================================
    // STRATEGY 5: Scalar Decomp after every UK×UK
    // ============================================
    cout << "=== STRATEGY 5: Scalar Decomp Reset After Each UK×UK ===\n";
    {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
        int64_t val = 2;
        int steps = 0;
        bool ok = true;

        for(int i = 0; i < 35 && ok; i++) {
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // Scalar decomp reset: multiply by 1 (add ct to itself 1 time)
            auto reset = zero_ct;
            reset = cc->EvalAdd(reset, ct);
            reset = cc->EvalAdd(reset, anchor0);
            ct = reset;
            
            steps++;
            val *= 2;

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            if(pt->GetPackedValue()[0] != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n";
    }

    // ============================================
    // STRATEGY 6: UK×UK + Full Scalar Decomp Chain
    // ============================================
    cout << "=== STRATEGY 6: UK×UK + Scalar Decomp ×2 Chain ===\n";
    {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
        int64_t val = 2;
        int steps = 0;
        bool ok = true;

        for(int i = 0; i < 35 && ok; i++) {
            // UK×UK
            ct = cc->EvalMult(ct, two_ct);
            ct = cc->EvalAdd(ct, anchor0);
            
            // Full scalar decomp: add ct to itself (×2) with ZANS
            auto decomp = zero_ct;
            for(int j = 0; j < 2; j++) {
                decomp = cc->EvalAdd(decomp, ct);
                decomp = cc->EvalAdd(decomp, anchor0);
            }
            ct = decomp;
            
            steps++;
            val *= 2;

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            int64_t dec_val = pt->GetPackedValue()[0];
            if(dec_val != val) ok = false;
        }
        cout << "  Steps: " << (ok ? steps : steps-1) << " | Noise: " << ct->GetNoiseScaleDeg() << "\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — UK×UK DEEP TEST COMPLETE               ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
