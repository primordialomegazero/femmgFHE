#include <openfhe.h>
#include <iostream>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t mod) { return ((v % mod) + mod) % mod; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;  // prime
    int64_t exponent = modulus - 2; // Fermat's Little Theorem

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(40);  // enough for ~30 squarings
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 10);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));

    int64_t test_vals[] = {2, 3, 5, 7, 42, 123, 1000, 999999};
    
    cout << "=== HOMOMORPHIC MODULAR INVERSE (Fermat's Little Theorem) ===" << endl;
    cout << "Exponent = p-2 = " << exponent << " (mod " << modulus << ")" << endl;
    cout << string(60, '-') << endl;

    for (int64_t a : test_vals) {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{a}));
        auto result = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));  // a^0
        auto base = ct;
        
        // Exponentiation by squaring
        int64_t exp = exponent;
        while (exp > 0) {
            if (exp & 1) {
                result = cc->EvalMult(result, base);
                result = cc->EvalAdd(result, anchor0);
                result = pool.stabilize(result);
            }
            base = cc->EvalMult(base, base);
            base = cc->EvalAdd(base, anchor0);
            base = pool.stabilize(base);
            exp >>= 1;
        }

        Plaintext pt;
        cc->Decrypt(keys.secretKey, result, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        int64_t expected = mod_pos(a == 0 ? 0 : (int64_t)pow((double)a, -1) * modulus, modulus);
        // Compute expected using Python-style modular inverse
        int64_t t = 0, newt = 1, r = modulus, newr = a;
        while (newr != 0) {
            int64_t q = r / newr;
            int64_t tmp = newt; newt = t - q * newt; t = tmp;
            tmp = newr; newr = r - q * newr; r = tmp;
        }
        expected = mod_pos(t, modulus);
        
        bool ok = (dec_val == expected);
        cout << "inv(" << a << "): dec=" << dec_val << " exp=" << expected 
             << " [" << (ok ? "OK" : "FAIL") << "]" << endl;
        if (!ok) return 1;
    }
    
    cout << string(60, '-') << endl;
    cout << "ALL INVERSES CORRECT (homomorphic, no decrypt!)" << endl;
    return 0;
}
