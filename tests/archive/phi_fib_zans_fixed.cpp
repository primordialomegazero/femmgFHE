// ΦΩ0 — FIBONACCI-ZANS FIXED v1
// Hypothesis: Enc(0) stabilization between Fibonacci additions prevents corruption
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
    cout <<   "║  ΦΩ0 — FIBONACCI-ZANS FIXED v1               ║\n";
    cout <<   "║  Enc(0) Stabilization Between Fib Parts       ║\n";
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
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    cout << "=== COMPARISON: OLD vs FIXED FIB-ZANS ===\n\n";

    // === OLD METHOD: No Enc(0) stabilization ===
    cout << "OLD METHOD (no Enc(0) stabilization):\n";
    vector<int64_t> test_multipliers = {5, 21, 42, 100};
    
    for(auto mult : test_multipliers) {
        auto parts = zeckendorf_decompose(mult);
        
        vector<int64_t> val_vec = {1};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val_vec));
        
        bool survived = true;
        int64_t result = 0;
        
        for(auto fib_part : parts) {
            vector<int64_t> fib_vec = {fib_part};
            auto fib_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(fib_vec));
            
            for(int64_t i = 0; i < fib_part && survived; i++) {
                ct = cc->EvalAdd(ct, fib_ct);
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt);
                if(pt->GetPackedValue()[0] < 0) {
                    survived = false;
                    result = pt->GetPackedValue()[0];
                }
            }
        }
        
        if(survived) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            result = pt->GetPackedValue()[0];
        }
        
        cout << "  ×" << mult << ": " << result;
        cout << " (expected " << (1 * mult) << ")";
        if(result == mult) cout << " ✅\n";
        else cout << " ❌ CORRUPTED\n";
    }

    // === FIXED METHOD: Enc(0) stabilization between parts ===
    cout << "\nFIXED METHOD (Enc(0) stabilization between parts):\n";
    
    for(auto mult : test_multipliers) {
        auto parts = zeckendorf_decompose(mult);
        
        vector<int64_t> val_vec = {1};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val_vec));
        
        bool survived = true;
        int64_t result = 0;
        
        for(auto fib_part : parts) {
            vector<int64_t> fib_vec = {fib_part};
            auto fib_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(fib_vec));
            
            for(int64_t i = 0; i < fib_part && survived; i++) {
                ct = cc->EvalAdd(ct, fib_ct);
                ct = cc->EvalAdd(ct, anchor0); // ZANS stabilization
                
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt);
                if(pt->GetPackedValue()[0] < 0) {
                    survived = false;
                    result = pt->GetPackedValue()[0];
                }
            }
        }
        
        if(survived) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            result = pt->GetPackedValue()[0];
        }
        
        cout << "  ×" << mult << ": " << result;
        cout << " (expected " << (1 * mult) << ")";
        if(result == mult) cout << " ✅\n";
        else cout << " ❌ CORRUPTED\n";
    }

    // === STRESS TEST: Large multiplier with fixed method ===
    cout << "\n=== FIXED METHOD STRESS TEST ===\n";
    vector<int64_t> large_multipliers = {100, 1000, 10000};
    
    for(auto mult : large_multipliers) {
        auto parts = zeckendorf_decompose(mult);
        cout << "  ×" << mult << " = ";
        for(size_t i = 0; i < parts.size(); i++) {
            cout << parts[i];
            if(i < parts.size()-1) cout << "+";
        }
        cout << " (" << parts.size() << " parts)\n";
        
        vector<int64_t> val_vec = {5};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val_vec));
        
        bool survived = true;
        int64_t result = 0;
        int total_additions = 0;
        
        auto start = high_resolution_clock::now();
        
        for(auto fib_part : parts) {
            vector<int64_t> fib_vec = {fib_part};
            auto fib_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(fib_vec));
            
            for(int64_t i = 0; i < fib_part && survived; i++) {
                ct = cc->EvalAdd(ct, fib_ct);
                ct = cc->EvalAdd(ct, anchor0); // FIX: ZANS stabilization
                total_additions++;
                
                if(total_additions % 1000 == 0) {
                    Plaintext pt;
                    cc->Decrypt(keys.secretKey, ct, &pt);
                    if(pt->GetPackedValue()[0] < 0) {
                        survived = false;
                        result = pt->GetPackedValue()[0];
                    }
                }
            }
        }
        
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        
        if(survived) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            result = pt->GetPackedValue()[0];
        }
        
        cout << "    Result: " << result;
        cout << " (expected " << (5 * mult) << ")";
        if(result == 5 * mult) cout << " ✅";
        else cout << " ❌ CORRUPTED";
        cout << " | " << total_additions << " adds | " << ms << "ms\n";
    }

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — FIB-ZANS FIXED TEST COMPLETE           ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
