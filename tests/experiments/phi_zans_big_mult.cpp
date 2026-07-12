// ΦΩ0 — MASSIVE MULTIPLICATION WITH LARGE PLAINTEXT MODULUS
// Fixed: 30-bit plaintext modulus to prevent overflow
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
    cout <<   "║  ΦΩ0 — MASSIVE MULT WITH LARGE MODULUS        ║\n";
    cout <<   "║  Fixed overflow: plaintextModulus = 1073643521 ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // USE LARGE PLAINTEXT MODULUS (30-bit)
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(1073643521);  // 30-bit prime! ~1 billion
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

    cout << "Φ Plaintext modulus: 1,073,643,521 (30 bits)\n";
    cout << "Φ Initial noise scale: " << zero_ct->GetNoiseScaleDeg() << "\n";
    cout << "Φ Max value before overflow: ~1 BILLION\n\n";

    // === MASSIVE MULTIPLIER TEST ===
    cout << "=== MASSIVE SCALAR MULTIPLICATION (Base=7) ===\n\n";

    vector<int64_t> b7 = {7};
    auto base7 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(b7));

    cout << "┌──────────────┬──────────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Multiplier   │ Result       │ Expected │ Noise    │ Time     │\n";
    cout << "├──────────────┼──────────────┼──────────┼──────────┼──────────┤\n";

    vector<int64_t> massive_mults = {10, 100, 1000, 10000, 100000, 1000000, 10000000};

    for(auto mult : massive_mults) {
        auto parts = zeckendorf_decompose(mult);
        auto res = zero_ct;
        auto start = high_resolution_clock::now();
        
        for(auto fp : parts) {
            for(int64_t i = 0; i < fp; i++) {
                res = cc->EvalAdd(res, base7);
                res = cc->EvalAdd(res, anchor0);
            }
        }
        
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, res, &pt);
        int64_t val = pt->GetPackedValue()[0];
        int64_t expected = 7 * mult;
        double noise = res->GetNoiseScaleDeg();
        
        // Format for display
        cout << "│ ×" << setw(9) << mult << " │ ";
        cout << setw(10) << val << "   │ ";
        cout << setw(8) << expected << " │ ";
        cout << "≡ " << setw(4) << noise << "   │ ";
        
        double seconds = ms / 1000.0;
        if(seconds >= 1.0) {
            cout << setw(5) << fixed << setprecision(1) << seconds << "s  │\n";
        } else {
            cout << setw(5) << ms << "ms │\n";
        }
        
        if(val != expected) {
            cout << "├──────────────┼──────────────┼──────────┼──────────┼──────────┤\n";
            cout << "│ ❌ CORRUPTED at ×" << setw(9) << mult << "                     │\n";
            cout << "└──────────────┴──────────────┴──────────┴──────────┴──────────┘\n\n";
            
            // Check if it's plaintext overflow
            if(expected >= 1073643521) {
                cout << "  Cause: PLAINTEXT OVERFLOW (expected " << expected << " > " << 1073643521 << ")\n";
            } else {
                cout << "  Cause: UNKNOWN (expected " << expected << " < modulus " << 1073643521 << ")\n";
            }
            break;
        }
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
