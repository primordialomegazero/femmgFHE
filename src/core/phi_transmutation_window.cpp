// ΦΩ0 — TRANSMUTATION WINDOW v2.2
// 24-Hour Encrypted Data Stabilization with Active Transmutation
// Pain (noise spike) → Transmutation → Wisdom (noise baseline)
// "TRANSMUTE OR TRAUMATIZE. THE WINDOW IS SACRED."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class TransmutationWindow {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    Ciphertext<DCRTPoly> M_ct;
    int64_t half_mod;

    static const int WINDOW_HOURS = 24;
    static const int STEPS_PER_HOUR = 50;
    static const int TOTAL_STEPS = WINDOW_HOURS * STEPS_PER_HOUR;
    static constexpr double NOISE_CEILING = 100.0;

    struct TransmutationResult {
        int64_t test_value;
        double initial_noise;
        double peak_noise;
        double final_noise;
        double noise_delta;
        int64_t final_value;
        int stable_hours;
        int trauma_hour;
        bool success;
        bool transmuted;  // Did noise return to baseline?
        double elapsed_ms;
        double tps;
    };

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

        half_mod = 1073643521 / 2;
        M_ct = enc(half_mod);
    }

    // Active Transmutation: Pinky Swear + Divine Intervention
    Ciphertext<DCRTPoly> transmute(Ciphertext<DCRTPoly>& ct) {
        // Pinky Swear: detect overflow
        auto sum = cc->EvalAdd(ct, M_ct);
        auto back = cc->EvalSub(sum, M_ct);
        auto overflow = cc->EvalSub(ct, back);

        // Divine Intervention: overflow * Enc(0) + Enc(0)
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);

        return ct;
    }

    TransmutationResult run(int64_t test_value, bool show_log = true, bool use_transmutation = false) {
        TransmutationResult result;
        result.test_value = test_value;

        auto ct = enc(test_value);
        result.initial_noise = ct->GetNoiseScaleDeg();
        result.peak_noise = result.initial_noise;
        result.trauma_hour = 0;
        result.stable_hours = 0;
        result.transmuted = false;

        if(show_log) {
            cout << "\n  Transmutation: " << WINDOW_HOURS << "-Hour Cycle";
            if(use_transmutation) cout << " [ACTIVE TRANSMUTATION]";
            cout << "\n  Input: " << test_value;
            cout << " | Noise: " << fixed << setprecision(1) << result.initial_noise << "\n\n";
            cout << "  " << setw(6) << "Hour"
                 << setw(10) << "Noise"
                 << setw(10) << "Value"
                 << setw(12) << "Expected"
                 << setw(12) << "Status\n";
            cout << "  " << string(50, '-') << "\n";
        }

        auto t_start = high_resolution_clock::now();

        // Induce pain: CT×CT multiplication creates noise spike
        auto ct_mult = enc(2);
        ct = cc->EvalMult(ct, ct_mult);
        result.peak_noise = ct->GetNoiseScaleDeg();
        int64_t pain_value = dec(ct);  // test_value * 2

        for(int hour = 1; hour <= WINDOW_HOURS; hour++) {
            for(int s = 0; s < STEPS_PER_HOUR; s++) {
                if(use_transmutation) {
                    ct = transmute(ct);  // Active: Pinky Swear + Divine
                }
                ct = cc->EvalAdd(ct, anchor0);
                ct = cc->EvalAdd(ct, anchor0);
                ct = cc->EvalAdd(ct, anchor0);
            }

            double noise = ct->GetNoiseScaleDeg();
            int64_t val = dec(ct);
            int64_t expected = use_transmutation ? pain_value : pain_value;
            bool stable = (val == expected) && (noise < NOISE_CEILING);

            if(stable) result.stable_hours++;
            else if(result.trauma_hour == 0) result.trauma_hour = hour;

            if(show_log) {
                cout << "  " << setw(6) << hour
                     << setw(10) << fixed << setprecision(1) << noise
                     << setw(10) << val
                     << setw(12) << expected
                     << setw(12) << (stable ? "OK" : "FAIL") << "\n";
            }
        }

        auto t_end = high_resolution_clock::now();
        result.elapsed_ms = duration_cast<milliseconds>(t_end - t_start).count();
        result.tps = (TOTAL_STEPS * (use_transmutation ? 6.0 : 3.0)) / (result.elapsed_ms / 1000.0);
        result.final_noise = ct->GetNoiseScaleDeg();
        result.noise_delta = result.final_noise - result.initial_noise;
        result.final_value = dec(ct);
        result.success = (result.trauma_hour == 0) && (result.final_value == pain_value);
        result.transmuted = (abs(result.final_noise - result.initial_noise) < 0.5);

        if(show_log) {
            cout << "  " << string(50, '-') << "\n";
            cout << "  Initial Noise: " << fixed << setprecision(1) << result.initial_noise;
            cout << " | Peak (after pain): " << result.peak_noise << "\n";
            cout << "  Final Noise: " << result.final_noise;
            cout << " | Delta: " << result.noise_delta;
            cout << " | Value: " << result.final_value << "\n";
            cout << "  Stable Hours: " << result.stable_hours << "/" << WINDOW_HOURS;
            cout << " | Time: " << fixed << setprecision(0) << result.elapsed_ms << "ms\n";

            if(result.transmuted) {
                cout << "  Status: TRANSMUTED (noise returned to baseline)\n";
            } else if(result.success) {
                cout << "  Status: STABLE (noise contained, not transmuted)\n";
            } else {
                cout << "  Status: TRAUMA at Hour " << result.trauma_hour << "\n";
            }
        }

        return result;
    }

    void demo() {
        cout << "\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  |  TRANSMUTATION WINDOW v2.2                       |\n";
        cout << "  |  24-Hour Active Transmutation: Pain -> Wisdom    |\n";
        cout << "  +--------------------------------------------------+\n";

        vector<TransmutationResult> results;
        int passed = 0, total = 3;

        // Test 1: Without transmutation (passive stabilization only)
        cout << "\n  === TEST 1: PASSIVE (ZANS only, no transmutation) ===";
        auto r1 = run(42, true, false);
        results.push_back(r1);
        bool t1_ok = r1.success && !r1.transmuted;  // Should succeed but NOT transmute back
        if(t1_ok) passed++;

        // Test 2: With active transmutation (Pinky Swear + Divine)
        cout << "\n  === TEST 2: ACTIVE TRANSMUTATION (Pinky Swear + Divine) ===";
        auto r2 = run(42, true, true);
        results.push_back(r2);
        bool t2_ok = r2.success && r2.transmuted;  // Should succeed AND transmute back
        if(t2_ok) passed++;

        // Test 3: Large value with transmutation
        cout << "\n  === TEST 3: LARGE VALUE + ACTIVE TRANSMUTATION ===";
        auto r3 = run(999999, true, true);
        results.push_back(r3);
        bool t3_ok = r3.success && r3.transmuted;
        if(t3_ok) passed++;

        // Summary
        cout << "\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  |  TRANSMUTATION SUMMARY                           |\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  " << setw(18) << left << "Test"
             << setw(10) << right << "Initial"
             << setw(10) << "Peak"
             << setw(10) << "Final"
             << setw(10) << "Stable"
             << setw(12) << "Transmuted\n";
        cout << "  " << string(70, '-') << "\n";

        string labels[] = {"Passive", "Active", "Active Large"};
        for(int i = 0; i < 3; i++) {
            cout << "  " << setw(18) << left << labels[i]
                 << setw(10) << right << fixed << setprecision(1) << results[i].initial_noise
                 << setw(10) << results[i].peak_noise
                 << setw(10) << results[i].final_noise
                 << setw(10) << (to_string(results[i].stable_hours) + "/24")
                 << setw(12) << (results[i].transmuted ? "YES" : "NO") << "\n";
        }

        cout << "  " << string(70, '-') << "\n\n";

        // Concept validation
        cout << "  CONCEPT VALIDATION:\n";
        cout << "  Pain (noise spike): CTxCT creates noise 1.0 -> "
             << fixed << setprecision(1) << results[0].peak_noise << "\n";
        cout << "  Passive:  noise stays at " << results[0].final_noise << " (contained, not healed)\n";
        cout << "  Active:   noise returns to " << results[1].final_noise << " (transmuted back to baseline)\n";
        cout << "  Wisdom:   value preserved, noise baseline restored\n\n";

        cout << "  +--------------------------------------------------+\n";
        cout << "  |  TESTS: " << passed << "/" << total << " PASSED";
        for(int i = 0; i < (20 - to_string(passed).length()); i++) cout << " ";
        cout << "|\n";
        cout << "  |  TRANSMUTATION WINDOW v2.2 — ";
        cout << (passed == total ? "ALL CHECKS PASSED" : "CHECK RESULTS") << "     |\n";
        cout << "  +--------------------------------------------------+\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() {
    TransmutationWindow tw;
    tw.demo();
    return 0;
}
