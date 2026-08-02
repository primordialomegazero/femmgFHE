// ΦΩ0 — ZANS FLOOR INVESTIGATION
// Testing: Different anchors, different plaintext moduli, different φ values
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace lbcrypto;
using namespace std;

// Measure noise using GetNoise() or estimate via decryption precision
double measure_noise(CryptoContext<DCRTPoly>& cc, 
                     const Ciphertext<DCRTPoly>& ct,
                     const PrivateKey<DCRTPoly>& sk) {
    Plaintext pt;
    cc->Decrypt(sk, ct, &pt);
    // Rough noise estimate: check if value still correct
    return pt->GetPackedValue()[0];
}

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — ZANS FLOOR INVESTIGATION               ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // === EXPERIMENT 1: Different Anchor Values ===
    cout << "=== EXPERIMENT 1: ANCHOR VALUE SWEEP ===\n";
    cout << "Testing Enc(k) for k = 0, 1, 2, 3, 5, 8, 13, 21, 42\n\n";

    vector<int64_t> anchor_values = {0, 1, 2, 3, 5, 8, 13, 21, 42};
    
    for(auto anchor_val : anchor_values) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(1);
        params.SetPlaintextModulus(65537);
        params.SetSecurityLevel(HEStd_NotSet);
        
        CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        // Create anchor
        vector<int64_t> anchor_vec = {anchor_val};
        Plaintext anchor_pt = cc->MakePackedPlaintext(anchor_vec);
        auto anchor = cc->Encrypt(keys.publicKey, anchor_pt);
        
        // Create test value
        vector<int64_t> val_vec = {42};
        Plaintext val_pt = cc->MakePackedPlaintext(val_vec);
        auto ct = cc->Encrypt(keys.publicKey, val_pt);
        
        // Run 1000 ZANS ops
        bool survived = true;
        int64_t last_val = 42;
        for(int i = 0; i < 1000; i++) {
            ct = cc->EvalAdd(ct, anchor);
            
            if(i == 999) {
                Plaintext check_pt;
                cc->Decrypt(keys.secretKey, ct, &check_pt);
                last_val = check_pt->GetPackedValue()[0];
            }
        }
        
        int64_t expected = 42 + (anchor_val * 1000);
        cout << "  Enc(" << anchor_val << "): ";
        cout << "value=" << last_val;
        cout << " (expected " << expected << ")";
        if(last_val == expected) cout << " ✅";
        else cout << " ❌ MISMATCH";
        cout << "\n";
    }

    // === EXPERIMENT 2: Fibonacci Anchor Pattern ===
    cout << "\n=== EXPERIMENT 2: FIBONACCI ANCHOR PATTERN ===\n";
    cout << "Alternating between Enc(0) and Enc(1) every operation\n\n";
    
    {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(1);
        params.SetPlaintextModulus(65537);
        params.SetSecurityLevel(HEStd_NotSet);
        
        CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        vector<int64_t> z_vec = {0}, o_vec = {1};
        auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(z_vec));
        auto anchor1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(o_vec));
        
        vector<int64_t> val_vec = {42};
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val_vec));
        
        int fib_count = 0;
        for(int i = 0; i < 1000; i++) {
            if(i % 2 == 0) ct = cc->EvalAdd(ct, anchor0);
            else ct = cc->EvalAdd(ct, anchor1);
            fib_count++;
            
            if(i == 999) {
                Plaintext check_pt;
                cc->Decrypt(keys.secretKey, ct, &check_pt);
                auto val = check_pt->GetPackedValue()[0];
                cout << "  After 1000 ops (500×Enc(0) + 500×Enc(1)):\n";
                cout << "  Value: " << val << " (expected " << (42 + 500) << ")";
                if(val == 42 + 500) cout << " ✅\n";
                else cout << " ❌\n";
            }
        }
    }

    // === EXPERIMENT 3: φ-Scaled Anchor ===
    cout << "\n=== EXPERIMENT 3: φ-SCALED ANCHOR VALUES ===\n";
    cout << "Using golden ratio multiples as anchor values\n\n";
    
    vector<int64_t> phi_anchors = {0, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
    // These are Fibonacci numbers — approximations of φ^n
    
    cout << "┌──────────┬──────────┬──────────────┐\n";
    cout << "│ Anchor   │ Max Ops  │ Drift/op     │\n";
    cout << "├──────────┼──────────┼──────────────┤\n";
    
    for(auto av : phi_anchors) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(5);  // More depth for this test
        params.SetPlaintextModulus(65537);
        params.SetSecurityLevel(HEStd_NotSet);
        
        CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        vector<int64_t> av_vec = {av};
        auto anchor = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(av_vec));
        
        vector<int64_t> val_vec = {0};  // Start from 0
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(val_vec));
        
        int max_ops = 0;
        int64_t last_val = 0;
        
        for(int i = 0; i < 10000; i++) {
            ct = cc->EvalAdd(ct, anchor);
            
            Plaintext check_pt;
            cc->Decrypt(keys.secretKey, ct, &check_pt);
            int64_t val = check_pt->GetPackedValue()[0];
            int64_t expected = av * (i + 1);
            
            if(val != expected) {
                max_ops = i;
                last_val = val;
                break;
            }
            
            if(i == 9999) {
                max_ops = 10000;
                last_val = val;
            }
        }
        
        cout << "│ Enc(" << setw(2) << av << ")    │ ";
        if(max_ops >= 10000) cout << "10000+    │ 0.0000       │\n";
        else cout << setw(4) << max_ops << "     │ " 
                  << fixed << setprecision(4) 
                  << (double)(av * max_ops - last_val) / max_ops 
                  << "       │\n";
    }
    cout << "└──────────┴──────────┴──────────────┘\n";

    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — FLOOR INVESTIGATION COMPLETE           ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
