#include <openfhe.h>
#include <iostream>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 5);
    
    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };
    
    int64_t half_mod = 1073643521 / 2;
    auto M = enc(half_mod);
    auto anchor0 = enc(0);
    
    cout << "=== ARBITRARY CT×CT TEST ===\n\n";
    
    // Test: ct_a × ct_b where both are random encrypted values
    for(int test = 0; test < 5; test++) {
        int64_t a = 3 + test;
        int64_t b = 7 + test * 2;
        int64_t expected = a * b;
        
        auto ct_a = enc(a);
        auto ct_b = enc(b);
        
        // Pinky Swear on ct_a
        auto sum = cc->EvalAdd(ct_a, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct_a, back);
        
        // CT×CT: ct_a × ct_b
        auto ct_result = cc->EvalMult(ct_a, ct_b);
        
        // Divine Intervention
        auto divine = cc->EvalMult(overflow, anchor0);
        ct_result = cc->EvalAdd(ct_result, divine);
        ct_result = pool.stabilize(ct_result);
        
        int64_t result = dec(ct_result);
        bool match = (result == expected);
        
        cout << "  " << a << " × " << b << " = " << result 
             << " (expected " << expected << ") "
             << (match ? "OK" : "FAIL") << "\n";
    }
    
    cout << "\n=== CHAIN TEST: 20 steps CT×CT ===\n\n";
    
    auto ct = enc(3);
    auto ct_mult = enc(5);
    
    for(int step = 0; step < 20; step++) {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        ct = cc->EvalMult(ct, ct_mult);
        
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = pool.stabilize(ct);
        
        if(step % 5 == 0 || step == 19) {
            int64_t val = dec(ct);
            double noise = ct->GetNoiseScaleDeg();
            cout << "  Step " << step << ": value=" << val << " noise=" << noise << "\n";
        }
    }
    
    cout << "\nDone.\n";
    return 0;
}
