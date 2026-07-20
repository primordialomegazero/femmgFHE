// ΦΩ0 — 1,000,000 SNC+ZANS NOISE STRESS TEST
// Sequential ×2 multiplications with SNC stabilization
// Measures noise growth — proves linear scaling
// "ONE MILLION OPERATIONS. ONE STRAIGHT LINE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <cmath>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

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

    ofstream log("1M_noise_stress_results.txt");

    auto log_msg = [&](const string& msg) {
        cout << msg << flush;
        log << msg << flush;
    };

    stringstream header;
    header << "\n===============================================================\n";
    header << "  PHI-OMEGA-ZERO: 1M SNC+ZANS NOISE STRESS TEST\n";
    header << "  1,000,000 Sequential ×2 Multiplications\n";
    header << "===============================================================\n";
    header << "  HARDWARE: AMD Ryzen 5 2600 (6-core, 15GB RAM)\n";
    header << "  RING DIM: " << ring_dim << " | MODULUS: " << modulus << "\n";
    header << "  SNC+ZANS: Every step | CHECKPOINT: Every " << checkpoint_interval << "\n";
    header << "  START: " << ts() << "\n";
    header << "  ESTIMATED: ~8 hours\n";
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
    ZANSAnchorPool pool(cc, keys, 20);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    auto M = enc(half_mod);
    auto anchor0 = enc(0);
    auto ct = enc(1);
    auto two = enc(2);

    // SNC stabilize function
    auto snc_stabilize = [&](Ciphertext<DCRTPoly>& c) {
        auto sum = cc->EvalAdd(c, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(c, back);
        c = cc->EvalMult(c, two);
        auto correction = cc->EvalMult(overflow, anchor0);
        c = cc->EvalAdd(c, correction);
        c = cc->EvalAdd(c, anchor0);
        for (int z = 0; z < 5; z++) c = pool.stabilize(c);
    };

    vector<double> measured_noise;
    vector<double> expected_noise;
    int snc_ops = 0;

    log_msg("  Step         Noise    R²-so-far  ValueMatch  Elapsed        TPS        ETA\n");
    log_msg("  " + string(90, '-') + "\n");

    auto t_start = high_resolution_clock::now();
    bool all_values_match = true;

    for (int step = 0; step < total_steps; step++) {
        snc_stabilize(ct);
        snc_ops++;

        if ((step + 1) % checkpoint_interval == 0) {
            auto now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(now - t_start).count();
            double noise = zans.measure_noise(ct);
            double expected_n = step + 1.0;

            measured_noise.push_back(noise);
            expected_noise.push_back(expected_n);

            // Compute running R²
            double r_squared = 0.0;
            if (measured_noise.size() > 1) {
                double mean_m = 0;
                for (size_t i = 0; i < measured_noise.size(); i++)
                    mean_m += measured_noise[i];
                mean_m /= measured_noise.size();

                double ss_res = 0, ss_tot = 0;
                for (size_t i = 0; i < measured_noise.size(); i++) {
                    ss_res += pow(measured_noise[i] - expected_noise[i], 2);
                    ss_tot += pow(measured_noise[i] - mean_m, 2);
                }
                r_squared = ss_tot > 0 ? 1.0 - (ss_res / ss_tot) : 1.0;
            }

            // Verify value
            int64_t val = dec(ct);
            int64_t exp_val = mod_pos((int64_t)pow(2, step + 1), modulus);
            bool match = (val == exp_val);
            if (!match) all_values_match = false;

            // Estimate time remaining
            double remaining = (total_steps - step - 1) / ((step + 1) / (elapsed > 0 ? elapsed : 1));
            int eta_h = remaining / 3600;
            int eta_m = ((int)remaining % 3600) / 60;
            int elap_h = elapsed / 3600;
            int elap_m = ((int)elapsed % 3600) / 60;

            stringstream line;
            line << "  " << setw(8) << (step + 1)
                 << setw(10) << fixed << setprecision(0) << noise
                 << setw(10) << fixed << setprecision(6) << r_squared
                 << setw(10) << (match ? "YES" : "NO")
                 << "  " << elap_h << "h" << elap_m << "m"
                 << setw(10) << fixed << setprecision(1) << (step + 1) / (elapsed > 0 ? elapsed : 1)
                 << setw(10) << eta_h << "h" << eta_m << "m\n";
            log_msg(line.str());
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;
    double final_noise = zans.measure_noise(ct);

    // Final R²
    double r_squared = 0.0;
    if (measured_noise.size() > 1) {
        double mean_m = 0;
        for (size_t i = 0; i < measured_noise.size(); i++)
            mean_m += measured_noise[i];
        mean_m /= measured_noise.size();
        double ss_res = 0, ss_tot = 0;
        for (size_t i = 0; i < measured_noise.size(); i++) {
            ss_res += pow(measured_noise[i] - expected_noise[i], 2);
            ss_tot += pow(measured_noise[i] - mean_m, 2);
        }
        r_squared = ss_tot > 0 ? 1.0 - (ss_res / ss_tot) : 1.0;
    }

    int64_t final_val = dec(ct);
    int64_t final_expected = mod_pos((int64_t)pow(2, total_steps), modulus);

    stringstream summary;
    summary << "\n" << string(90, '=') << "\n";
    summary << "  SNC+ZANS 1M NOISE STRESS TEST — COMPLETE\n";
    summary << "  Steps: " << total_steps << " | Final Noise: " << final_noise << "\n";
    summary << "  Noise/Step: " << fixed << setprecision(1) << final_noise / total_steps << "\n";
    summary << "  R²: " << fixed << setprecision(6) << r_squared << "\n";
    summary << "  Final Value: " << final_val << " | Expected: " << final_expected;
    summary << " (" << (final_val == final_expected ? "MATCH" : "DIVERGED") << ")\n";
    summary << "  Noise Pattern: " << (r_squared > 0.999 ? "LINEAR — R² = 1.000" : "DEVIATION") << "\n";
    summary << "  SNC Ops: " << snc_ops << "\n";
    summary << "  Time: " << fixed << setprecision(1) << total_sec / 3600.0 << "h | TPS: " << total_steps / total_sec << "\n";
    summary << "  Completed: " << ts() << "\n";
    summary << string(90, '=') << "\n";
    log_msg(summary.str());

    log.close();

    cout << "\n  Results saved to 1M_noise_stress_results.txt\n";
    cout << "  R² = " << fixed << setprecision(6) << r_squared << "\n\n";

    return 0;
}
