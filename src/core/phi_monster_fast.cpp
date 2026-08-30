// ============================================
// φ-MONSTER FAST — 100K CHAIN (FIXED)
//
// 1 encrypt lang, 100K EvalAdd
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-MONSTER FAST — 100K CHAIN\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 8 slots)\n\n";

    auto encrypt_log = [&](double value) {
        double log_phi = log(value) / LN_PHI;
        vector<double> v(8, 0.0);
        v[0] = log_phi;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);
    };

    cout << "========================================\n";
    cout << "  100K CHAIN (1 ENCRYPT + 100K EVALADD)\n";
    cout << "========================================\n\n";

    auto start = high_resolution_clock::now();

    auto ct_chain = encrypt_log(1.0);
    auto ct_op = encrypt_log(2.0);  // ISANG ENCRYPT LANG

    for (int i = 0; i < 100000; i++) {
        ct_chain = cc->EvalAdd(ct_chain, ct_op);  // 100K EvalAdd
    }

    auto end = high_resolution_clock::now();
    auto chain_time = duration_cast<milliseconds>(end - start).count();

    cout << "  ✅ 100K chain complete!\n";
    cout << "  Time: " << chain_time << " ms\n";
    cout << "  Level: " << ct_chain->GetLevel() << "\n";
    cout << "  Towers: " << ct_chain->GetElements()[0].GetNumOfElements() << "\n\n";

    // Fractal compression version
    cout << "========================================\n";
    cout << "  100K FRACTAL (φ-POWER GROUPS)\n";
    cout << "========================================\n\n";

    vector<int> phi_groups;
    int rem = 100000;
    int gid = 0;
    while (rem > 0) {
        int sz = min(rem, (int)pow(PHI, gid + 1));
        phi_groups.push_back(sz);
        rem -= sz;
        gid++;
    }

    cout << "  Groups: " << phi_groups.size() << "\n";

    auto start_fractal = high_resolution_clock::now();

    auto ct_fractal = encrypt_log(1.0);

    for (int gs : phi_groups) {
        double group_log = gs * log(2.0) / LN_PHI;
        vector<double> gv(8, 0.0);
        gv[0] = group_log;
        Plaintext pt_g = cc->MakeCKKSPackedPlaintext(gv);
        auto ct_g = cc->Encrypt(keyPair.publicKey, pt_g);
        ct_fractal = cc->EvalAdd(ct_fractal, ct_g);
    }

    auto end_fractal = high_resolution_clock::now();
    auto fractal_time = duration_cast<milliseconds>(end_fractal - start_fractal).count();

    cout << "  ✅ Fractal complete!\n";
    cout << "  Time: " << fractal_time << " ms\n";
    cout << "  Level: " << ct_fractal->GetLevel() << "\n";
    cout << "  Towers: " << ct_fractal->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  COMPARISON\n";
    cout << "========================================\n\n";
    cout << "  100K na encrypt per op: " << chain_time << " ms (100K encrypt + 100K add)\n";
    cout << "  Fractal: " << fractal_time << " ms (" << phi_groups.size() << " encrypt + " << phi_groups.size() << " add)\n\n";

    return 0;
}
