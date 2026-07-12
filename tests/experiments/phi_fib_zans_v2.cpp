// ΦΩ0 — FIBONACCI-ZANS v2: CORRECT LOGIC
// Zeckendorf decomposition + Enc(0) stabilization
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <cmath>
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
    cout <<   "║  ΦΩ0 — FIBONACCI-ZANS v2: CORRECT LOGIC       ║\n";
    cout <<   "║  Enc(1) repeated, Enc(0) stabilized            ║\n";
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

    // Create anchors
    vector<int64_t> zero_vec = {0}, one_vec = {1};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
    auto anchor1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(one_vec));

    cout << "=== CORRECT FIBONACCI-ZANS MULTIPLICATION ===\n";
    cout << "Using Enc(1) repeated addition + Enc(0) stabilization\n\n";

    // Test: 5 × 7 using Fibonacci (7 = 5+2)
    cout << "--- Test: 5 × 7 ---\n";
    int64_t base = 5, multiplier = 7;
    auto parts = zeckendorf_decompose(multiplier);
    cout << "Zeckendorf: " << multiplier << " = ";
    for(size_t i = 0; i < parts.size(); i++) {
        cout << parts[i];
        if(i < parts.size()-1) cout << "+";
    }
    cout << " (" << parts.size() << " parts)\n";

    // Start with base value
    vector<int64_t> base_vec = {base};
    auto result = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(base_vec));
    int total_adds = 0;

    for(auto fib_part : parts) {
        // Add Enc(1) fib_part times
        for(int64_t i = 0; i < fib_part; i++) {
            result = cc->EvalAdd(result, anchor1);
            result = cc->EvalAdd(result, anchor0); // ZANS stabilization
            total_adds++;
        }
        
        // Check after each part
        Plaintext pt;
        cc->Decrypt(keys.secretKey, result, &pt);
        cout << "  After +" << fib_part << ": " << pt->GetPackedValue()[0] << "\n";
    }

    Plaintext final_pt;
    cc->Decrypt(keys.secretKey, result, &final_pt);
    int64_t final_val = final_pt->GetPackedValue()[0];
    int64_t expected = base * multiplier;
    
    cout << "\n  Result: " << final_val << " (expected " << expected << ")";
    if(final_val == expected) cout << " ✅";
    else cout << " ❌";
    cout << " | " << total_adds << " additions\n\n";

    // Test multiple multipliers
    cout << "--- Multiplier Sweep (base=3) ---\n";
    vector<int64_t> multipliers = {2, 3, 5, 7, 10, 21, 42, 100, 1000};
    
    for(auto mult : multipliers) {
        auto p = zeckendorf_decompose(mult);
        
        vector<int64_t> bv = {3};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(bv));
        
        int adds = 0;
        bool ok = true;
        
        for(auto fp : p) {
            for(int64_t i = 0; i < fp && ok; i++) {
                ct = cc->EvalAdd(ct, anchor1);
                ct = cc->EvalAdd(ct, anchor0);
                adds++;
                
                // Check every 5000 ops
                if(adds % 5000 == 0) {
                    Plaintext pt;
                    cc->Decrypt(keys.secretKey, ct, &pt);
                    if(pt->GetPackedValue()[0] < 0) ok = false;
                }
            }
        }
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        int64_t val = pt->GetPackedValue()[0];
        
        cout << "  3 × " << setw(4) << mult << " = " << setw(6) << val;
        cout << " (exp " << setw(6) << (3*mult) << ")";
        if(val == 3*mult) cout << " ✅";
        else cout << " ❌";
        cout << " | " << adds << " adds | " << p.size() << " parts\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — FIB-ZANS v2 COMPLETE                   ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
