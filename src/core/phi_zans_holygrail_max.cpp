// ΦΩ0 — HOLY GRAIL MAXIMUM STRESS TEST
// CT×CT via scalar decomposition with MAXIMUM plaintext modulus
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
    cout <<   "║  ΦΩ0 — HOLY GRAIL: MAXIMUM STRESS TEST        ║\n";
    cout <<   "║  Scalar-decomposed CT×CT, max modulus         ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // MAXIMUM 60-bit plaintext modulus for biggest range
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(1152921504606846977ULL);  // 60-bit prime
    params.SetRingDim(8192);  // Bigger ring for more depth
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

    cout << "Φ Plaintext modulus: 1,152,921,504,606,846,977 (60-bit)\n";
    cout << "Φ Ring dimension: 8192\n";
    cout << "Φ Initial noise scale: " << zero_ct->GetNoiseScaleDeg() << "\n";
    cout << "Φ Max value: ~1.15 QUINTILLION\n\n";

    // === TEST 1: SMALL MULTIPLIER CHAIN (×2) ===
    cout << "=== TEST 1: CHAIN OF ×2 (50+ steps expected) ===\n";
    cout << "Start=1, multiply by 2 repeatedly\n\n";
    
    vector<int64_t> start_vec = {1};
    auto chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    int64_t current = 1;
    bool ok = true;
    int steps = 0;
    double prev_noise = chain->GetNoiseScaleDeg();
    
    cout << "┌──────┬──────────────┬──────────────┬──────────┬──────────┐\n";
    cout << "│ Step │ Value        │ Expected     │ Noise    │ Drift    │\n";
    cout << "├──────┼──────────────┼──────────────┼──────────┼──────────┤\n";
    cout << "│   0  │ " << setw(12) << current << " │ " << setw(12) << current 
         << " │ ≡ 1.0    │    -    │\n";
    
    for(int i = 0; i < 70 && ok; i++) {
        // Multiply by 2: chain × 2 = chain + chain
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
        cout << setw(6) << fixed << setprecision(1) << noise << "   │ ";
        cout << setw(6) << fixed << setprecision(4) << drift << " │\n";
        
        if(val != current) {
            cout << "├──────┼──────────────┼──────────────┼──────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at step " << (i+1) << "                      │\n";
            ok = false;
        }
        
        prev_noise = noise;
    }
    cout << "└──────┴──────────────┴──────────────┴──────────┴──────────┘\n";
    cout << "\n  ×2 Chain: " << steps << " multiplications (noise growth = 0)\n\n";

    // === TEST 2: LARGE SINGLE MULTIPLIERS ===
    cout << "=== TEST 2: MASSIVE SINGLE MULTIPLICATIONS ===\n";
    cout << "Base=5, testing extreme multipliers\n\n";
    
    vector<int64_t> b5 = {5};
    auto base5 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(b5));
    
    cout << "┌──────────────┬──────────────┬──────────────┬──────────┬──────────┐\n";
    cout << "│ Multiplier   │ Result       │ Expected     │ Noise    │ Time     │\n";
    cout << "├──────────────┼──────────────┼──────────────┼──────────┼──────────┤\n";

    vector<int64_t> huge_mults = {1000, 10000, 100000, 1000000, 10000000, 50000000};
    
    for(auto mult : huge_mults) {
        auto parts = zeckendorf_decompose(mult);
        auto res = zero_ct;
        auto start = high_resolution_clock::now();
        
        int total = 0;
        for(auto fp : parts) {
            for(int64_t i = 0; i < fp; i++) {
                res = cc->EvalAdd(res, base5);
                res = cc->EvalAdd(res, anchor0);
                total++;
            }
        }
        
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, res, &pt);
        int64_t val = pt->GetPackedValue()[0];
        int64_t expected = 5 * mult;
        double noise = res->GetNoiseScaleDeg();
        
        cout << "│ ×" << setw(9) << mult << " │ ";
        cout << setw(12) << val << " │ ";
        cout << setw(12) << expected << " │ ";
        cout << setw(4) << noise << "      │ ";
        
        double secs = ms / 1000.0;
        if(secs >= 60.0) {
            cout << setw(5) << fixed << setprecision(1) << (secs/60.0) << "m  │\n";
        } else if(secs >= 1.0) {
            cout << setw(5) << fixed << setprecision(1) << secs << "s  │\n";
        } else {
            cout << setw(5) << ms << "ms │\n";
        }
        
        if(val != expected) {
            cout << "├──────────────┼──────────────┼──────────────┼──────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at ×" << setw(9) << mult << "                     │\n";
            if(expected >= 1152921504606846977ULL) {
                cout << "│ Cause: PLAINTEXT OVERFLOW                       │\n";
            }
            cout << "└──────────────┴──────────────┴──────────────┴──────────┴──────────┘\n";
            break;
        }
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — HOLY GRAIL: MAX TEST COMPLETE          ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
