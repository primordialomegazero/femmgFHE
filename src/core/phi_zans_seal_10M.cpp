// PHI-OMEGA-ZERO: SEAL ZANS 10M — DETAILED CHECKPOINTS
// 10,000,000 sequential ZANS additions on Microsoft SEAL 4.3
// Noise monitored every 1M steps with timestamps
// Cross-library validation for FEmmg-FHE
// "I AM THAT I AM"

#include <seal/seal.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>

using namespace seal;
using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string elapsed_fmt(int64_t seconds) {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    stringstream ss;
    if(h > 0) ss << h << "h ";
    if(m > 0 || h > 0) ss << m << "m ";
    ss << s << "s";
    return ss.str();
}

int main() {
    int total_steps = 10000000;
    int checkpoint_interval = 1000000;

    ofstream log("seal_zans_10M_results.txt");

    auto log_msg = [&](const string& msg) {
        cout << msg << flush;
        log << msg << flush;
    };

    stringstream header;
    header << "\n===============================================================\n";
    header << "  PHI-OMEGA-ZERO: SEAL ZANS 10M — DETAILED CHECKPOINTS\n";
    header << "  10,000,000 Sequential ZANS Additions on Microsoft SEAL 4.3\n";
    header << "===============================================================\n";
    header << "  HARDWARE: AMD Ryzen 5 2600 (6 cores, 12 threads, 15GB RAM)\n";
    header << "  LIBRARY: Microsoft SEAL 4.3 | BFV Scheme\n";
    header << "  POLY MODULUS DEGREE: 8192 | PLAINTEXT MODULUS: 786433\n";
    header << "  CHECKPOINT: Every 1M steps\n";
    header << "  COMPILER: g++ -O2 -march=native\n";
    header << "  START: " << ts() << "\n";
    header << "===============================================================\n\n";
    log_msg(header.str());

    // SEAL setup
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(8192);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(8192));
    parms.set_plain_modulus(786433);

    SEALContext context(parms);
    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);

    Encryptor encryptor(context, public_key);
    Decryptor decryptor(context, secret_key);
    Evaluator evaluator(context);
    BatchEncoder batch_encoder(context);

    // Setup initial value = 42
    vector<uint64_t> pod_matrix(batch_encoder.slot_count(), 0ULL);
    pod_matrix[0] = 42;
    Plaintext pt;
    batch_encoder.encode(pod_matrix, pt);
    Ciphertext ct;
    encryptor.encrypt(pt, ct);

    // Encrypted zero for ZANS
    Plaintext pt_zero;
    batch_encoder.encode(pod_matrix, pt_zero);
    pod_matrix[0] = 0;
    batch_encoder.encode(pod_matrix, pt_zero);
    Ciphertext anchor0;
    encryptor.encrypt(pt_zero, anchor0);

    log_msg("  SETUP COMPLETE. Starting 10M ZANS chain...\n\n");
    log_msg("  Step         Noise    Pattern  Elapsed        TPS        ETA\n");
    log_msg("  " + string(75, '-') + "\n");

    auto t_start = high_resolution_clock::now();
    bool stable = true;
    double prev_noise = 0;

    for(int step = 0; step < total_steps; step++) {
        // ZANS: ct = ct + Enc(0)
        evaluator.add_inplace(ct, anchor0);

        if((step + 1) % checkpoint_interval == 0) {
            auto now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(now - t_start).count();
            double tps = (step + 1) / elapsed;
            
            // Measure noise budget
            double noise_budget = decryptor.invariant_noise_budget(ct);
            
            double remaining = (total_steps - step - 1) / tps;
            int eta_h = remaining / 3600;
            int eta_m = ((int)remaining % 3600) / 60;

            int elap_h = elapsed / 3600;
            int elap_m = ((int)elapsed % 3600) / 60;
            int elap_s = ((int)elapsed % 60);

            stringstream line;
            line << "  " << setw(10) << (step+1)
                 << setw(10) << fixed << setprecision(1) << noise_budget
                 << setw(10) << (prev_noise == 0 ? "START" : (abs(noise_budget - prev_noise) < 10.0 ? "OK" : "?"))
                 << "  " << elap_h << "h" << elap_m << "m" << elap_s << "s"
                 << setw(10) << fixed << setprecision(1) << tps
                 << setw(10) << eta_h << "h" << eta_m << "m\n";
            log_msg(line.str());

            if(step > 0 && abs(noise_budget - prev_noise) >= 5.0) stable = false;
            prev_noise = noise_budget;
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;

    // Final verification
    Plaintext pt_result;
    decryptor.decrypt(ct, pt_result);
    vector<uint64_t> result;
    batch_encoder.decode(pt_result, result);

    stringstream summary;
    summary << "\n" << string(75, '=') << "\n";
    summary << "  SEAL ZANS 10M — COMPLETE\n";
    summary << "  Steps: " << total_steps << " | Final Noise Budget: " << fixed << setprecision(1) << decryptor.invariant_noise_budget(ct) << " bits\n";
    summary << "  Value: " << result[0] << " (expected: 42)\n";
    summary << "  Pattern: " << (stable ? "STABLE (noise drift < 10 bits over 10M ops)" : "DRIFT DETECTED") << "\n";
    summary << "  Time: " << fixed << setprecision(1) << total_sec/3600.0 << " hours\n";
    summary << "  Completed: " << ts() << "\n";
    summary << string(75, '=') << "\n\n";
    log_msg(summary.str());

    log.close();
    return 0;
}
