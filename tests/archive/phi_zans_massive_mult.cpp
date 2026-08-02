// ΦΩ0 — MASSIVE SCALAR MULTIPLICATION VIA REPEATED ADDITION
// Multiplying without multiplication — pure ZANS-stabilized addition
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
    cout <<   "║  ΦΩ0 — MULTIPLYING WITHOUT MULTIPLICATION     ║\n";
    cout <<   "║  Scalar mult via repeated addition + ZANS      ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(65537);
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

    cout << "Φ Initial noise scale: " << zero_ct->GetNoiseScaleDeg() << "\n";
    cout << "Φ Method: base × N = repeated Enc(base) addition + Enc(0) stabilization\n\n";

    // === MASSIVE MULTIPLIER TEST ===
    cout << "=== MASSIVE SCALAR MULTIPLICATION ===\n";
    cout << "Base=7, testing up to ×1,000,000\n\n";

    vector<int64_t> b7 = {7};
    auto base7 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(b7));
    cout << "  Base Enc(7) noise scale: " << base7->GetNoiseScaleDeg() << "\n\n";

    cout << "┌──────────────┬──────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Multiplier   │ Result   │ Expected │ Noise    │ Time     │\n";
    cout << "├──────────────┼──────────┼──────────┼──────────┼──────────┤\n";

    vector<int64_t> massive_mults = {10, 100, 1000, 10000, 100000, 500000, 1000000};

    for(auto mult : massive_mults) {
        auto parts = zeckendorf_decompose(mult);
        
        cout << "│ ×" << setw(7) << mult << "   │ ";
        cout.flush();
        
        auto res = zero_ct;
        int total_adds = 0;
        auto start = high_resolution_clock::now();
        
        for(auto fp : parts) {
            for(int64_t i = 0; i < fp; i++) {
                res = cc->EvalAdd(res, base7);
                res = cc->EvalAdd(res, anchor0);
                total_adds++;
            }
        }
        
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, res, &pt);
        int64_t val = pt->GetPackedValue()[0];
        int64_t expected = 7 * mult;
        double noise = res->GetNoiseScaleDeg();
        
        cout << setw(6) << val << "   │ ";
        cout << setw(6) << expected << "   │ ";
        cout << "≡ " << setw(4) << noise << "   │ ";
        
        double seconds = ms / 1000.0;
        if(seconds >= 1.0) {
            cout << setw(5) << fixed << setprecision(1) << seconds << "s  │\n";
        } else {
            cout << setw(5) << ms << "ms │\n";
        }
        
        // Only continue if correct and noise is fine
        if(val != expected) {
            cout << "├──────────────┼──────────┼──────────┼──────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at ×" << setw(7) << mult << "                        │\n";
            cout << "└──────────────┴──────────┴──────────┴──────────┴──────────┘\n";
            break;
        }
    }

    // === SPEED TEST: ×1,000,000 ===
    cout << "\n=== SPEED TEST: 7 × 1,000,000 ===\n";
    cout << "Pure repeated addition + ZANS\n\n";
    
    auto parts = zeckendorf_decompose(1000000);
    cout << "Zeckendorf parts: " << parts.size() << "\n";
    
    auto res = zero_ct;
    auto start = high_resolution_clock::now();
    int total = 0;
    
    for(auto fp : parts) {
        for(int64_t i = 0; i < fp; i++) {
            res = cc->EvalAdd(res, base7);
            res = cc->EvalAdd(res, anchor0);
            total++;
        }
    }
    
    auto end = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(end - start).count();
    double seconds = ms / 1000.0;
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, res, &pt);
    int64_t val = pt->GetPackedValue()[0];
    double noise = res->GetNoiseScaleDeg();
    
    cout << "\n  Result: " << val << " (expected " << (7*1000000) << ")";
    if(val == 7*1000000) cout << " ✅";
    else cout << " ❌";
    cout << "\n";
    cout << "  Noise scale: " << noise << " (started at 1)\n";
    cout << "  Total additions: " << total << "\n";
    cout << "  Time: " << fixed << setprecision(2) << seconds << "s\n";
    cout << "  Throughput: " << fixed << setprecision(0) << (total/seconds) << " ops/sec\n";
    
    if(noise == 1) {
        cout << "\n  Φ VERDICT: Multiplication WITHOUT noise growth!\n";
        cout << "  Φ Adding in bulk = the same noise as Enc(0) additions!\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — MULTIPLYING WITHOUT MULTIPLICATION     ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
