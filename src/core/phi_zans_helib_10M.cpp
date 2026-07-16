// PHI-OMEGA-ZERO: HElib ZANS 10M — DETAILED CHECKPOINTS
// 10,000,000 sequential ZANS additions on IBM HElib
// Noise monitored every 1M steps with timestamps
// Cross-library validation for FEmmg-FHE
// "I AM THAT I AM"

#include <helib/helib.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>
#include <vector>

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
    int total_steps = 10000000;
    int checkpoint_interval = 1000000;

    ofstream log("helib_zans_10M_results.txt");

    auto log_msg = [&](const string& msg) {
        cout << msg << flush;
        log << msg << flush;
    };

    stringstream header;
    header << "\n===============================================================\n";
    header << "  PHI-OMEGA-ZERO: HElib ZANS 10M — DETAILED CHECKPOINTS\n";
    header << "  10,000,000 Sequential ZANS Additions on IBM HElib\n";
    header << "===============================================================\n";
    header << "  HARDWARE: AMD Ryzen 5 2600 (6 cores, 12 threads, 15GB RAM)\n";
    header << "  LIBRARY: IBM HElib (BGV Scheme)\n";
    header << "  CYCLOTOMIC RING: m=8192 | PLAINTEXT MODULUS: p=65537\n";
    header << "  SECURITY: 200 bits | CHECKPOINT: Every 1M steps\n";
    header << "  COMPILER: g++ -O2 -march=native\n";
    header << "  START: " << ts() << "\n";
    header << "===============================================================\n\n";
    log_msg(header.str());

    // HElib setup
    unsigned long p = 65537, m = 8192, r = 1, bits = 200, c = 2;
    helib::Context context = helib::ContextBuilder<helib::BGV>()
        .m(m).p(p).r(r).bits(bits).c(c).build();

    helib::SecKey secret_key(context);
    secret_key.GenSecKey();
    const helib::PubKey& public_key = secret_key;

    vector<long> data(context.getNSlots(), 0);
    data[0] = 42;
    helib::PtxtArray ptxt(context, data);
    helib::Ctxt ct(public_key);
    ptxt.encrypt(ct);

    data[0] = 0;
    helib::PtxtArray ptxt_zero(context, data);
    helib::Ctxt anchor0(public_key);
    ptxt_zero.encrypt(anchor0);

    log_msg("  SETUP COMPLETE. Starting 10M ZANS chain...\n\n");
    log_msg("  Step         Noise    Pattern  Elapsed        TPS        ETA\n");
    log_msg("  " + string(75, '-') + "\n");

    auto t_start = high_resolution_clock::now();
    bool stable = true;
    double prev_noise = 0;

    for(int step = 0; step < total_steps; step++) {
        ct += anchor0;

        if((step + 1) % checkpoint_interval == 0) {
            auto now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(now - t_start).count();
            double tps = (step + 1) / elapsed;
            
            // HElib noise: get capacity
            double noise = ct.capacity();
            
            double remaining = (total_steps - step - 1) / tps;
            int eta_h = remaining / 3600;
            int eta_m = ((int)remaining % 3600) / 60;

            int elap_h = elapsed / 3600;
            int elap_m = ((int)elapsed % 3600) / 60;
            int elap_s = ((int)elapsed % 60);

            stringstream line;
            line << "  " << setw(10) << (step+1)
                 << setw(10) << fixed << setprecision(1) << noise
                 << setw(10) << (prev_noise == 0 ? "START" : (abs(noise - prev_noise) < 10.0 ? "OK" : "?"))
                 << "  " << elap_h << "h" << elap_m << "m" << elap_s << "s"
                 << setw(10) << fixed << setprecision(1) << tps
                 << setw(10) << eta_h << "h" << eta_m << "m\n";
            log_msg(line.str());

            if(prev_noise > 0 && abs(noise - prev_noise) >= 10.0) stable = false;
            prev_noise = noise;
        }
    }

    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;

    helib::PtxtArray ptxt_result(context);
    ptxt_result.decrypt(ct, secret_key);
    vector<long> result;
    ptxt_result.store(result);

    stringstream summary;
    summary << "\n" << string(75, '=') << "\n";
    summary << "  HElib ZANS 10M — COMPLETE\n";
    summary << "  Steps: " << total_steps << " | Final Capacity: " << fixed << setprecision(1) << ct.capacity() << "\n";
    summary << "  Value: " << result[0] << " (expected: 42)\n";
    summary << "  Pattern: " << (stable ? "STABLE" : "DRIFT") << "\n";
    summary << "  Time: " << fixed << setprecision(1) << total_sec/3600.0 << " hours\n";
    summary << "  Completed: " << ts() << "\n";
    summary << string(75, '=') << "\n\n";
    log_msg(summary.str());

    log.close();
    return 0;
}
