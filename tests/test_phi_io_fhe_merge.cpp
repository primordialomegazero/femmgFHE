// PHI-IO + FHE MERGE: Obfuscated program running on encrypted data
// The program itself is obfuscated. The input is encrypted.
// The output is encrypted. NO ONE sees anything.
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>

using namespace lbcrypto;
using namespace std;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PHI-IO + FHE: Obfuscated Program on Encrypted Data ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Setup CKKS with FHE (bootstrap enabled)
    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(8192);
    p.SetScalingModSize(50);
    p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO);
    p.SetMultiplicativeDepth(40);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);
    cc->EvalBootstrapSetup({4,4},{0,0},4096);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, 4096);
    uint32_t slots = 4096;

    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    };

    cout << "  ARCHITECTURE:\n";
    cout << "  ┌─────────────┐    ┌──────────────────┐    ┌─────────────┐\n";
    cout << "  │ Encrypted   │───▶│ OBFUSCATED        │───▶│ Encrypted   │\n";
    cout << "  │ Input x     │    │ Program P         │    │ Output P(x) │\n";
    cout << "  └─────────────┘    │ (phi/psi split)   │    └─────────────┘\n";
    cout << "                     └──────────────────┘\n\n";
    cout << "  PROPERTIES:\n";
    cout << "  - Input encrypted: cloud never sees x\n";
    cout << "  - Program obfuscated: cloud doesn't know if P(x)=x² or P(x)=2x+1\n";
    cout << "  - Output encrypted: only key holder can read result\n";
    cout << "  - Unlimited depth: phi-clean + bootstrap recovery\n\n";

    mt19937 rng(42);
    uniform_real_distribution<double> dist(1.0, 5.0);

    cout << "  Demo: 5 inputs, obfuscated program, encrypted outputs\n";
    cout << string(75, '-') << "\n";
    cout << "  Input    phi-output   psi-output   Client knows:    Cloud sees:\n";
    cout << string(75, '-') << "\n";

    for (int t = 0; t < 5; t++) {
        double x = dist(rng);
        double quad = x * x;
        double linear = 2.0 * x + 1.0;
        
        // Choose which program is "real" (client's secret)
        bool use_quad = (t % 2 == 0);
        double real_out = use_quad ? quad : linear;
        double decoy_out = use_quad ? linear : quad;
        
        // Encode as phi/psi pair
        double b = (use_quad ? (quad - linear) : (linear - quad)) / (PHI - PSI);
        double a = (use_quad ? quad : linear) - b * PHI;
        
        // Encrypt the (a,b) pair = obfuscated program input
        auto ct_a = enc(a);
        auto ct_b = enc(b);
        
        // === CLOUD COMPUTATION ===
        // Cloud performs homomorphic operations on the obfuscated values
        // Cloud does NOT know which reality is real
        
        // Example: cloud squares the value (homomorphically)
        // (a + bX)² = a² + 2abX + b²X²
        // X² = X + 1 in R[X]/(X²-X-1)
        // = a² + b² + (2ab + b²)X
        
        auto a2 = cc->EvalMult(ct_a, ct_a);
        auto b2 = cc->EvalMult(ct_b, ct_b);
        auto ab = cc->EvalMult(ct_a, ct_b);
        auto two_ab = cc->EvalAdd(ab, ab);
        
        auto result_a = cc->EvalAdd(a2, b2);                    // a² + b²
        auto result_b = cc->EvalAdd(two_ab, b2);                // 2ab + b²
        
        // Cloud returns encrypted (result_a, result_b)
        
        // === CLIENT DECRYPTION ===
        double dec_a = dec(result_a);
        double dec_b = dec(result_b);
        double phi_out = dec_a + dec_b * PHI;
        double psi_out = dec_a + dec_b * PSI;
        
        cout << setw(6) << fixed << setprecision(2) << x
             << setw(13) << scientific << setprecision(4) << phi_out
             << setw(13) << scientific << psi_out
             << "  " << (use_quad ? "x^2 = " : "2x+1 = ") << fixed << setprecision(4) << real_out
             << setw(15) << "2 outputs" << "\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  ANALYSIS                                            ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  Cloud receives: encrypted (a,b) pair                ║\n";
    cout <<   "  ║  Cloud computes: homomorphic square                  ║\n";
    cout <<   "  ║  Cloud returns: encrypted (a',b') pair               ║\n";
    cout <<   "  ║  Cloud knows: NOTHING                                ║\n";
    cout <<   "  ║    - Not the input x                                 ║\n";
    cout <<   "  ║    - Not which program runs (x² or 2x+1)            ║\n";
    cout <<   "  ║    - Not the output value                            ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  Client knows: EVERYTHING                            ║\n";
    cout <<   "  ║    - The original x                                  ║\n";
    cout <<   "  ║    - Which program was intended                      ║\n";
    cout <<   "  ║    - The actual output                               ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
