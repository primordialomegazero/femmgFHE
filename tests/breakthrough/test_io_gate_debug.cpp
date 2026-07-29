#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

SecureContext create_context() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20);
    p.SetScalingModSize(50);
    p.SetBatchSize(256);
    p.SetRingDim(2048);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate encrypt_input(SecureContext& sc, double val) {
    return {sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decrypt_value(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double reveal(DualGate& dg, SecureContext& sc, double root) {
    return decrypt_value(sc, dg.a) + decrypt_value(sc, dg.b) * root;
}

DualGate gate_standard(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

DualGate gate_nand_not(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_result = gate_standard(sc, X, Y);
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg_one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto not_a = sc.cc->EvalSub(one, nand_result.a);
    auto not_b = sc.cc->EvalMult(neg_one, nand_result.b);
    return {not_a, not_b};
}

DualGate gate_demorgan(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg_one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    
    DualGate not_x = {sc.cc->EvalSub(one, X.a), sc.cc->EvalMult(neg_one, X.b)};
    DualGate not_y = {sc.cc->EvalSub(one, Y.a), sc.cc->EvalMult(neg_one, Y.b)};
    
    auto or_a = sc.cc->EvalSub(one, sc.cc->EvalMult(
        sc.cc->EvalSub(one, not_x.a), sc.cc->EvalSub(one, not_y.a)));
    auto or_b = sc.cc->EvalAdd(
        sc.cc->EvalAdd(
            sc.cc->EvalMult(sc.cc->EvalSub(one, not_x.a), not_y.b),
            sc.cc->EvalMult(not_x.b, sc.cc->EvalSub(one, not_y.a))
        ), sc.cc->EvalMult(not_x.b, not_y.b));
    
    return {sc.cc->EvalSub(one, or_a), sc.cc->EvalMult(neg_one, or_b)};
}

DualGate gate_arithmetic(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto inv_phi = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PHI});
    
    auto x_phi = sc.cc->EvalAdd(X.a, sc.cc->EvalMult(X.b, phi_pt));
    auto y_phi = sc.cc->EvalAdd(Y.a, sc.cc->EvalMult(Y.b, phi_pt));
    auto prod = sc.cc->EvalMult(x_phi, y_phi);
    auto a_out = sc.cc->EvalMult(prod, inv_phi);
    
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

int main() {
    std::cout << "\n  Gate Debug — Individual Verification\n\n";
    
    SecureContext sc = create_context();
    
    struct GateInfo {
        std::string name;
        DualGate (*func)(SecureContext&, DualGate&, DualGate&);
    };
    
    GateInfo gates[4] = {
        {"Standard", gate_standard},
        {"NAND+NOT", gate_nand_not},
        {"De Morgan", gate_demorgan},
        {"Arithmetic", gate_arithmetic}
    };
    
    for (int g = 0; g < 4; g++) {
        std::cout << "  " << gates[g].name << ":\n";
        int ok = 0;
        for (int x = 0; x <= 1; x++) {
            for (int y = 0; y <= 1; y++) {
                DualGate X = encrypt_input(sc, (double)x);
                DualGate Y = encrypt_input(sc, (double)y);
                DualGate result = gates[g].func(sc, X, Y);
                double val = reveal(result, sc, PHI);
                int bit = (val > 0.5) ? 1 : 0;
                int expected = x & y;
                if (bit == expected) ok++;
                std::cout << "    " << x << " AND " << y << " = " << bit << " (" << expected << ") " 
                          << (bit == expected ? "OK" : "FAIL") << "\n";
            }
        }
        std::cout << "    " << ok << "/4 correct\n\n";
    }
    
    return 0;
}
