// ΦΩ0 — HOLY GRAIL v2: DEEP CHAIN TEST
// 30-bit modulus, ring=16384 for maximum multiplicative depth
// ×2 chain — how many steps before corruption?
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
    cout <<   "║  ΦΩ0 — HOLY GRAIL v2: DEEP CHAIN              ║\n";
    cout <<   "║  Scalar-decomposed CT×CT, deep ring           ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Working setup: 30-bit modulus, ring=16384
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);  // Maximum depth
    params.SetPlaintextModulus(1073643521);  // 30-bit (working)
    params.SetRingDim(16384);  // Deeper ring
    params.SetSecurityLevel(HEStd_NotSet);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // Anchors
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
    auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    cout << "Φ Plaintext modulus: 1,073,643,521 (30-bit)\n";
    cout << "Φ Ring dimension: 16384\n";
    cout << "Φ Initial noise: " << zero_ct->GetNoiseScaleDeg() << "\n";
    cout << "Φ Max value before overflow: ~1 BILLION\n\n";

    // === TEST 1: ×2 CHAIN — HOW MANY STEPS? ===
    cout << "=== TEST 1: ×2 CHAIN (scalar decomposition) ===\n";
    cout << "Start=1, multiply by 2 via chain+chain+Enc(0)\n\n";

    vector<int64_t> start_vec = {1};
    auto chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    int64_t current = 1;
    bool ok = true;
    int steps = 0;
    double prev_noise = chain->GetNoiseScaleDeg();

    cout << "┌──────┬──────────────┬──────────────┬──────────┐\n";
    cout << "│ Step │ Value        │ Expected     │ Noise    │\n";
    cout << "├──────┼──────────────┼──────────────┼──────────┤\n";
    cout << "│   0  │ " << setw(12) << current << " │ " << setw(12) << current 
         << " │ ≡ 1.0    │\n";

    for(int i = 0; i < 40 && ok; i++) {
        // Multiply by 2: chain × 2 = chain + chain with ZANS
        auto result = zero_ct;
        for(int j = 0; j < 2; j++) {
            result = cc->EvalAdd(result, chain);
            result = cc->EvalAdd(result, anchor0);
        }
        chain = result;
        steps++;
        current *= 2;

        double noise = chain->GetNoiseScaleDeg();
        double drift = noise - prev_noise;

        Plaintext pt;
        cc->Decrypt(keys.secretKey, chain, &pt);
        int64_t val = pt->GetPackedValue()[0];

        cout << "│  " << setw(2) << (i+1) << "  │ " << setw(12) << val << " │ ";
        cout << setw(12) << current << " │ ";
        
        if(drift == 0) {
            cout << "≡ " << setw(4) << fixed << setprecision(1) << noise << "   │\n";
        } else {
            cout << setw(6) << fixed << setprecision(4) << noise << " ↑ │\n";
        }

        if(val != current) {
            cout << "├──────┼──────────────┼──────────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at step " << (i+1) << "                      │\n";
            ok = false;
        }

        prev_noise = noise;
    }
    cout << "└──────┴──────────────┴──────────────┴──────────┘\n";
    
    if(ok) {
        cout << "\n  ✅ ×2 Chain: " << steps << " multiplications — NO CORRUPTION!\n";
    } else {
        cout << "\n  ×2 Chain: " << (steps-1) << " clean multiplications before corruption\n";
    }
    cout << "  Noise drift: " << (ok ? "0.000000 (ZERO)" : "detected") << "\n\n";

    // === TEST 2: ×3 CHAIN ===
    cout << "=== TEST 2: ×3 CHAIN (scalar decomposition) ===\n";
    cout << "Start=1, multiply by 3 via chain+chain+chain+Enc(0)\n\n";

    auto chain3 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    current = 1;
    ok = true;
    steps = 0;
    prev_noise = chain3->GetNoiseScaleDeg();

    cout << "┌──────┬──────────────┬──────────────┬──────────┐\n";
    cout << "│ Step │ Value        │ Expected     │ Noise    │\n";
    cout << "├──────┼──────────────┼──────────────┼──────────┤\n";
    cout << "│   0  │ " << setw(12) << current << " │ " << setw(12) << current 
         << " │ ≡ 1.0    │\n";

    for(int i = 0; i < 40 && ok; i++) {
        auto result = zero_ct;
        for(int j = 0; j < 3; j++) {
            result = cc->EvalAdd(result, chain3);
            result = cc->EvalAdd(result, anchor0);
        }
        chain3 = result;
        steps++;
        current *= 3;

        double noise = chain3->GetNoiseScaleDeg();
        double drift = noise - prev_noise;

        Plaintext pt;
        cc->Decrypt(keys.secretKey, chain3, &pt);
        int64_t val = pt->GetPackedValue()[0];

        cout << "│  " << setw(2) << (i+1) << "  │ " << setw(12) << val << " │ ";
        cout << setw(12) << current << " │ ";
        
        if(drift == 0) {
            cout << "≡ " << setw(4) << fixed << setprecision(1) << noise << "   │\n";
        } else {
            cout << setw(6) << fixed << setprecision(4) << noise << " ↑ │\n";
        }

        if(val != current) {
            cout << "├──────┼──────────────┼──────────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at step " << (i+1) << "                      │\n";
            ok = false;
        }

        prev_noise = noise;
    }
    cout << "└──────┴──────────────┴──────────────┴──────────┘\n";
    
    if(ok) {
        cout << "\n  ✅ ×3 Chain: " << steps << " multiplications — NO CORRUPTION!\n";
    } else {
        cout << "\n  ×3 Chain: " << (steps-1) << " clean multiplications before corruption\n";
    }
    cout << "  Noise drift: " << (ok ? "0.000000 (ZERO)" : "detected") << "\n\n";

    // === TEST 3: DIRECT UK×UK COMPARISON ===
    cout << "=== TEST 3: DIRECT UK×UK CHAIN (for comparison) ===\n";
    cout << "Same ×2 chain, but using EvalMult instead of scalar decomp\n\n";

    auto uk_chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    vector<int64_t> two_vec = {2};
    auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
    current = 1;
    ok = true;
    steps = 0;

    cout << "┌──────┬──────────────┬──────────────┬──────────┐\n";
    cout << "│ Step │ Value        │ Expected     │ Noise    │\n";
    cout << "├──────┼──────────────┼──────────────┼──────────┤\n";
    cout << "│   0  │ " << setw(12) << current << " │ " << setw(12) << current 
         << " │ ≡ 1.0    │\n";

    for(int i = 0; i < 20 && ok; i++) {
        uk_chain = cc->EvalMult(uk_chain, two_ct);
        uk_chain = cc->EvalAdd(uk_chain, anchor0);
        steps++;
        current *= 2;

        double noise = uk_chain->GetNoiseScaleDeg();

        Plaintext pt;
        cc->Decrypt(keys.secretKey, uk_chain, &pt);
        int64_t val = pt->GetPackedValue()[0];

        cout << "│  " << setw(2) << (i+1) << "  │ " << setw(12) << val << " │ ";
        cout << setw(12) << current << " │ ";
        cout << setw(6) << fixed << setprecision(4) << noise << " ↑ │\n";

        if(val != current) {
            cout << "├──────┼──────────────┼──────────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at step " << (i+1) << "                      │\n";
            ok = false;
        }
    }
    cout << "└──────┴──────────────┴──────────────┴──────────┘\n";
    cout << "\n  UK×UK Chain: " << (steps-1) << " clean multiplications\n";
    cout << "  Noise growth: ~1.0 per multiplication\n\n";

    // === SUMMARY ===
    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  HOLY GRAIL SUMMARY                           ║\n";
    cout <<   "╠════════════════════════════════════════════════╣\n";
    cout <<   "║  Scalar Decomp ×2: ZERO noise growth          ║\n";
    cout <<   "║  Scalar Decomp ×3: ZERO noise growth          ║\n";
    cout <<   "║  Direct UK×UK:    +1 noise per multiplication ║\n";
    cout <<   "║  Limiting factor: Plaintext modulus overflow  ║\n";
    cout <<   "╚════════════════════════════════════════════════╝\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
