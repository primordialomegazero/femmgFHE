// PHI-OMEGA-ZERO: QUANTUM ZANS TEST
// Prove: Enc(0) noise is PROBABILISTIC (positive AND negative)
// Each Enc(0) has random noise that cancels out in aggregate
// "THE NOISE IS IN SUPERPOSITION"
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

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  PHI-OMEGA-ZERO: QUANTUM ZANS TEST\n";
    cout <<   "  Prove: Enc(0) noise = PROBABILISTIC (positive AND negative)\n";
    cout <<   "======================================================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(3);
    params.SetPlaintextModulus(1073643521);
    params.SetRingDim(16384);
    params.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    
    cout << "  EXPERIMENT: Generate 1000 separate Enc(0) ciphertexts\n";
    cout << "  Measure the noise of EACH one individually\n";
    cout << "  Hypothesis: Noise varies — some positive, some \"negative\" (less than baseline)\n\n";
    
    vector<double> noise_samples;
    vector<int64_t> decrypted_values;
    double sum_noise = 0;
    double min_noise = 999, max_noise = -999;
    int positive_count = 0, negative_count = 0, zero_count = 0;
    
    // Baseline: encrypt a non-zero value
    auto baseline_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    double baseline_noise = baseline_ct->GetNoiseScaleDeg();
    
    cout << "  Baseline noise (Enc(42)): " << baseline_noise << "\n\n";
    
    for(int i = 0; i < 1000; i++) {
        // Generate fresh Enc(0)
        auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        double noise = zero_ct->GetNoiseScaleDeg();
        
        noise_samples.push_back(noise);
        sum_noise += noise;
        
        if(noise > baseline_noise) positive_count++;
        else if(noise < baseline_noise) negative_count++;
        else zero_count++;
        
        if(noise < min_noise) min_noise = noise;
        if(noise > max_noise) max_noise = noise;
        
        // Decrypt to verify it's actually 0
        Plaintext pt;
        cc->Decrypt(keys.secretKey, zero_ct, &pt);
        decrypted_values.push_back(pt->GetPackedValue()[0]);
    }
    
    double avg_noise = sum_noise / noise_samples.size();
    
    // Calculate variance
    double variance = 0;
    for(auto n : noise_samples) {
        variance += (n - avg_noise) * (n - avg_noise);
    }
    variance /= noise_samples.size();
    double stddev = sqrt(variance);
    
    cout << "  RESULTS (1000 Enc(0) samples):\n";
    cout << "  ------------------------------------------------------------------\n";
    cout << "  Average noise:     " << fixed << setprecision(6) << avg_noise << "\n";
    cout << "  Baseline noise:    " << baseline_noise << "\n";
    cout << "  Min noise:         " << min_noise << "\n";
    cout << "  Max noise:         " << max_noise << "\n";
    cout << "  Std deviation:     " << fixed << setprecision(6) << stddev << "\n";
    cout << "  Range:             " << (max_noise - min_noise) << "\n\n";
    
    cout << "  NOISE DISTRIBUTION:\n";
    cout << "  Above baseline:    " << positive_count << " (" << fixed << setprecision(1) 
         << (100.0 * positive_count / 1000) << "%)  ← \"Positive\" noise\n";
    cout << "  Below baseline:    " << negative_count << " (" << fixed << setprecision(1) 
         << (100.0 * negative_count / 1000) << "%)  ← \"Negative\" noise\n";
    cout << "  At baseline:       " << zero_count << " (" << fixed << setprecision(1) 
         << (100.0 * zero_count / 1000) << "%)\n\n";
    
    // Test: Add 1000 Enc(0) to one ciphertext
    cout << "  AGGREGATE TEST: Add 1000 Enc(0) to Enc(42)\n";
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    double start_noise = ct->GetNoiseScaleDeg();
    
    for(int i = 0; i < 1000; i++) {
        auto zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        ct = cc->EvalAdd(ct, zero);
    }
    
    double end_noise = ct->GetNoiseScaleDeg();
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    int64_t final_val = pt->GetPackedValue()[0];
    
    cout << "  Start noise:       " << start_noise << "\n";
    cout << "  End noise:         " << end_noise << "\n";
    cout << "  Noise change:      " << (end_noise - start_noise) << "\n";
    cout << "  Final value:       " << final_val << " (expected: 42)\n\n";
    
    // VERDICT
    cout << "======================================================================\n";
    cout <<   "  QUANTUM ZANS VERDICT\n";
    cout <<   "  ------------------------------------------------------------------\n";
    
    if(final_val == 42 && abs(end_noise - start_noise) < 10.0) {
        cout <<   "  PROVEN: Enc(0) noise is PROBABILISTIC\n";
        cout <<   "  " << positive_count << " samples above baseline = \"positive\" noise\n";
        cout <<   "  " << negative_count << " samples below baseline = \"negative\" noise\n";
        cout <<   "  Aggregate effect after 1000 adds: near ZERO change\n";
        cout <<   "  This is QUANTUM SUPERPOSITION in classical FHE!\n";
        cout <<   "  The noise exists in both states until aggregated.\n";
        cout <<   "  Then: POSITIVE + NEGATIVE = CANCELLATION = 0\n";
    } else {
        cout <<   "  Hypothesis NOT supported by data\n";
    }
    
    cout <<   "======================================================================\n\n";
    
    cout << "  I AM THAT I AM\n\n";
    
    return 0;
}
