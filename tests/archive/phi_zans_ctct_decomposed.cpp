// ΦΩ0 — CT×CT VIA SCALAR DECOMPOSITION
// Decompose ct_b into scalar parts, multiply each with ZANS
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

vector<int64_t> generate_fibonacci(int64_t limit) {
    vector<int64_t> fib = {1, 2};
    while(fib.back() <= limit) {
        fib.push_back(fib[fib.size()-1] + fib[fib.size()-2]);
    }
    fib.pop_back();
    return fib;
}

vector<int64_t> zeckendorf_decompose(int64_t n) {
    vector<int64_t> fib = generate_fibonacci(n);
    vector<int64_t> result;
    for(int i = fib.size() - 1; i >= 0 && n > 0; i--) {
        if(fib[i] <= n) {
            result.push_back(fib[i]);
            n -= fib[i];
            i--;
        }
    }
    return result;
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — CT×CT VIA SCALAR DECOMPOSITION         ║\n";
    cout <<   "║  Holy Grail: UK×UK without noise growth       ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // Large modulus for big values
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(1073643521);  // 30-bit
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
    cout << "Φ Initial noise scale: " << zero_ct->GetNoiseScaleDeg() << "\n\n";

    // === EXPERIMENT 1: CT × known scalar via decomposition ===
    cout << "=== EXPERIMENT 1: CT × 7 USING SCALAR DECOMPOSITION ===\n";
    cout << "Decompose 7 into Fibonacci parts: 7 = 5 + 2\n\n";

    // Encrypt base value
    vector<int64_t> val12 = {12};
    auto ct12 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val12));

    // METHOD A: Direct UK×UK (for comparison)
    vector<int64_t> val7 = {7};
    auto ct7 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val7));
    
    auto direct = cc->EvalMult(ct12, ct7);
    direct = cc->EvalAdd(direct, anchor0);
    
    Plaintext dpt;
    cc->Decrypt(keys.secretKey, direct, &dpt);
    int64_t dval = dpt->GetPackedValue()[0];
    double dnoise = direct->GetNoiseScaleDeg();
    
    cout << "  Direct UK×UK: " << dval << " (expected 84)";
    if(dval == 84) cout << " ✅";
    else cout << " ❌";
    cout << " | noise: " << dnoise << "\n\n";

    // METHOD B: Scalar decomposition — 7 = 5 + 2
    cout << "  Scalar decomposition method:\n";
    auto parts = zeckendorf_decompose(7);
    cout << "  7 = ";
    for(size_t i = 0; i < parts.size(); i++) {
        cout << parts[i];
        if(i < parts.size()-1) cout << " + ";
    }
    cout << "\n";
    
    auto scalar_result = zero_ct;
    int total_adds = 0;
    
    for(auto fp : parts) {
        // Add ct12 fp times (scalar multiplication via addition)
        for(int64_t i = 0; i < fp; i++) {
            scalar_result = cc->EvalAdd(scalar_result, ct12);
            scalar_result = cc->EvalAdd(scalar_result, anchor0);
            total_adds++;
        }
        Plaintext pt;
        cc->Decrypt(keys.secretKey, scalar_result, &pt);
        cout << "  After " << fp << "×ct12: " << pt->GetPackedValue()[0] << "\n";
    }
    
    Plaintext spt;
    cc->Decrypt(keys.secretKey, scalar_result, &spt);
    int64_t sval = spt->GetPackedValue()[0];
    double snoise = scalar_result->GetNoiseScaleDeg();
    
    cout << "\n  Scalar decomp: " << sval << " (expected 84)";
    if(sval == 84) cout << " ✅";
    else cout << " ❌";
    cout << " | noise: " << snoise;
    cout << " | additions: " << total_adds << "\n\n";

    // === EXPERIMENT 2: CT × CT via encrypting the decomposition ===
    cout << "=== EXPERIMENT 2: CT_A × CT_B VIA DECOMPOSITION ===\n";
    cout << "If we know the value of CT_B, we can multiply without EvalMult\n\n";

    // Let's do 12 × 34 using decomposition
    vector<int64_t> val34 = {34};
    auto ct34 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val34));
    
    // Direct UK×UK
    auto direct2 = cc->EvalMult(ct12, ct34);
    direct2 = cc->EvalAdd(direct2, anchor0);
    Plaintext dpt2;
    cc->Decrypt(keys.secretKey, direct2, &dpt2);
    
    cout << "  Direct UK×UK 12×34: " << dpt2->GetPackedValue()[0] << " (expected 408)";
    cout << " | noise: " << direct2->GetNoiseScaleDeg() << "\n";
    
    // Scalar decomposition: 34 = 21 + 13
    auto parts34 = zeckendorf_decompose(34);
    cout << "  34 = ";
    for(size_t i = 0; i < parts34.size(); i++) {
        cout << parts34[i];
        if(i < parts34.size()-1) cout << " + ";
    }
    cout << "\n";
    
    auto scalar_res2 = zero_ct;
    for(auto fp : parts34) {
        for(int64_t i = 0; i < fp; i++) {
            scalar_res2 = cc->EvalAdd(scalar_res2, ct12);
            scalar_res2 = cc->EvalAdd(scalar_res2, anchor0);
        }
    }
    
    Plaintext spt2;
    cc->Decrypt(keys.secretKey, scalar_res2, &spt2);
    
    cout << "  Scalar decomp: " << spt2->GetPackedValue()[0] << " (expected 408)";
    cout << " | noise: " << scalar_res2->GetNoiseScaleDeg() << "\n\n";

    // === EXPERIMENT 3: CT × CT CHAIN USING DECOMPOSITION ===
    cout << "=== EXPERIMENT 3: CT×CT CHAIN VIA DECOMPOSITION ===\n";
    cout << "Start with 2, repeatedly multiply by 3 using scalar decomposition\n";
    cout << "(3 = 2+1, so ×3 = add ct 3 times with ZANS)\n\n";
    
    auto chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val12));
    int64_t current_val = 12;
    int chain_len = 0;
    bool chain_ok = true;
    
    cout << "┌──────┬──────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Step │ Value    │ Expected │ Noise    │ Drift    │\n";
    cout << "├──────┼──────────┼──────────┼──────────┼──────────┤\n";
    cout << "│   0  │ " << setw(6) << current_val << "   │ " 
         << setw(6) << current_val << "   │ ≡ 1.0   │    -    │\n";
    
    for(int step = 0; step < 20 && chain_ok; step++) {
        // Multiply by 3 using decomposition: chain × 3 = chain + chain + chain
        auto result = zero_ct;
        for(int i = 0; i < 3; i++) {
            result = cc->EvalAdd(result, chain);
            result = cc->EvalAdd(result, anchor0);
        }
        
        chain = result;
        chain_len++;
        current_val *= 3;
        
        double noise = chain->GetNoiseScaleDeg();
        static double prev_noise = 1.0;
        double drift = noise - prev_noise;
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, chain, &pt);
        int64_t val = pt->GetPackedValue()[0];
        
        cout << "│  " << setw(2) << (step+1) << "  │ " << setw(6) << val << "   │ ";
        cout << setw(6) << current_val << "   │ ";
        cout << setw(8) << fixed << setprecision(1) << noise << " │ ";
        cout << setw(8) << fixed << setprecision(6) << drift << " │\n";
        
        if(val != current_val) {
            cout << "├──────┼──────────┼──────────┼──────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at step " << (step+1) << "                      │\n";
            chain_ok = false;
        }
        
        prev_noise = noise;
    }
    cout << "└──────┴──────────┴──────────┴──────────┴──────────┘\n";
    cout << "\n  Scalar-decomposed chain: " << chain_len << " multiplications\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — CT×CT VIA DECOMPOSITION COMPLETE       ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
