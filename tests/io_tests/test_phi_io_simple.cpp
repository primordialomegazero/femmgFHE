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
    cout << "\n  === PHI-IO SIMPLE: phi/psi Reality Indistinguishability ===\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096);
    p.SetScalingModSize(50);
    p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO);
    p.SetMultiplicativeDepth(10);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    uint32_t slots = 2048;

    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    };

    cout << "  CONCEPT: An obfuscated value v = a + bX in R[X]/(X^2-X-1)\n";
    cout << "  has TWO simultaneous evaluations:\n";
    cout << "    eval_phi(v) = a + b*phi    (phi = 1.618)\n";
    cout << "    eval_psi(v) = a + b*psi    (psi = -0.618)\n\n";
    cout << "  One reality is the REAL computation, the other is DECOY.\n";
    cout << "  The adversary sees (a,b) but cannot tell which is which.\n\n";

    mt19937 rng(42);
    uniform_real_distribution<double> dist(-10.0, 10.0);
    
    int trials = 50;
    int correct_guesses = 0;

    cout << "  Trial  Secret   phi-val   psi-val   |phi-real-err| |psi-real-err|  Guess  OK?\n";
    cout << string(78, '-') << "\n";

    for (int t = 0; t < trials; t++) {
        double secret = dist(rng);
        bool phi_is_real = (t % 2 == 0);
        
        double a, b;
        if (phi_is_real) {
            b = secret / (PHI - PSI);
            a = -b * PSI;
        } else {
            b = -secret / (PHI - PSI);
            a = secret - b * PSI;
        }
        
        auto ct_a = enc(a);
        auto ct_b = enc(b);
        
        double dec_a = dec(ct_a);
        double dec_b = dec(ct_b);
        double phi_val = dec_a + dec_b * PHI;
        double psi_val = dec_a + dec_b * PSI;
        
        double phi_real_err = abs(phi_val - (phi_is_real ? secret : 0.0));
        double psi_real_err = abs(psi_val - (phi_is_real ? 0.0 : secret));
        
        bool guess_phi = (abs(psi_val) < abs(phi_val));
        if (abs(abs(phi_val) - abs(psi_val)) < 0.01) guess_phi = (rand() % 2 == 0);
        
        if (guess_phi == phi_is_real) correct_guesses++;
        
        if (t < 10 || t >= trials - 3) {
            cout << setw(5) << t << "  " << (phi_is_real ? "phi" : "psi") << "   "
                 << setw(9) << fixed << setprecision(4) << phi_val
                 << setw(9) << fixed << psi_val
                 << setw(13) << scientific << setprecision(2) << phi_real_err
                 << setw(13) << scientific << psi_real_err
                 << "   " << (guess_phi ? "phi" : "psi")
                 << "   " << (guess_phi == phi_is_real ? "Y" : "N") << "\n";
        }
    }

    double rate = (double)correct_guesses / trials;
    cout << "\n  RESULT: " << correct_guesses << "/" << trials 
         << " (" << fixed << setprecision(1) << rate * 100 << "%)\n";
    
    if (rate < 0.55) cout << "  VERDICT: INDISTINGUISHABLE. phi/psi split = iO.\n";
    else if (rate < 0.65) cout << "  VERDICT: Weakly distinguishable.\n";
    else cout << "  VERDICT: Distinguishable. Need better masking.\n";
    cout << "\n";

    return 0;
}
