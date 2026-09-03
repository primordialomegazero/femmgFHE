// ============================================
// φ-1 TRILLION DEBUG — PRINT LAHAT
//
// Makikita natin kung saan nagkakaproblema
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
    cout << "  φ-1 TRILLION DEBUG\n";
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

    auto encrypt_log = [&](double value) {
        double log_phi = log(value) / LN_PHI;
        vector<double> v(8, 0.0);
        v[0] = log_phi;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // DEBUG: Tingnan ang first few groups
    // ============================================

    long long total_ops = 1000000000000LL;
    double log_per_op = log(2.0) / LN_PHI;

    vector<long long> phi_groups;
    long long rem = total_ops;
    int gid = 0;
    while (rem > 0) {
        long long sz = min(rem, (long long)pow(PHI, gid + 1));
        phi_groups.push_back(sz);
        rem -= sz;
        gid++;
    }

    cout << "  log_per_op: " << log_per_op << "\n";
    cout << "  Total groups: " << phi_groups.size() << "\n\n";

    cout << "  FIRST 10 GROUPS:\n";
    cout << "  Group | Size | Group Log | Mod φ\n";
    cout << "  ------|------|-----------|-------\n";

    vector<double> meta_groups;
    for (int i = 0; i < min(10, (int)phi_groups.size()); i++) {
        long long gs = phi_groups[i];
        double group_log = gs * log_per_op;
        double meta_mod = fmod(group_log, PHI);
        meta_groups.push_back(meta_mod);

        cout << "  " << setw(5) << i << " | "
             << setw(4) << gs << " | "
             << setw(9) << fixed << setprecision(4) << group_log << " | "
             << setw(5) << meta_mod << "\n";
    }

    // I-continue ang meta_groups para sa lahat
    for (int i = 10; i < phi_groups.size(); i++) {
        long long gs = phi_groups[i];
        double group_log = gs * log_per_op;
        double meta_mod = fmod(group_log, PHI);
        meta_groups.push_back(meta_mod);
    }

    cout << "\n  LAST 5 GROUPS:\n";
    cout << "  Group | Size | Group Log | Mod φ\n";
    cout << "  ------|------|-----------|-------\n";

    for (int i = phi_groups.size() - 5; i < phi_groups.size(); i++) {
        long long gs = phi_groups[i];
        double group_log = gs * log_per_op;
        double meta_mod = fmod(group_log, PHI);
        cout << "  " << setw(5) << i << " | "
             << setw(4) << gs << " | "
             << setw(9) << fixed << setprecision(4) << group_log << " | "
             << setw(5) << meta_mod << "\n";
    }

    cout << "\n  ========================================\n";
    cout << "  COMPUTATION DEBUG\n";
    cout << "  ========================================\n\n";

    auto ct_result = encrypt_log(1.0);
    cout << "  Start: log_φ(1) = 0\n";
    cout << "  Initial ciphertext: 0\n\n";

    double accumulated = 0.0;

    cout << "  FIRST 10 ADDITIONS:\n";
    cout << "  Step | Add Value | Accumulated (plain) | Ciphertext (decrypted)\n";
    cout << "  -----|-----------|---------------------|----------------------\n";

    for (int i = 0; i < min(10, (int)meta_groups.size()); i++) {
        double mg = meta_groups[i];
        
        vector<double> gv(8, 0.0);
        gv[0] = mg;
        Plaintext pt_g = cc->MakeCKKSPackedPlaintext(gv);
        auto ct_g = cc->Encrypt(keyPair.publicKey, pt_g);
        
        ct_result = cc->EvalAdd(ct_result, ct_g);
        accumulated = fmod(accumulated + mg, PHI);
        
        double decrypted = decrypt_log(ct_result);
        
        cout << "  " << setw(4) << i << " | "
             << setw(9) << fixed << setprecision(4) << mg << " | "
             << setw(19) << setprecision(4) << accumulated << " | "
             << setw(20) << decrypted << "\n";
    }

    cout << "\n  ========================================\n";
    cout << "  PROBLEM ANALYSIS\n";
    cout << "  ========================================\n\n";
    
    cout << "  Expected (accumulated): " << accumulated << "\n";
    cout << "  Kung ang ciphertext decryption ay lumalaki,\n";
    cout << "  ibig sabihin hindi naa-apply ang modulo sa encrypted domain.\n\n";
    
    cout << "  Ang ciphertext ay nag-a-accumulate ng RAW values,\n";
    cout << "  hindi ang modded values.\n\n";

    return 0;
}
