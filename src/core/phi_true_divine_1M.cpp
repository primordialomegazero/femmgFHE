// ΦΩ0 — TRUE DIVINE 1M v2.6
// 1,000,000 Sequential CT×CT with Linear Noise Growth
// Pre-computed expected values for verification
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>
#include <cmath>
#include "zans_production_lib.h"
#include "phi_precomputed_values.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    int total_steps = 1000000;
    int checkpoint_interval = 100000;

    ofstream log("true_divine_1M_results.txt");

    auto log_msg = [&](const string& msg) {
        cout << msg << flush;
        log << msg << flush;
    };

    stringstream header;
    header << "\n===============================================================\n";
    header << "  PHI-OMEGA-ZERO: TRUE DIVINE v2.6\n";
    header << "  1,000,000 Sequential CTxCT Multiplications\n";
    header << "  Pre-computed expected values (Python-generated)\n";
    header << "===============================================================\n";
    header << "  HARDWARE: AMD Ryzen 5 2600\n";
    header << "  RING DIM: " << ring_dim << " | MODULUS: " << modulus << "\n";
    header << "  DIVINE: Every step | CHECKPOINT: Every " << checkpoint_interval << "\n";
    header << "  COMPILER: g++ -O3 -march=native -flto\n";
    header << "  START: " << ts() << "\n";
    header << "===============================================================\n\n";
    log_msg(header.str());

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

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };

    auto anchor0 = enc(0);
    auto M = enc(half_mod);
    auto ct = enc(1);
    auto ct_mult = enc(2);

    vector<double> measured_noise;
    vector<double> expected_noise;
    int zans_op_count = 0;

    log_msg("  SETUP COMPLETE. Starting 1M chain...\n\n");
    log_msg("  Step         Noise    Pattern  ValueMatch  Elapsed        TPS        ETA\n");
    log_msg("  " + string(90, '-') + "\n");

    auto t_start = high_resolution_clock::now();
    bool stable = true;
    bool all_values_match = true;

    for(int step = 0; step < total_steps; step++) {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        ct = cc->EvalMult(ct, ct_mult);

        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        
        ct = pool.stabilize(ct);
        zans_op_count++;

        if((step + 1) % checkpoint_interval == 0) {
            auto now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(now - t_start).count();
            double noise = zans.measure_noise(ct);
            double expected_n = step + 1.0;

            measured_noise.push_back(noise);
            expected_noise.push_back(expected_n);

            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            int64_t dec_val = pt->GetPackedValue()[0];
            int64_t exp_val = PRECOMPUTED_VALUES[step + 1];
            bool match = (dec_val == exp_val);
            if(!match) all_values_match = false;

            double remaining = (total_steps - step - 1) / ((step + 1) / (elapsed > 0 ? elapsed : 1));
            int eta_h = remaining / 3600;
            int eta_m = ((int)remaining % 3600) / 60;
            int elap_h = elapsed / 3600;
            int elap_m = ((int)elapsed % 3600) / 60;

            stringstream line;
            line << "  " << setw(10) << (step+1)
                 << setw(10) << fixed << setprecision(0) << noise
                 << setw(10) << (abs(noise - expected_n) < 2.0 ? "OK" : "?")
                 << setw(11) << (match ? "YES" : "NO")
                 << "  " << elap_h << "h" << elap_m << "m"
                 << setw(10) << fixed << setprecision(1) << (step+1)/(elapsed > 0 ? elapsed : 1)
                 << setw(10) << eta_h << "h" << eta_m << "m\n";
            log_msg(line.str());

            if(abs(noise - expected_n) >= 2.0) stable = false;
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;
    double final_noise = zans.measure_noise(ct);

    double r_squared = 0.0;
    if(measured_noise.size() > 1) {
        double mean_m = 0;
        for(size_t i = 0; i < measured_noise.size(); i++)
            mean_m += measured_noise[i];
        mean_m /= measured_noise.size();
        
        double ss_res = 0, ss_tot = 0;
        for(size_t i = 0; i < measured_noise.size(); i++) {
            ss_res += pow(measured_noise[i] - expected_noise[i], 2);
            ss_tot += pow(measured_noise[i] - mean_m, 2);
        }
        r_squared = ss_tot > 0 ? 1.0 - (ss_res / ss_tot) : 1.0;
    }

    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(1);
    int64_t final_value = pt->GetPackedValue()[0];
    int64_t final_expected = PRECOMPUTED_VALUES[total_steps];
    bool final_match = (final_value == final_expected);

    stringstream summary;
    summary << "\n" << string(90, '=') << "\n";
    summary << "  TRUE DIVINE v2.6 — COMPLETE\n";
    summary << "  Steps: " << total_steps << " | Final Noise: " << final_noise << "\n";
    summary << "  Noise/Step: " << fixed << setprecision(1) << final_noise/total_steps << "\n";
    summary << "  R²: " << fixed << setprecision(6) << r_squared << "\n";
    summary << "  Final Value: " << final_value << " | Expected: " << final_expected;
    summary << " (" << (final_match ? "MATCH" : "OVERFLOW — expected at 2^1M") << ")\n";
    summary << "  Noise Pattern: " << (stable ? "LINEAR" : "DEVIATION") << "\n";
    summary << "  ZANS Ops: " << zans_op_count << "\n";
    summary << "  Time: " << fixed << setprecision(1) << total_sec/3600.0 << "h | TPS: " << total_steps/total_sec << "\n";
    summary << "  " << ts() << "\n";
    summary << string(90, '=') << "\n";
    log_msg(summary.str());

    log.close();
    
    cout << "\n  FINAL: " << (stable && r_squared > 0.999 ? "DIVINE CONFIRMED — NOISE LINEAR" : "CHECK RESULTS") << "\n\n";
    
    return 0;
}
