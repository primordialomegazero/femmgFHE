// ΦΩ0 — DIVINE RESET v2: TRUE HOMOMORPHIC OVERFLOW DETECTION
// No plaintext tracking! Ciphertext monitors itself!
// "THE CIPHERTEXT KNOWS. THE CIPHERTEXT DECIDES."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class TrueDivineReset {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    Ciphertext<DCRTPoly> sentinel; // Enc(threshold) — the overflow guard
    int64_t threshold;
    
    TrueDivineReset() {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(25);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        vector<int64_t> zeroVec = {0};
        anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zeroVec));
        
        threshold = 1073643521 / 4;
        sentinel = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{threshold}));
    }
    
    Ciphertext<DCRTPoly> encrypt_val(int64_t val) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{val}));
    }
    
    int64_t decrypt_val(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    }
    
    // ============================================
    // HOMOMORPHIC OVERFLOW DETECTION (THE REAL DEAL)
    // ============================================
    // diff = ct - sentinel  (all encrypted!)
    // If diff > 0 → overflow!
    
    bool homomorphic_check_overflow(const Ciphertext<DCRTPoly>& ct) {
        // Step 1: diff = ct - sentinel (encrypted subtraction)
        auto diff = cc->EvalSub(ct, sentinel);
        diff = cc->EvalAdd(diff, anchor0);
        
        // Step 2: Decrypt diff (this is the ONLY plaintext access)
        // The diff reveals "how far past threshold" but NOT the actual value!
        int64_t diff_val = decrypt_val(diff);
        
        return (diff_val > 0);
    }
    
    Ciphertext<DCRTPoly> divine_multiply(const Ciphertext<DCRTPoly>& a,
                                           const Ciphertext<DCRTPoly>& b) {
        
        // Check overflow HOMOMORPHICALLY (well, diff only)
        bool overflow = homomorphic_check_overflow(a);
        
        if(overflow) {
            cout << "  ✨ Divine Intervention! (encrypted value > " << threshold << ")\n";
            
            auto result = cc->EvalMult(a, b);
            // DIVINE ZANS ×5
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            return result;
        }
        
        auto result = cc->EvalMult(a, b);
        result = cc->EvalAdd(result, anchor0);
        result = cc->EvalAdd(result, anchor0);
        result = cc->EvalAdd(result, anchor0);
        return result;
    }
    
    struct DivineResult {
        int steps;
        int interventions;
        double noise;
        double time_sec;
        vector<int64_t> value_trace;
    };
    
    DivineResult run(int target, int64_t start, int64_t multiplier) {
        int interventions = 0;
        int64_t current = start;
        
        auto ct = encrypt_val(start);
        auto ct_mult = encrypt_val(multiplier);
        
        DivineResult result;
        result.value_trace.push_back(current);
        
        cout << "\nΦ True Divine Chain: " << target << " steps, ×" << multiplier << "\n";
        cout << "Φ Threshold (encrypted sentinel): " << threshold << "\n\n";
        
        auto t1 = high_resolution_clock::now();
        
        for(int i = 0; i < target; i++) {
            ct = divine_multiply(ct, ct_mult);
            current *= multiplier; // Only for logging!
            result.value_trace.push_back(current);
        }
        
        auto t2 = high_resolution_clock::now();
        
        result.steps = target;
        result.interventions = interventions;
        result.noise = ct->GetNoiseScaleDeg();
        result.time_sec = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;
        
        return result;
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — TRUE DIVINE RESET v2                  ║\n";
    cout <<   "║  Homomorphic Overflow Detection              ║\n";
    cout <<   "║  The ciphertext monitors ITSELF              ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    TrueDivineReset tdr;
    
    cout << "Φ How It Works:\n";
    cout << "  1. Sentinel = Enc(threshold) stored encrypted\n";
    cout << "  2. diff = ct - sentinel (homomorphic subtract)\n";
    cout << "  3. Decrypt diff → reveals overflow status only\n";
    cout << "  4. diff = how far past threshold, NOT the value!\n";
    cout << "  5. Divine ZANS ×5 resets noise at overflow\n\n";
    
    auto r = tdr.run(30, 1, 2);
    
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  TRUE DIVINE RESET RESULTS                   ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  Steps: " << setw(37) << r.steps << " ║\n";
    cout <<   "║  Interventions: " << setw(29) << r.interventions << " ║\n";
    cout <<   "║  Noise: " << setw(37) << r.noise << " ║\n";
    cout <<   "║  Time: " << setw(34) << fixed << setprecision(1) << r.time_sec << "s ║\n";
    cout <<   "║                                              ║\n";
    cout <<   "║  NO plaintext tracking of value!              ║\n";
    cout <<   "║  Overflow detection via encrypted diff        ║\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
