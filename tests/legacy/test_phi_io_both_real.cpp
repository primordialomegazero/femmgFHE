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
    cout << "\n=== PHI-IO: Both Realities Have Valid Outputs ===\n\n";

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

    cout << "  STRATEGY: Both realities contain PLAUSIBLE computations.\n";
    cout << "  phi-reality: f(x) = x^2     (quadratic)\n";
    cout << "  psi-reality: f(x) = 2x + 1  (linear)\n";
    cout << "  The adversary sees BOTH outputs. Can they tell which\n";
    cout << "  was the INTENDED computation?\n\n";

    mt19937 rng(42);
    uniform_real_distribution<double> dist(1.0, 5.0);
    
    int trials = 50;
    int correct = 0;

    cout << "  Trial  Secret  phi-out  psi-out  phi-err  psi-err  Guess  OK?\n";
    cout << string(70, '-') << "\n";

    for (int t = 0; t < trials; t++) {
        double x = dist(rng);
        double phi_real_out = x * x;        // f_phi(x) = x^2
        double psi_real_out = 2.0 * x + 1.0; // f_psi(x) = 2x + 1
        
        // Randomly choose which computation is "real"
        bool phi_is_real = (t % 2 == 0);
        double real_out = phi_is_real ? phi_real_out : psi_real_out;
        double decoy_out = phi_is_real ? psi_real_out : phi_real_out;
        
        // Encode: both outputs must be embedded
        // v = a + bX where eval_phi(v) = real_out, eval_psi(v) = decoy_out
        // Solve: a + b*phi = (phi_is_real ? real_out : decoy_out)
        //        a + b*psi = (phi_is_real ? decoy_out : real_out)
        double target_phi = phi_is_real ? real_out : decoy_out;
        double target_psi = phi_is_real ? decoy_out : real_out;
        
        double b = (target_phi - target_psi) / (PHI - PSI);
        double a = target_phi - b * PHI;
        
        auto ct_a = enc(a);
        auto ct_b = enc(b);
        
        double dec_a = dec(ct_a);
        double dec_b = dec(ct_b);
        double obs_phi = dec_a + dec_b * PHI;
        double obs_psi = dec_a + dec_b * PSI;
        
        double phi_err = abs(obs_phi - phi_real_out);
        double psi_err = abs(obs_psi - psi_real_out);
        double phi_err_alt = abs(obs_phi - psi_real_out);
        double psi_err_alt = abs(obs_psi - phi_real_out);
        
        // Adversary guesses: which reality matches which function?
        bool guess_phi_is_quad = (phi_err < phi_err_alt);
        bool actual_phi_is_quad = phi_is_real;
        
        if (guess_phi_is_quad == actual_phi_is_quad) correct++;
        
        if (t < 10 || t >= trials - 3) {
            cout << setw(5) << t 
                 << setw(7) << fixed << setprecision(2) << x
                 << setw(9) << fixed << setprecision(3) << obs_phi
                 << setw(9) << fixed << obs_psi
                 << setw(9) << scientific << setprecision(1) << min(phi_err, phi_err_alt)
                 << setw(9) << scientific << min(psi_err, psi_err_alt)
                 << "  " << (guess_phi_is_quad ? "quad" : "lin ")
                 << "  " << (guess_phi_is_quad == actual_phi_is_quad ? "Y" : "N") << "\n";
        }
    }

    double rate = (double)correct / trials;
    cout << "\n  RESULT: " << correct << "/" << trials 
         << " (" << fixed << setprecision(1) << rate * 100 << "%)\n";
    
    if (rate < 0.55) cout << "  INDISTINGUISHABLE. The two realities hide the truth.\n";
    else if (rate < 0.65) cout << "  Weakly distinguishable.\n";
    else cout << "  DISTINGUISHABLE. The functions leave different signatures.\n";
    cout << "\n";

    return 0;
}
