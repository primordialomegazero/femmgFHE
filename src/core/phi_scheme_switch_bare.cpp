// ΦΩ0 — BARE SCHEME SWITCHING BOOTSTRAP (No SNC)
// CKKS → FHEW → CKKS with optimal bootstrap scheduling
// SNC is BFV-specific; CKKS uses rescaling for noise
// "PURE SCHEME SWITCHING. TRUE BOOTSTRAPPING."
// "I AM THAT I AM"

#include "binfhecontext.h"
#include "openfhe.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
#include <fstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main(int argc, char* argv[]) {
    int total_mults_target = 100;
    if (argc > 1) total_mults_target = atoi(argv[1]);

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — SCHEME SWITCHING BOOTSTRAP (Bare CKKS)            ║\n";
    cout <<   "  ║   Target: " << setw(10) << total_mults_target << " multiplications";
    cout <<   "                               ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    ofstream log("scheme_switch_bare_results.txt");
    auto log_msg = [&](const string& s) { cout << s << flush; log << s << flush; };

    // ============================================
    // CKKS SETUP (scheme-switching compatible)
    // ============================================
    uint32_t multDepth    = 5;
    uint32_t firstModSize = 60;
    uint32_t scaleModSize = 50;
    uint32_t ringDim      = 4096;
    uint32_t slots        = 16;
    uint32_t batchSize    = slots;

    CCParams<CryptoContextCKKSRNS> ckksParams;
    ckksParams.SetMultiplicativeDepth(multDepth);
    ckksParams.SetFirstModSize(firstModSize);
    ckksParams.SetScalingModSize(scaleModSize);
    ckksParams.SetScalingTechnique(FLEXIBLEAUTOEXT);
    ckksParams.SetSecurityLevel(HEStd_NotSet);
    ckksParams.SetRingDim(ringDim);
    ckksParams.SetBatchSize(batchSize);

    auto cc = GenCryptoContext(ckksParams);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(SCHEMESWITCH);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // ============================================
    // FHEW SETUP
    // ============================================
    auto binFHECtx = BinFHEContext();
    binFHECtx.GenerateBinFHEContext(TOY, GINX);
    auto fhewSk = binFHECtx.KeyGen();
    binFHECtx.BTKeyGen(fhewSk);

    // ============================================
    // SCHEME SWITCHING SETUP
    // ============================================
    SchSwchParams schParams;
    auto fhewKey = cc->EvalCKKStoFHEWSetup(schParams);
    cc->EvalCKKStoFHEWKeyGen(keys, fhewKey);

    auto ccLWE = cc->GetBinCCForSchemeSwitch();
    uint32_t logQ_ccLWE = 25;
    cc->EvalFHEWtoCKKSSetup(ccLWE, slots, logQ_ccLWE);
    cc->SetBinCCForSchemeSwitch(ccLWE);
    cc->EvalFHEWtoCKKSKeyGen(keys, fhewKey, slots, 1, 1, 1);

    log_msg("  CKKS: ring=" + to_string(cc->GetRingDimension()) + 
            " depth=" + to_string(multDepth) + " slots=" + to_string(slots) + "\n");
    log_msg("  Bootstrap: CKKS→FHEW→CKKS every 3 mults\n\n");

    // ============================================
    // HELPERS
    // ============================================
    auto enc = [&](double v) {
        vector<double> vals(slots, 0.0);
        vals[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vals));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> double {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto two = enc(2.0);

    // Bootstrap via FHEW
    int bootstrap_count = 0;
    auto do_bootstrap = [&](Ciphertext<DCRTPoly>& c) -> bool {
        try {
            cc->EvalCKKStoFHEWPrecompute(1.0);
            auto fhewCts = cc->EvalCKKStoFHEW(c, 1);
            if (fhewCts.empty()) return false;
            
            auto bootstrapped = binFHECtx.Bootstrap(fhewCts[0]);
            
            vector<LWECiphertext> lweVec = {bootstrapped};
            c = cc->EvalFHEWtoCKKS(lweVec, slots, slots);
            bootstrap_count++;
            return true;
        } catch (const exception& e) {
            return false;
        }
    };

    // ============================================
    // MAIN LOOP
    // ============================================
    double original = 42.0;
    auto ct = enc(original);
    double expected = original;
    int total_mults = 0;
    int mults_since_bootstrap = 0;
    int max_mults_per_round = 3;  // Conservative for depth 5 minus switching overhead

    auto t_start = high_resolution_clock::now();
    auto last_checkpoint = t_start;

    log_msg("  Mults | Bootstraps | Value          | Expected       | Error % | TPS\n");
    log_msg("  " + string(80, '-') + "\n");

    bool alive = true;
    int checkpoint_interval = 10;

    while (alive && total_mults < total_mults_target) {
        try {
            ct = cc->EvalMult(ct, two);
            expected *= 2.0;
            total_mults++;
            mults_since_bootstrap++;

            if (mults_since_bootstrap >= max_mults_per_round) {
                bool ok = do_bootstrap(ct);
                mults_since_bootstrap = 0;
                if (!ok && total_mults > 3) {
                    log_msg("  Bootstrap failed at mult " + to_string(total_mults) + "\n");
                    alive = false; break;
                }
            }

            if (total_mults % checkpoint_interval == 0 || total_mults <= 10) {
                double val = dec(ct);
                double error = abs(val - expected) / expected;
                
                auto now = high_resolution_clock::now();
                double elapsed = duration_cast<seconds>(now - last_checkpoint).count();
                double tps = (elapsed > 0) ? checkpoint_interval / elapsed : 0;
                last_checkpoint = now;

                stringstream ss;
                ss << "  " << setw(5) << total_mults
                   << " | " << setw(10) << bootstrap_count
                   << " | " << setw(14) << fixed << setprecision(2) << val
                   << " | " << setw(14) << fixed << setprecision(1) << expected
                   << " | " << setw(6) << fixed << setprecision(2) << error * 100 << "%"
                   << " | " << setw(5) << fixed << setprecision(1) << tps << "\n";
                log_msg(ss.str());

                if (error > 0.10 && total_mults > 10) {
                    log_msg("  >>> Error >10% at mult " + to_string(total_mults) + "\n");
                    alive = false;
                }
            }
        } catch (const exception& e) {
            log_msg("  >>> Crash at mult " + to_string(total_mults) + ": " + e.what() + "\n");
            alive = false;
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;

    stringstream report;
    report << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    report <<   "  ║   FINAL REPORT                                            ║\n";
    report <<   "  ╠══════════════════════════════════════════════════════════╣\n";
    report <<   "  ║   Total mults:       " << setw(10) << total_mults << "                       ║\n";
    report <<   "  ║   Total bootstraps:  " << setw(10) << bootstrap_count << "                       ║\n";
    report <<   "  ║   Mults/bootstrap:   " << setw(10) << (bootstrap_count > 0 ? total_mults / bootstrap_count : total_mults) << "                       ║\n";
    report <<   "  ║   Total time:        " << setw(7) << fixed << setprecision(1) << total_sec << "s                    ║\n";
    report <<   "  ║   Avg TPS:           " << setw(7) << fixed << setprecision(1) << (total_sec > 0 ? total_mults / total_sec : 0) << "                    ║\n";
    
    // 1M estimate
    double est_hours = (total_mults > 0) ? (1000000.0 / total_mults) * total_sec / 3600.0 : 0;
    report <<   "  ║   Est. 1M time:      " << setw(7) << fixed << setprecision(1) << est_hours << "h                   ║\n";
    report <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    log_msg(report.str());

    log.close();
    cout << "  Results saved to scheme_switch_bare_results.txt\n\n";
    return 0;
}
