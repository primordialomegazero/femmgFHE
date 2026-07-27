#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <random>
#include "openfhe.h"

using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE phi(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE psi(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = decrypt_val(cc, kp, s.a), b = decrypt_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}

int extract_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return (get_ratio(cc, kp, s) > 0.5) ? 1 : 0;
}

double elapsed_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec - s.tv_sec) * 1000.0 + (e.tv_usec - s.tv_usec) / 1000.0;
}

PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;
    return {
        cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
        cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))
    };
}

const double PHI = 1.618033988749895;
const double PSI = -0.6180339887498949;

std::mt19937 rng(std::random_device{}());

PE nand_gate(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aa = cc->EvalMult(A.a, B.a);
    auto bb = cc->EvalMult(A.b, B.b);
    PE raw = {cc->EvalSub(bb, aa), bb};
    for (int i = 0; i < 4; i++) raw = phi(cc, raw);
    for (int i = 0; i < 4; i++) raw = psi(cc, raw);
    return raw;
}

PE nand_encrypt(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, int rounds) {
    PE s = in;
    PE c = encrypt_bit(cc, kp, 1);
    for (int i = 0; i < rounds; i++) s = nand_gate(cc, s, c);
    return s;
}

void apply_phi_psi_turbulence(CryptoContext<DCRTPoly>& cc, PE& s, int phi_count, int psi_count) {
    for (int i = 0; i < phi_count; i++) s = phi(cc, s);
    for (int i = 0; i < psi_count; i++) s = psi(cc, s);
}

void apply_asymmetric_oscillation(CryptoContext<DCRTPoly>& cc, PE& s, int pairs, bool phi_first) {
    for (int i = 0; i < pairs; i++) {
        if (phi_first) { s = phi(cc, s); s = psi(cc, s); }
        else           { s = psi(cc, s); s = phi(cc, s); }
    }
}

PE layer_veil(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s = nand_encrypt(cc, kp, in, 3);
    apply_asymmetric_oscillation(cc, s, 3, false);
    apply_phi_psi_turbulence(cc, s, 2, 4);
    return s;
}

PE layer_signal(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s = nand_encrypt(cc, kp, in, 3);
    apply_asymmetric_oscillation(cc, s, 3, true);
    apply_phi_psi_turbulence(cc, s, 4, 2);
    return s;
}

PE layer_fractal_chaos(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s = nand_encrypt(cc, kp, in, 4);
    for (int i = 0; i < 5; i++) {
        s = phi(cc, s); s = phi(cc, s); s = psi(cc, s);
    }
    for (int i = 0; i < 3; i++) {
        s = phi(cc, s); s = psi(cc, s); s = phi(cc, s);
    }
    return s;
}

PE layer_harmonic_omega(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s = nand_encrypt(cc, kp, in, 4);
    apply_asymmetric_oscillation(cc, s, 3, true);
    apply_phi_psi_turbulence(cc, s, 3, 2);
    apply_asymmetric_oscillation(cc, s, 2, false);
    apply_phi_psi_turbulence(cc, s, 1, 2);
    return s;
}

struct SourceState {
    PE veil;
    PE signal;
    PE fractal;
    PE omega;
};

SourceState source_protect(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& input) {
    SourceState st;
    st.veil    = layer_veil(cc, kp, input);
    st.signal  = layer_signal(cc, kp, input);
    st.fractal = layer_fractal_chaos(cc, kp, input);
    st.omega   = layer_harmonic_omega(cc, kp, input);
    return st;
}

int main() {
    time_t start_time = time(0);
    std::cout << "\n";
    std::cout << "  Source iO — 16K Ring Dimension\n";
    std::cout << "  Started: " << ctime(&start_time) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);
    p.SetScalingModSize(50);
    p.SetBatchSize(1024);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    PE b0 = encrypt_bit(cc, kp, 0);
    PE b1 = encrypt_bit(cc, kp, 1);
    const int TRIALS = 100;

    std::cout << "  PHI = " << std::fixed << std::setprecision(6) << PHI << "\n";
    std::cout << "  PSI = " << PSI << "\n";
    std::cout << "  PHI * PSI = " << PHI * PSI << "\n";
    std::cout << "  Trials: " << TRIALS << "\n";
    std::cout << "  Progress: " << std::flush;

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    
    int veil_err = 0, signal_err = 0, fractal_err = 0, omega_err = 0;
    int adv_1 = 0, adv_0 = 0, total_1 = 0, total_0 = 0;
    
    for (int t = 0; t < TRIALS; t++) {
        int secret = rng() % 2;
        PE input = (secret == 0) ? b0 : b1;
        SourceState st = source_protect(cc, kp, input);
        
        if (extract_bit(cc, kp, st.veil)    != secret) veil_err++;
        if (extract_bit(cc, kp, st.signal)  != secret) signal_err++;
        if (extract_bit(cc, kp, st.fractal) != secret) fractal_err++;
        
        int omega_bit = extract_bit(cc, kp, st.omega);
        if (omega_bit != secret) omega_err++;
        
        if (secret == 1) { total_1++; if (omega_bit == 1) adv_1++; }
        else             { total_0++; if (omega_bit == 1) adv_0++; }
        
        if ((t + 1) % 10 == 0) std::cout << "." << std::flush;
    }
    
    gettimeofday(&t1, NULL);
    time_t end_time = time(0);

    std::cout << "\n\n";
    std::cout << "  Results:\n";
    std::cout << "  ─────────────────────────────────────────────\n";
    printf("  Veil      (decoy)    : %5.1f%% error\n", 100.0 * veil_err / TRIALS);
    printf("  Signal    (carrier)  : %5.1f%% error\n", 100.0 * signal_err / TRIALS);
    printf("  Fractal   (chaos)    : %5.1f%% error\n", 100.0 * fractal_err / TRIALS);
    printf("  Omega     (harmonic) : %5.1f%% error\n", 100.0 * omega_err / TRIALS);
    printf("  Advantage (Omega)    : %7.4f%%\n", std::abs(100.0 * adv_1 / total_1 - 100.0 * adv_0 / total_0));
    std::cout << "  ─────────────────────────────────────────────\n";
    printf("  Duration: %.0f min\n", elapsed_ms(t0, t1) / 60000.0);
    std::cout << "  Ended: " << ctime(&end_time) << "\n";

    return 0;
}
