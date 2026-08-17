// L(k) + OPENFHE BFV — EXACT INTEGER FHE
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "L(k) + OPENFHE BFV\n";
    std::cout << "==================\n\n";
    
    NTL::ZZ L_k = NTL::to_ZZ("599074578");
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731"));
    
    std::cout << "L(k) = " << L_k << "\n";
    std::cout << "inv_L(k) = " << inv_L_k << "\n\n";
    
    // BFV — exact integer
    CCParams<CryptoContextBFVRNS> params;
    params.SetPlaintextModulus(65537);  // Large prime para sa L(k)
    params.SetMultiplicativeDepth(5);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    std::cout << "BFV Parameters:\n";
    std::cout << "  Plaintext modulus: 65537\n";
    std::cout << "  Ring: " << cc->GetRingDimension() << "\n\n";
    
    // Encrypt L(k) at 0
    int64_t lk_val = 599074578LL % 65537;  // Mod 65537
    std::cout << "L(k) mod 65537 = " << lk_val << "\n\n";
    
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({lk_val}));
    
    // Decrypt test
    Plaintext pt0, pt1;
    cc->Decrypt(keys.secretKey, ct0, &pt0);
    cc->Decrypt(keys.secretKey, ct1, &pt1);
    std::cout << "Decrypt(0) = " << pt0->GetPackedValue()[0] << "\n";
    std::cout << "Decrypt(1) = " << pt1->GetPackedValue()[0] << " (exp " << lk_val << ")\n\n";
    
    // NAND: 1 - a·b sa BFV
    auto nand_11 = cc->EvalSub(ct1, cc->EvalMult(ct1, ct1));
    Plaintext pt_nand;
    cc->Decrypt(keys.secretKey, nand_11, &pt_nand);
    std::cout << "NAND(1,1) = " << pt_nand->GetPackedValue()[0] << " (exp 0)\n";
    
    auto nand_00 = cc->EvalSub(ct1, cc->EvalMult(ct0, ct0));
    Plaintext pt_nand00;
    cc->Decrypt(keys.secretKey, nand_00, &pt_nand00);
    std::cout << "NAND(0,0) = " << pt_nand00->GetPackedValue()[0] << " (exp " << lk_val << ")\n";
    
    return 0;
}
