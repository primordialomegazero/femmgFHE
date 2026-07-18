// RANDOM CHAIN 10K CT×CT TEST v3 — CORRECT START VALUE
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include "zans_production_lib.h"
#include "phi_random_chain_1M.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    int total_steps = 10000;
    int checkpoint = 1000;

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
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));

    int64_t start_val = 686579304;  // INITIAL_X — before any multiplication
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{start_val}));

    cout << "\n=== RANDOM CHAIN 10K CT×CT TEST v3 ===" << endl;
    cout << "Start: " << start_val << " | Steps: " << total_steps << " | Seed: 42" << endl;
    cout << "Verification: VALUES[step] = expected after each MULTS[step]" << endl;
    cout << string(70, '-') << endl;

    auto t_start = high_resolution_clock::now();
    bool all_ok = true;

    for(int step = 0; step < total_steps; step++) {
        auto ct_mult = cc->Encrypt(keys.publicKey, 
            cc->MakePackedPlaintext(vector<int64_t>{RANDOM_CHAIN_MULTS[step]}));

        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);

        ct = cc->EvalMult(ct, ct_mult);

        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);

        ct = pool.stabilize(ct);

        if((step + 1) % checkpoint == 0) {
            auto now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(now - t_start).count();
            double scale_deg = ct->GetNoiseScaleDeg();

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            int64_t dec_val = pt->GetPackedValue()[0];
            int64_t exp_val = RANDOM_CHAIN_VALUES[step];
            bool match = (dec_val == exp_val);

            if(!match) all_ok = false;

            double tps = (step+1) / (elapsed > 0 ? elapsed : 1);
            cout << "Step " << setw(5) << (step+1) 
                 << " | Scale=" << setw(5) << (int)scale_deg
                 << " | Value=" << (match ? "OK" : "FAIL")
                 << " | TPS=" << setw(4) << (int)tps
                 << " | " << (int)(elapsed/60) << "m" << (int)elapsed%60 << "s" << endl;
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;

    cout << string(70, '-') << endl;
    cout << (all_ok ? "ALL 10 CHECKPOINTS PASSED" : "SOME FAILURES") << endl;
    cout << "Total time: " << (int)(total_sec/60) << "m " << (int)total_sec%60 << "s" << endl;
    return all_ok ? 0 : 1;
}
