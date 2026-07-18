// ΦΩ0 — FINAL ARBITRARY CT×CT WITH PRE-COMPUTED MULTIPLIERS
// Clean build test — after OpenFHE rebuild
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;

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
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));

    // Pre-compute 200 random multipliers
    mt19937_64 rng(42);
    uniform_int_distribution<int64_t> dist(2, 100);
    
    vector<int64_t> mults;
    vector<Ciphertext<DCRTPoly>> ct_mults;
    for (int i = 0; i < 200; i++) {
        int64_t m = dist(rng);
        mults.push_back(m);
        ct_mults.push_back(cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{m})));
    }

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "\n=== FINAL ARBITRARY CT×CT TEST ===" << endl;
    cout << "200 random multipliers (2-100), pre-computed, Full True Divine" << endl;
    cout << setw(5) << "Step" << setw(8) << "Mult" << setw(10) << "Scale" 
         << setw(14) << "Expected" << setw(14) << "Decrypted" << setw(8) << "Result" << endl;
    cout << string(62, '-') << endl;

    int max_ok = 0;
    for (int step = 0; step < 200; step++) {
        expected = mod_pos(expected * mults[step], modulus);

        // Full True Divine
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mults[step]);
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        bool ok = (dec_val == expected);
        
        if (ok) max_ok = step + 1;

        if ((step+1) % 20 == 0 || !ok) {
            cout << setw(5) << step+1 << setw(8) << mults[step] 
                 << setw(10) << ct->GetNoiseScaleDeg()
                 << setw(14) << expected << setw(14) << dec_val 
                 << setw(8) << (ok ? "OK" : "FAIL") << endl;
            if (!ok) break;
        }
    }
    
    cout << string(62, '-') << endl;
    cout << "Steps passed: " << max_ok << "/200" << endl;
    if (max_ok == 200) cout << "*** UNLIMITED ARBITRARY CT×CT ACHIEVED! ***" << endl;
    
    return (max_ok == 200) ? 0 : 1;
}
