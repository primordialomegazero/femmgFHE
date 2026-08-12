// ================================================================
// TEST 1: HOMOMORPHIC NAND
// ================================================================
// Prove: nand_op(sc, ct_a, ct_b) correctly computes NAND on encrypted
// inputs. No intermediate plaintext. Only decrypt at the very end.
// ================================================================

#include <iostream>
#include <iomanip>
#include "openfhe.h"
#include "src/core/constants.h"

using namespace lbcrypto;

// Copy DualGate NAND from fhe_core.h (standalone test)
struct DualGate {
    Ciphertext<DCRTPoly> a;
    Ciphertext<DCRTPoly> b;
};

struct SecureContext {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> kp;
};

SecureContext create_context() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192);
    p.SetMultiplicativeDepth(60);
    p.SetScalingModSize(50);
    p.SetBatchSize(512);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    return {cc, kp};
}

DualGate encrypt(SecureContext& sc, double v) {
    return {
        sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
        sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))
    };
}

double decrypt(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

DualGate nand_op(SecureContext& sc, DualGate& X, DualGate& Y) {
    // NAND(a,b) = 1 - a*b
    auto a_mul_b = sc.cc->EvalMult(X.a, Y.a);
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    return {sc.cc->EvalSub(one, a_mul_b), 
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

int main() {
    std::cout << "======================================================================\n";
    std::cout << "  TRUE HOMOMORPHIC NAND — No Intermediate Plaintext\n";
    std::cout << "  All operations on encrypted data. Decrypt only at the end.\n";
    std::cout << "======================================================================\n\n";

    auto sc = create_context();
    
    // NAND truth table: (0,0)=1, (0,1)=1, (1,0)=1, (1,1)=0
    double inputs[4][2] = {{0,0}, {0,1}, {1,0}, {1,1}};
    double expected[4] = {1, 1, 1, 0};
    
    std::cout << "  NAND Truth Table (all operations in ciphertext domain):\n\n";
    std::cout << "  " << std::setw(8) << "a" << std::setw(8) << "b" 
              << std::setw(12) << "Enc NAND" << std::setw(12) << "Expected" 
              << std::setw(8) << "Status\n";
    std::cout << "  " << std::string(48, '-') << "\n";
    
    int pass = 0;
    for (int i = 0; i < 4; i++) {
        // Encrypt inputs
        auto ct_a = encrypt(sc, inputs[i][0]);
        auto ct_b = encrypt(sc, inputs[i][1]);
        
        // HOMOMORPHIC NAND — no decryption here
        auto ct_result = nand_op(sc, ct_a, ct_b);
        
        // Decrypt ONLY at the end
        double result = decrypt(sc, ct_result.a);
        
        bool correct = (fabs(result - expected[i]) < 0.1);
        if (correct) pass++;
        
        std::cout << "  " << std::setw(8) << inputs[i][0] 
                  << std::setw(8) << inputs[i][1]
                  << std::setw(12) << std::fixed << std::setprecision(4) << result
                  << std::setw(12) << expected[i]
                  << std::setw(8) << (correct ? "PASS" : "FAIL") << "\n";
    }
    
    std::cout << "  " << std::string(48, '-') << "\n";
    std::cout << "  Result: " << pass << "/4\n\n";
    
    std::cout << "======================================================================\n";
    std::cout << "  " << (pass == 4 ? "HOMOMORPHIC NAND VERIFIED" : "FAILED") << "\n";
    std::cout << "  All NAND operations performed on encrypted data.\n";
    std::cout << "  No intermediate plaintext. No shortcuts.\n";
    std::cout << "======================================================================\n";
    
    return pass == 4 ? 0 : 1;
}
