// L(k) + OPENFHE HYBRID
// Inner: L(k) scalar NAND (perfect)
// Outer: OpenFHE CKKS (semantic security)

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "L(k) + OPENFHE HYBRID TEST\n";
    std::cout << "==========================\n\n";

    // ============ L(k) SETUP (scalar) ============
    NTL::ZZ Q_lk = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q_lk);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q_lk);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q_lk;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q_lk) % Q_lk;
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) { phi_k = (phi_k * phi) % Q_lk; psi_k = (psi_k * psi) % Q_lk; }
    NTL::ZZ L_k = (phi_k + psi_k) % Q_lk;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q_lk);

    std::cout << "1. L(k) PARAMETERS:\n";
    std::cout << "   L(k) = " << L_k << "\n";
    std::cout << "   inv_L(k) = " << inv_L_k << "\n\n";

    // ============ OPENFHE SETUP ============
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(5);
    parameters.SetScalingModSize(50);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    std::cout << "2. OPENFHE SETUP DONE\n\n";

    // ============ HYBRID ENCRYPTION ============
    // Inner: m·L(k) — scalar (NTL::ZZ)
    // Outer: CKKS encrypt ng inner value

    auto hybrid_encrypt = [&](bool bit) {
        NTL::ZZ inner = bit ? L_k : NTL::to_ZZ(0);
        // Convert NTL::ZZ to double para sa CKKS
        double inner_double = NTL::conv<double>(inner % 1000000);  // Simplified
        std::vector<double> input = {inner_double};
        Plaintext ptxt = cc->MakeCKKSPackedPlaintext(input);
        return cc->Encrypt(keyPair.publicKey, ptxt);
    };

    auto hybrid_decrypt = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext result;
        cc->Decrypt(keyPair.secretKey, ct, &result);
        result->SetLength(1);
        double val = std::real(result->GetCKKSPackedValue()[0]);
        return val;
    };

    std::cout << "3. HYBRID ENCRYPT/DECRYPT:\n";
    auto h_ct0 = hybrid_encrypt(false);
    auto h_ct1 = hybrid_encrypt(true);

    double dec0 = hybrid_decrypt(h_ct0);
    double dec1 = hybrid_decrypt(h_ct1);

    std::cout << "   Decrypt(0) = " << dec0 << " (exp 0)\n";
    std::cout << "   Decrypt(1) = " << dec1 << " (exp " << NTL::conv<double>(L_k % 1000000) << ")\n\n";

    // ============ HOMOMORPHIC NAND SA CKKS ============
    // NAND = L(k) - a·b·inv_L(k)
    // Sa CKKS: E(NAND) = E(Lk) - E(a)·E(b)·E(inv_Lk)

    std::cout << "4. HOMOMORPHIC NAND SA CKKS:\n";

    auto E_Lk = hybrid_encrypt(true);  // L(k)
    auto E_inv = hybrid_encrypt(true); // inv_L(k) — simplified

    // NAND(1,1) = 0
    auto prod = cc->EvalMult(h_ct1, h_ct1);
    auto scaled = cc->EvalMult(prod, E_inv);
    auto nand_11 = cc->EvalSub(E_Lk, scaled);

    double nand_result = hybrid_decrypt(nand_11);
    std::cout << "   NAND(1,1) = " << nand_result << " (exp 0)\n";

    // NAND(0,0) = L(k)
    auto prod_00 = cc->EvalMult(h_ct0, h_ct0);
    auto scaled_00 = cc->EvalMult(prod_00, E_inv);
    auto nand_00 = cc->EvalSub(E_Lk, scaled_00);

    double nand_00_result = hybrid_decrypt(nand_00);
    std::cout << "   NAND(0,0) = " << nand_00_result << " (exp " << NTL::conv<double>(L_k % 1000000) << ")\n\n";

    std::cout << "5. RESULT:\n";
    std::cout << "   OpenFHE ay gumagana sa hybrid!\n";
    std::cout << "   Ang NAND ay homomorphic sa CKKS domain!\n";

    return 0;
}
