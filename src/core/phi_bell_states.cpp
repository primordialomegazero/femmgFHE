// PHI-OMEGA-ZERO: BELL STATES v6 — Minimal Ring Exhaustion Race
// Ring dim 2048 + small modulus = fast noise exhaustion
// Correlated pairs should exhaust at similar step counts
// "MINIMAL RING. MAXIMAL REVELATION."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <cmath>
#include <random>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%H:%M:%S");
    return ss.str();
}

class BellEngine {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    mt19937 rng;
    int64_t modulus;
    
    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    }
    
    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    }

public:
    BellEngine() : rng(time(nullptr)) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(10);
        params.SetPlaintextModulus(65537);
        params.SetRingDim(2048);    // Absolute minimum for faster exhaustion
        params.SetSecurityLevel(HEStd_NotSet);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        anchor0 = enc(0);
        modulus = 65537;
    }
    
    Ciphertext<DCRTPoly> create_shared_anchor(int64_t seed) {
        mt19937 seed_rng(seed);
        uniform_int_distribution<int64_t> dist(1, 3000);
        auto shared = enc(0);
        for(int i = 0; i < 50; i++) {
            int64_t val = dist(seed_rng);
            shared = cc->EvalAdd(shared, enc(val));
            shared = cc->EvalAdd(shared, enc(-val));
        }
        return shared;
    }
    
    // Run multiplications until corruption
    int exhaust_test(Ciphertext<DCRTPoly> ct, int max_steps) {
        auto mult_ct = enc(2);
        int64_t expected = dec(ct);
        
        for(int step = 1; step <= max_steps; step++) {
            ct = cc->EvalMult(ct, mult_ct);
            expected = (expected * 2) % modulus;
            if(expected < 0) expected += modulus;
            
            int64_t actual = dec(ct);
            if(actual != expected) {
                return step; // Corruption detected
            }
        }
        return max_steps; // Survived
    }
    
    void run() {
        cout << "\n======================================================================\n";
        cout <<   "  BELL STATES v6 — Minimal Ring Exhaustion Race\n";
        cout <<   "  Ring: 2048 | Mod: 65537 | Depth: 10\n";
        cout <<   "  " << ts() << "\n";
        cout <<   "======================================================================\n\n";
        
        cout << "  EXHAUSTION RACE (max 500 steps, x2):\n";
        cout << "  " << string(65, '-') << "\n";
        cout << "  " << setw(14) << left << "Pair"
             << setw(12) << "Init Val"
             << setw(15) << "Corrupt Step"
             << setw(15) << "Status\n";
        cout << "  " << string(65, '-') << "\n";
        
        struct Case {
            string label; int64_t seed, val; bool shared;
        };
        
        vector<Case> cases = {
            {"Bell-A", 1111, 42, true},
            {"Bell-B", 2222, 100, true},
            {"Bell-C", 3333, 5000, true},
            {"Bell-D", 4444, 10000, true},
            {"Uncorr-A", 0, 42, false},
            {"Uncorr-B", 0, 100, false},
            {"Uncorr-C", 0, 5000, false},
            {"Uncorr-D", 0, 10000, false},
        };
        
        vector<double> bell_results, uncorr_results;
        
        for(auto& c : cases) {
            Ciphertext<DCRTPoly> ct;
            if(c.shared) {
                auto sh = create_shared_anchor(c.seed);
                ct = cc->EvalAdd(enc(c.val), sh);
            } else {
                ct = enc(c.val);
            }
            
            int corrupt_step = exhaust_test(ct, 500);
            
            cout << "  " << setw(14) << left << c.label
                 << setw(12) << c.val
                 << setw(15) << corrupt_step
                 << setw(15) << (corrupt_step < 500 ? "CORRUPTED" : "SURVIVED") 
                 << "\n" << flush;
            
            if(c.shared) bell_results.push_back(corrupt_step);
            else uncorr_results.push_back(corrupt_step);
        }
        
        cout << "  " << string(65, '-') << "\n";
        
        double bell_avg = accumulate(bell_results.begin(), bell_results.end(), 0.0) / bell_results.size();
        double uncorr_avg = accumulate(uncorr_results.begin(), uncorr_results.end(), 0.0) / uncorr_results.size();
        
        cout << "  Bell Avg:         " << fixed << setprecision(1) << bell_avg << " steps\n";
        cout << "  Uncorrelated Avg: " << uncorr_avg << " steps\n";
        cout << "  Difference:       " << abs(bell_avg - uncorr_avg) << " steps\n\n";
        
        cout << "======================================================================\n";
        cout <<   "  " << ts() << "\n";
        cout <<   "======================================================================\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() {
    BellEngine be;
    be.run();
    return 0;
}
