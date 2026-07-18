#include <openfhe.h>
#include <iostream>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }
int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521, half_mod = modulus / 2;
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    ZANSAnchorPool pool(cc, keys, 10);
    
    auto enc = [&](int64_t v){ return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v})); };
    auto M = enc(half_mod);
    auto anchor0 = enc(0);
    auto ct = enc(1);
    int64_t expected = 1;

    cout << "=== CIRCUIT TEST: Group operations ===" << endl;
    cout << "Method: (ct × 2) + Enc(0) as single expression" << endl;
    
    for (int step = 0; step < 100; step++) {
        auto mult = enc(2);
        expected = mod_pos(expected * 2, modulus);
        
        // PARENTHESIS: Group multiply and ZANS in one expression
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        auto multiplied = cc->EvalMult(ct, mult);
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(cc->EvalAdd(multiplied, divine), anchor0);
        ct = pool.stabilize(ct);

        if ((step+1) % 20 == 0) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
            cout << "Step " << step+1 << ": " << (dec == expected ? "OK" : "FAIL") << endl;
            if (dec != expected) return 1;
        }
    }
    cout << "ALL 100 OK!" << endl;
}
