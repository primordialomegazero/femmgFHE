// XOR DEBUG — Direct log-space gate verification
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

const double ZERO_LOG = -100.0;
const double THRESHOLD = -1.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualGate encode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    double log_val = (val < 0.5) ? ZERO_LOG : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{log_val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decode_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const DualGate& dg) {
    Plaintext pt; cc->Decrypt(kp.secretKey, dg.a, &pt);
    double log_val = pt->GetCKKSPackedValue()[0].real();
    return (log_val < THRESHOLD) ? 0.0 : 1.0;
}

double decrypt_raw(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const DualGate& dg) {
    Plaintext pt; cc->Decrypt(kp.secretKey, dg.a, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

DualGate gate_and_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    return {cc->EvalAdd(X.a, Y.a), cc->EvalAdd(X.b, Y.b)};
}

DualGate gate_not_log(CryptoContext<DCRTPoly>& cc, const DualGate& X) {
    auto hundred = cc->MakeCKKSPackedPlaintext(std::vector<double>{100.0});
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto shifted = cc->EvalAdd(X.a, hundred);
    auto not_log = cc->EvalMult(shifted, neg_one);
    return {not_log, X.b};
}

DualGate gate_nand_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    return gate_not_log(cc, gate_and_log(cc, X, Y));
}

DualGate gate_or_log(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    DualGate not_x = gate_not_log(cc, X);
    DualGate not_y = gate_not_log(cc, Y);
    return gate_nand_log(cc, not_x, not_y);
}

// DIRECT XOR: (X OR Y) AND NOT(X AND Y)
DualGate gate_xor_direct(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    DualGate or_xy = gate_or_log(cc, X, Y);
    DualGate and_xy = gate_and_log(cc, X, Y);
    DualGate not_and = gate_not_log(cc, and_xy);
    return gate_and_log(cc, or_xy, not_and);
}

int main() {
    std::cout << "\n  XOR DEBUG — Step-by-step log-space values\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(10); p.SetScalingModSize(50); p.SetBatchSize(512);
    p.SetRingDim(4096); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            std::cout << "  ┌────────────────────────────────────────┐\n";
            std::cout << "  │  X=" << x << ", Y=" << y << "  (Expected XOR=" << (x^y) << ")                     │\n";
            std::cout << "  ├────────────────────────────────────────┤\n";
            
            DualGate X = encode_log(cc, kp, (double)x);
            DualGate Y = encode_log(cc, kp, (double)y);
            
            std::cout << "  │  X log: " << std::setw(8) << decrypt_raw(cc, kp, X) << "                    │\n";
            std::cout << "  │  Y log: " << std::setw(8) << decrypt_raw(cc, kp, Y) << "                    │\n";
            
            DualGate and_xy = gate_and_log(cc, X, Y);
            std::cout << "  │  AND log: " << std::setw(8) << decrypt_raw(cc, kp, and_xy) << " → " << (int)decode_log(cc, kp, and_xy) << "                │\n";
            
            DualGate or_xy = gate_or_log(cc, X, Y);
            std::cout << "  │  OR log: " << std::setw(8) << decrypt_raw(cc, kp, or_xy) << " → " << (int)decode_log(cc, kp, or_xy) << "                │\n";
            
            DualGate nand_xy = gate_nand_log(cc, X, Y);
            std::cout << "  │  NAND log: " << std::setw(8) << decrypt_raw(cc, kp, nand_xy) << " → " << (int)decode_log(cc, kp, nand_xy) << "                │\n";
            
            DualGate xor_xy = gate_xor_direct(cc, X, Y);
            double xor_raw = decrypt_raw(cc, kp, xor_xy);
            int xor_bit = (int)decode_log(cc, kp, xor_xy);
            std::cout << "  │  XOR log: " << std::setw(8) << xor_raw << " → " << xor_bit << "                │\n";
            std::cout << "  │  Status: " << (xor_bit == (x^y) ? "OK ✓" : "FAIL") << "                            │\n";
            std::cout << "  └────────────────────────────────────────┘\n\n";
        }
    }
    
    return 0;
}
