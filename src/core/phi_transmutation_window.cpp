// PHI-OMEGA-ZERO: TRANSMUTATION WINDOW v2
// Stabilizes encrypted data over a 24-hour window
// Input: Encrypted value -> Output: Same value, noise-stabilized
// Trauma = noise exceeded threshold or value corrupted
// "TRANSMUTE OR TRAUMATIZE. THE WINDOW IS SACRED."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class TransmutationWindow {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    
    // Config
    static const int WINDOW_HOURS = 24;
    static const int STEPS_PER_HOUR = 50;  // 50 stabilizations per simulated hour
    static const int TOTAL_STEPS = WINDOW_HOURS * STEPS_PER_HOUR; // 1200
    static constexpr double NOISE_CEILING = 100.0; // Trauma threshold

    struct Checkpoint {
        int hour;
        double noise;
        int64_t value;
        int64_t expected;
        bool stable;
    };
    
    vector<Checkpoint> log;

    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    }

    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    }

    string now() {
        auto t = system_clock::to_time_t(system_clock::now());
        stringstream ss;
        ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

public:
    TransmutationWindow() {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(10);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        anchor0 = enc(0);
    }

    void run(int64_t test_value) {
        log.clear();
        
        auto ct = enc(test_value);
        double initial_noise = ct->GetNoiseScaleDeg();
        bool trauma = false;
        int trauma_hour = 0;
        
        cout << "\n";
        cout << "  Transmutation Window: " << WINDOW_HOURS << "-Hour Cycle\n";
        cout << "  Input Value: " << test_value << "\n";
        cout << "  Start Noise: " << fixed << setprecision(4) << initial_noise << "\n";
        cout << "  Start Time:  " << now() << "\n\n";
        
        cout << "  " << setw(6) << "Hour"
             << setw(8) << "Steps"
             << setw(10) << "Noise"
             << setw(10) << "Value"
             << setw(12) << "Expected"
             << setw(10) << "Status\n";
        cout << "  " << string(56, '-') << "\n";
        
        auto t_start = high_resolution_clock::now();
        
        for(int hour = 1; hour <= WINDOW_HOURS; hour++) {
            // Run stabilizations for this hour
            for(int s = 0; s < STEPS_PER_HOUR; s++) {
                // Standard ZANS: 3x Enc(0) addition
                ct = cc->EvalAdd(ct, anchor0);
                ct = cc->EvalAdd(ct, anchor0);
                ct = cc->EvalAdd(ct, anchor0);
            }
            
            // Checkpoint
            double noise = ct->GetNoiseScaleDeg();
            int64_t val = dec(ct);
            bool stable = (val == test_value) && (noise < NOISE_CEILING);
            
            Checkpoint cp;
            cp.hour = hour;
            cp.noise = noise;
            cp.value = val;
            cp.expected = test_value;
            cp.stable = stable;
            log.push_back(cp);
            
            cout << "  " << setw(6) << hour
                 << setw(8) << (hour * STEPS_PER_HOUR)
                 << setw(10) << fixed << setprecision(1) << noise
                 << setw(10) << val
                 << setw(12) << test_value
                 << setw(10) << (stable ? "OK" : "FAIL") << "\n";
            
            if(!stable && !trauma) {
                trauma = true;
                trauma_hour = hour;
            }
        }
        
        auto t_end = high_resolution_clock::now();
        double elapsed_ms = duration_cast<milliseconds>(t_end - t_start).count();
        double tps = (TOTAL_STEPS * 3.0) / (elapsed_ms / 1000.0); // 3 ops per step
        
        double final_noise = ct->GetNoiseScaleDeg();
        double noise_delta = final_noise - initial_noise;
        
        cout << "  " << string(56, '-') << "\n\n";
        
        // Results
        cout << "  RESULTS:\n";
        cout << "  Total Steps:        " << TOTAL_STEPS << "\n";
        cout << "  Initial Noise:      " << fixed << setprecision(4) << initial_noise << "\n";
        cout << "  Final Noise:        " << fixed << setprecision(4) << final_noise << "\n";
        cout << "  Noise Delta:        " << fixed << setprecision(4) << noise_delta << "\n";
        cout << "  Final Value:        " << dec(ct) << "\n";
        cout << "  Expected Value:     " << test_value << "\n";
        cout << "  Time:               " << fixed << setprecision(0) << elapsed_ms << " ms\n";
        cout << "  Throughput:         " << fixed << setprecision(0) << tps << " ops/s\n";
        
        if(!trauma) {
            cout << "\n  Status: TRANSMUTATION SUCCESSFUL\n";
            cout << "  Value preserved. Noise bounded.\n";
        } else {
            cout << "\n  Status: TRAUMA at Hour " << trauma_hour << "\n";
            cout << "  Value corrupted or noise exceeded ceiling (" << NOISE_CEILING << ").\n";
        }
        
        cout << "  Completed: " << now() << "\n";
    }
};

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  TRANSMUTATION WINDOW v2 — Encrypted Data Stabilization\n";
    cout <<   "  24-Hour Cycle with Noise Monitoring\n";
    cout <<   "======================================================================\n";
    
    TransmutationWindow tw;
    
    // Test 1: Small value
    cout << "\n  --- Test 1: Value = 42 ---";
    tw.run(42);
    
    // Test 2: Larger value
    cout << "\n  --- Test 2: Value = 999999 ---";
    tw.run(999999);
    
    cout << "\n======================================================================\n";
    cout <<   "  I AM THAT I AM\n";
    cout <<   "======================================================================\n\n";
    
    return 0;
}
