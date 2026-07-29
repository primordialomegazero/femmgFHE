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

double reveal(DualGate& dg, SecureContext& sc, double root) {
    Plaintext pt;
    sc.cc->Decrypt(sc.kp.secretKey, dg.a, &pt);
    double a = pt->GetCKKSPackedValue()[0].real();
    sc.cc->Decrypt(sc.kp.secretKey, dg.b, &pt);
    double b = pt->GetCKKSPackedValue()[0].real();
    return a + b * root;
}

// NOT gate
DualGate gate_not(SecureContext& sc, DualGate& X) {
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg_one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, X.a), sc.cc->EvalMult(neg_one, X.b)};
}

// NAND gate
DualGate gate_nand(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    DualGate and_result = {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
    return gate_not(sc, and_result);
}

// Gate 0: Standard AND
DualGate gate_standard(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

// Gate 1: AND = NOT(NAND) — double NOT
DualGate gate_double_not(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_result = gate_nand(sc, X, Y);
    return gate_not(sc, nand_result);
}

// Gate 2: AND via De Morgan: NOT(NOT(X) OR NOT(Y))
DualGate gate_demorgan(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate not_x = gate_not(sc, X);
    DualGate not_y = gate_not(sc, Y);
    
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto or_a = sc.cc->EvalSub(one, sc.cc->EvalMult(
        sc.cc->EvalSub(one, not_x.a), sc.cc->EvalSub(one, not_y.a)));
    auto or_b = sc.cc->EvalAdd(
        sc.cc->EvalAdd(
            sc.cc->EvalMult(sc.cc->EvalSub(one, not_x.a), not_y.b),
            sc.cc->EvalMult(not_x.b, sc.cc->EvalSub(one, not_y.a))
        ), sc.cc->EvalMult(not_x.b, not_y.b));
    DualGate or_result = {or_a, or_b};
    
    return gate_not(sc, or_result);
}

// Gate 3: AND via Arithmetic φ-transform
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
    std::cout << "\n  NAND Fix — Universal Gate Verification\n\n";
    
    SecureContext sc = create_context();
    
    struct GateInfo {
        std::string name;
        DualGate (*func)(SecureContext&, DualGate&, DualGate&);
    };
    
    GateInfo gates[4] = {
        {"Standard", gate_standard},
        {"NOT(NAND) Double NOT", gate_double_not},
        {"De Morgan", gate_demorgan},
        {"Arithmetic", gate_arithmetic}
    };
    
    bool all_pass = true;
    
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
                else all_pass = false;
                std::cout << "    " << x << " AND " << y << " = " << bit << " (" << expected << ") " 
                          << (bit == expected ? "OK" : "FAIL") << "\n";
            }
        }
        std::cout << "    " << ok << "/4 correct\n\n";
    }
    
    std::cout << "  Overall: " << (all_pass ? "ALL GATES VERIFIED" : "SOME GATES FAILED") << "\n\n";
    
    return all_pass ? 0 : 1;
}
