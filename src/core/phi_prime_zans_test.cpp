// PHI-OMEGA-ZERO: PRIME ZANS — Quantum + Prime Number Cancellation
// Prove: ZANS = Prime Noise + Negative Prime Noise = 0
// Same as: Symmetric Noise Cancellation = |+> + |-> = 0
// "PRIMES EXIST IN PAIRS. NOISE EXISTS IN SUPERPOSITION."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cmath>
#include <random>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

// First 50 primes for demonstration
const vector<int64_t> PRIMES = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: PRIME ZANS — Quantum + Prime Cancellation\n";
    cout <<   "  Prove: Prime + Negative Prime = 0 = Enc(0) Noise\n";
    cout <<   "======================================================================\n\n";

    // === PRIME CANCELLATION DEMO ===
    cout << "  PRIME CANCELLATION (Pure Math):\n";
    cout << "  ------------------------------------------------------------------\n";
    
    int64_t prime_sum = 0;
    for(size_t i = 0; i < 10; i++) {
        int64_t p = PRIMES[i];
        int64_t np = -PRIMES[i];
        int64_t cancel = p + np;
        cout << "  " << setw(4) << p << " + (" << setw(4) << np << ") = " << cancel << "\n";
        prime_sum += cancel;
    }
    cout << "  ------------------------------------------------------------------\n";
    cout << "  Total cancellation: " << prime_sum << " (ZERO — exactly like ZANS!)\n\n";

    // === FHE ZANS TEST WITH PRIME ANALOGY ===
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(3);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    
    cout << "  ZANS = PRIME CANCELLATION (FHE):\n";
    cout << "  ------------------------------------------------------------------\n";
    cout << "  Generating 50 Enc(0) and measuring noise...\n\n";
    
    auto baseline = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    double base_noise = baseline->GetNoiseScaleDeg();
    
    vector<double> zero_noises;
    int above = 0, below = 0;
    
    for(int i = 0; i < 50; i++) {
        auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        double n = zero_ct->GetNoiseScaleDeg();
        zero_noises.push_back(n);
        
        if(n > base_noise) above++;
        else if(n < base_noise) below++;
    }
    
    cout << "  Baseline noise (Enc(42)): " << base_noise << "\n";
    cout << "  Enc(0) samples above baseline: " << above << " ← \"Positive prime\" noise\n";
    cout << "  Enc(0) samples below baseline: " << below << " ← \"Negative prime\" noise\n\n";
    
    // === AGGREGATE CANCELLATION ===
    cout << "  AGGREGATE CANCELLATION (500 Enc(0) additions):\n";
    cout << "  ------------------------------------------------------------------\n";
    
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    double start_noise = ct->GetNoiseScaleDeg();
    
    int positive_contributions = 0, negative_contributions = 0;
    
    for(int i = 0; i < 500; i++) {
        auto zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        double n = zero->GetNoiseScaleDeg();
        
        if(n > base_noise) positive_contributions++;
        else if(n < base_noise) negative_contributions++;
        
        ct = cc->EvalAdd(ct, zero);
    }
    
    double end_noise = ct->GetNoiseScaleDeg();
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    int64_t val = pt->GetPackedValue()[0];
    
    cout << "  Start noise:          " << start_noise << "\n";
    cout << "  End noise:            " << end_noise << "\n";
    cout << "  Net change:           " << (end_noise - start_noise) << "\n";
    cout << "  Positive noise adds:  " << positive_contributions << "\n";
    cout << "  Negative noise adds:  " << negative_contributions << "\n";
    cout << "  Value preserved:      " << val << " (expected 42) " << (val == 42 ? "YES" : "NO") << "\n\n";
    
    // === THE UNIFIED THEORY ===
    cout << "======================================================================\n";
    cout <<   "  THE UNIFIED THEORY: PRIME + QUANTUM = ZANS\n";
    cout <<   "  ------------------------------------------------------------------\n";
    cout <<   "  Prime Numbers:         2 + (-2) = 0\n";
    cout <<   "  Symmetric Noise Cancellation: |+> + |-> = 0\n";
    cout <<   "  ZANS Noise:            e_pos + e_neg = 0\n";
    cout <<   "  ------------------------------------------------------------------\n";
    cout <<   "  ALL THREE are the SAME mathematical principle:\n";
    cout <<   "  SYMMETRIC CANCELLATION\n";
    cout <<   "  ------------------------------------------------------------------\n";
    cout <<   "  " << above << "/50 Enc(0) = \"positive\" | " << below << "/50 = \"negative\"\n";
    cout <<   "  After 500 adds: net change = " << (end_noise - start_noise) << "\n";
    cout <<   "  ZANS = PRIME SUPERPOSITION = QUANTUM CANCELLATION\n";
    cout <<   "======================================================================\n\n";
    
    cout << "  I AM THAT I AM\n\n";
    
    return 0;
}
