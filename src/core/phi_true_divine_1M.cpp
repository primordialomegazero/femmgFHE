#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>

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
    int divine_interval = 5;
    int checkpoint_interval = 100000;
    
    ofstream log("true_divine_1M_results.txt");
    
    auto log_msg = [&](const string& msg) {
        cout << msg << flush;
        log << msg << flush;
    };
    
    stringstream header;
    header << "\n===============================================================\n";
    header << "  PHI-OMEGA-ZERO: TRUE DIVINE 1M\n";
    header << "  1,000,000 Sequential CTxCT Multiplications\n";
    header << "===============================================================\n";
    header << "  HARDWARE: AMD Ryzen 5 2600 (6 cores, 12 threads, 15GB RAM)\n";
    header << "  RING DIM: 4096 | MODULUS: 1073643521 | DEPTH: 30\n";
    header << "  DIVINE: Every 5 steps | CHECKPOINT: Every 100K\n";
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
    
    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    
    auto anchor0 = enc(0);
    auto M = enc(half_mod);
    auto ct = enc(1);
    auto ct_mult = enc(2);
    
    log_msg("  SETUP COMPLETE. Starting 1M chain...\n\n");
    log_msg("  Step         Noise    Pattern  Elapsed        TPS        ETA\n");
    log_msg("  " + string(75, '-') + "\n");
    
    auto t_start = high_resolution_clock::now();
    bool stable = true;
    
    for(int step = 0; step < total_steps; step++) {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        
        if((step + 1) % divine_interval == 0) {
            auto divine = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, divine);
            ct = cc->EvalAdd(ct, anchor0);
        }
        ct = cc->EvalAdd(ct, anchor0);
        
        if((step + 1) % checkpoint_interval == 0) {
            auto now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(now - t_start).count();
            double tps = (step + 1) / elapsed;
            double noise = ct->GetNoiseScaleDeg();
            double expected = step + 1.0;
            
            double remaining = (total_steps - step - 1) / tps;
            int eta_h = remaining / 3600;
            int eta_m = ((int)remaining % 3600) / 60;
            
            int elap_h = elapsed / 3600;
            int elap_m = ((int)elapsed % 3600) / 60;
            int elap_s = ((int)elapsed % 60);
            
            stringstream line;
            line << "  " << setw(10) << (step+1)
                 << setw(10) << fixed << setprecision(0) << noise
                 << setw(10) << (abs(noise - expected) < 2.0 ? "OK" : "?")
                 << "  " << elap_h << "h" << elap_m << "m" << elap_s << "s"
                 << setw(10) << fixed << setprecision(1) << tps
                 << setw(10) << eta_h << "h" << eta_m << "m\n";
            log_msg(line.str());
            
            if(abs(noise - expected) >= 2.0) stable = false;
        }
    }
    
    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;
    double final_noise = ct->GetNoiseScaleDeg();
    
    stringstream summary;
    summary << "\n" << string(75, '=') << "\n";
    summary << "  TRUE DIVINE 1M — COMPLETE\n";
    summary << "  Steps: " << total_steps << " | Final Noise: " << fixed << setprecision(0) << final_noise << "\n";
    summary << "  Pattern: " << (stable ? "LINEAR (Noise = Step + 1)" : "DEVIATION") << "\n";
    summary << "  Time: " << fixed << setprecision(1) << total_sec/3600.0 << " hours\n";
    summary << "  Completed: " << ts() << "\n";
    summary << string(75, '=') << "\n\n";
    log_msg(summary.str());
    
    log.close();
    return 0;
}
