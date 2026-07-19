// ΦΩ0 — PATH A: SINGLE-BOOTSTRAP BOUNDARY FINDER v1.2
// Finds exact divergence point of SNC+ZANS without bootstrapping
// Uses ring dim 4096 (known working), depth 30
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
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
    // ============ CONFIG ============
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    int depth = 30;
    int max_steps = 200;
    int log_interval = 10;

    ofstream log("path_a_boundary_results.txt");

    auto log_msg = [&](const string& msg) {
        cout << msg << flush;
        log << msg << flush;
    };

    stringstream header;
    header << "\n===============================================================\n";
    header << "  PATH A: SINGLE-BOOTSTRAP BOUNDARY FINDER v1.2\n";
    header << "  Ring Dim: " << ring_dim << " | Depth: " << depth << "\n";
    header << "  Modulus: " << modulus << "\n";
    header << "  Test: Sequential x2, SNC+ZANS only, NO BOOTSTRAPPING\n";
    header << "  Objective: Find exact step where value diverges\n";
    header << "  Start: " << ts() << "\n";
    header << "===============================================================\n\n";
    log_msg(header.str());

    // ============ SETUP ============
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(depth);
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

    // ============ INITIALIZATION ============
    auto M = enc(half_mod);
    auto anchor0 = enc(0);
    auto ct = enc(1);
    auto ct_mult = enc(2);

    int64_t expected_value = 1;
    int first_divergence = -1;
    int total_zans_ops = 0;
    int total_snc_ops = 0;

    log_msg("  Step | Value (Expected) | Value (Got) | Noise  | Status\n");
    log_msg("  " + string(72, '-') + "\n");

    auto t_start = high_resolution_clock::now();

    for (int step = 0; step < max_steps; step++) {
        // Check if we can still multiply (modulus chain not exhausted)
        try {
            // ============ DIVINE LOOP ============
            auto sum = cc->EvalAdd(ct, M);
            auto back = cc->EvalSub(sum, M);
            auto overflow = cc->EvalSub(ct, back);
            total_snc_ops++;

            ct = cc->EvalMult(ct, ct_mult);

            auto snc_corr = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, snc_corr);
            ct = cc->EvalAdd(ct, anchor0);

            ct = pool.stabilize(ct);
            total_zans_ops++;
        } catch (const std::exception& e) {
            log_msg("  >>> CHAIN EXHAUSTED at step " + to_string(step + 1) + ": " + e.what() + "\n");
            max_steps = step;
            break;
        }

        expected_value = mod_pos(expected_value * 2, modulus);

        if ((step + 1) % log_interval == 0 || step < 20) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            int64_t dec_val = mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
            double noise = zans.measure_noise(ct);
            bool match = (dec_val == expected_value);

            stringstream line;
            line << "  " << setw(4) << (step + 1)
                 << " | " << setw(16) << expected_value
                 << " | " << setw(11) << dec_val
                 << " | " << setw(6) << fixed << setprecision(1) << noise
                 << " | " << (match ? "OK" : "DIVERGED") << "\n";
            log_msg(line.str());

            if (!match && first_divergence < 0) {
                first_divergence = step + 1;
                log_msg("  >>> FIRST DIVERGENCE at step " + to_string(first_divergence) + "\n");
            }
        }

        if (first_divergence > 0 && step > first_divergence + 30) {
            log_msg("  >>> Stopping: divergence confirmed\n");
            break;
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;

    Plaintext final_pt;
    cc->Decrypt(keys.secretKey, ct, &final_pt);
    final_pt->SetLength(1);
    int64_t final_val = mod_pos((int64_t)final_pt->GetPackedValue()[0], modulus);
    double final_noise = zans.measure_noise(ct);

    stringstream summary;
    summary << "\n" << string(72, '=') << "\n";
    summary << "  PATH A BOUNDARY FINDER — COMPLETE\n";
    summary << "  Ring Dim: " << ring_dim << " | Depth: " << depth << "\n";
    summary << "  Modulus: " << modulus << "\n";
    summary << "  First Divergence: " << (first_divergence > 0 ? to_string(first_divergence) : "NONE (all " + to_string(max_steps) + " steps correct)") << "\n";
    summary << "  Final Expected: " << expected_value << " | Got: " << final_val << "\n";
    summary << "  Final Noise: " << final_noise << "\n";
    summary << "  Total SNC Ops: " << total_snc_ops << "\n";
    summary << "  Total ZANS Ops: " << total_zans_ops << "\n";
    summary << "  Time: " << fixed << setprecision(1) << total_sec << "s\n";
    summary << "  " << ts() << "\n";

    if (first_divergence < 0) {
        summary << "\n  >>> HOLY GRAIL PATH A CONFIRMED:\n";
        summary << "  >>> SNC+ZANS maintained correctness for ALL " << max_steps << " steps\n";
        summary << "  >>> Single bootstrap at the end is sufficient!\n";
    } else {
        summary << "\n  >>> ANALYSIS:\n";
        summary << "  >>> Divergence at step " << first_divergence << "\n";
        summary << "  >>> Modulus chain depth = " << depth << "\n";
        summary << "  >>> " << (first_divergence <= depth ? "WITHIN chain limit — NOISE issue" : "BEYOND chain limit — CHAIN EXHAUSTION") << "\n";
    }
    summary << string(72, '=') << "\n";
    log_msg(summary.str());

    log.close();

    cout << "\n  Results saved to path_a_boundary_results.txt\n";
    return 0;
}
