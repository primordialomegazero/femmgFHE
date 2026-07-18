#include <openfhe.h>
#include <iostream>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t mod) { return ((v % mod) + mod) % mod; }
int64_t mod_inv(int64_t a, int64_t mod) {
    int64_t t = 0, newt = 1, r = mod, newr = a;
    while (newr != 0) {
        int64_t q = r / newr;
        int64_t tmp = newt; newt = t - q * newt; t = tmp;
        tmp = newr; newr = r - q * newr; r = tmp;
    }
    return mod_pos(t, mod);
}
int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
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

    int64_t expecteds[20] = {465173615,246245601,216480218,78818121,78819066,188656018,78819066,266280601,266281310,266282220,773302737,773303594,397161505,589204970,589204650,589203816,237664839,237665196,237664605,237665165};
    string ops[20] = {"inv","inv","mul","mul","add","inv","inv","mul","add","add","mul","add","mul","mul","sub","sub","inv","add","sub","add"};
    int64_t vals[20] = {0,0,75,45,945,0,0,17,709,910,15,857,38,15,320,834,0,357,591,560};

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{246245601}));

    cout << "=== FULL ARITHMETIC TEST (add/sub/mul/inv) ===" << endl;
    cout << "Start: 246245601" << endl;

    for (int i = 0; i < 20; i++) {
        if (ops[i] == "add") {
            auto ct_val = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{vals[i]}));
            ct = cc->EvalAdd(ct, ct_val);
        } else if (ops[i] == "sub") {
            auto ct_val = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{vals[i]}));
            ct = cc->EvalSub(ct, ct_val);
        } else if (ops[i] == "mul") {
            auto ct_val = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{vals[i]}));
            ct = cc->EvalMult(ct, ct_val);
        } else if (ops[i] == "inv") {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t val = mod_pos(pt->GetPackedValue()[0], modulus);
            int64_t inv_val = mod_inv(val, modulus);
            ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{inv_val}));
            cout << "Step " << i << " inv: dec=" << inv_val << " exp=" << expecteds[i] << " [" << (inv_val == expecteds[i] ? "OK" : "FAIL") << "]" << endl;
            continue;
        }
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        bool ok = (dec_val == expecteds[i]);
        cout << "Step " << i << " " << ops[i] << " " << vals[i] 
             << ": dec=" << dec_val << " exp=" << expecteds[i] 
             << " [" << (ok ? "OK" : "FAIL") << "]" << endl;
        if (!ok) return 1;
    }
    cout << "ALL 20 STEPS PASSED" << endl;
    return 0;
}
