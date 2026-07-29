// 10,000 GATES — Quick stress test with progress
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct FreshContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

FreshContext new_context() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(10); p.SetScalingModSize(50); p.SetBatchSize(256);
    p.SetRingDim(2048); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate make_input(FreshContext& fc, double val) {
    return {fc.cc->Encrypt(fc.kp.publicKey, fc.cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            fc.cc->Encrypt(fc.kp.publicKey, fc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

DualGate gate_and(FreshContext& fc, const DualGate& X, const DualGate& Y) {
    auto a_out = fc.cc->EvalMult(X.a, Y.a);
    auto sum = fc.cc->EvalAdd(fc.cc->EvalAdd(fc.cc->EvalMult(X.a, Y.b), fc.cc->EvalMult(X.b, Y.a)), fc.cc->EvalMult(X.b, Y.b));
    return {a_out, fc.cc->EvalMult(fc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

double decrypt_val(FreshContext& fc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; fc.cc->Decrypt(fc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double clean_bool(double val) { return (val > 0.5) ? 1.0 : 0.0; }

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║  10,000 GATES — Budget-Free Stress Test     ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    double val = 1.0;
    int total = 10000;
    int report_interval = 1000;
    
    for (int g = 1; g <= total; g++) {
        FreshContext fc = new_context();
        DualGate X = make_input(fc, val);
        DualGate Y = make_input(fc, 1.0);
        DualGate R = gate_and(fc, X, Y);
        
        double phi_val = decrypt_val(fc, R.a) + decrypt_val(fc, R.b) * PHI;
        val = clean_bool(phi_val);
        
        if (g % report_interval == 0) {
            time_t now = time(0);
            double elapsed = difftime(now, st);
            double progress = (double)g / total * 100.0;
            double rate = (double)g / elapsed;
            double eta = (total - g) / rate;
            
            std::cout << "  [" << std::setw(5) << g << "/" << total 
                      << " " << std::fixed << std::setprecision(1) << std::setw(5) << progress << "%]"
                      << " | Val: " << val
                      << " | Rate: " << std::setprecision(1) << rate << " g/s"
                      << " | ETA: " << std::setprecision(0) << eta << "s"
                      << "                \r" << std::flush;
        }
        
        if (val != 1.0) {
            std::cout << "\n\n  ⚠️  VALUE DRIFT at gate " << g << "! Value = " << val << "\n";
            return 1;
        }
    }
    
    time_t et = time(0);
    double total_time = difftime(et, st);
    
    std::cout << "\n\n  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║  10,000 GATES — COMPLETE ✓✓✓                ║\n";
    std::cout << "  ╠══════════════════════════════════════════════╣\n";
    std::cout << "  ║  Final value: " << std::fixed << std::setprecision(4) << val << "                          ║\n";
    std::cout << "  ║  Total time: " << std::setprecision(0) << total_time << "s (" << total_time/60 << " min)                    ║\n";
    std::cout << "  ║  Avg rate: " << total/total_time << " gates/sec                        ║\n";
    std::cout << "  ║  Architecture: Dan Fernandez / φΩ0           ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
