// 4-BIT MULTIPLIER — SINGLE TEST LANG
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "4-BIT MULTIPLIER SINGLE\n";
    std::cout << "======================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(20);
    parameters.SetRingDim(32768);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    // Use only 1 slot para sa test
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };
    
    auto NOT = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { return NOT(nand(a, b)); };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };
    
    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };
    
    // 4-bit multiplier: A=1010(10), B=0110(6) → 60 = 00111100
    bool a[4] = {0, 1, 0, 1};  // LSB first: 1010
    bool b[4] = {0, 1, 1, 0};  // LSB first: 0110
    
    auto ca0 = a[0] ? ct1 : ct0;
    auto ca1 = a[1] ? ct1 : ct0;
    auto ca2 = a[2] ? ct1 : ct0;
    auto ca3 = a[3] ? ct1 : ct0;
    auto cb0 = b[0] ? ct1 : ct0;
    auto cb1 = b[1] ? ct1 : ct0;
    auto cb2 = b[2] ? ct1 : ct0;
    auto cb3 = b[3] ? ct1 : ct0;
    
    std::cout << "Computing 10 × 6 = 60...\n\n";
    
    // Partial products
    auto p00 = AND(ca0, cb0);
    auto p01 = AND(ca0, cb1);
    auto p02 = AND(ca0, cb2);
    auto p03 = AND(ca0, cb3);
    auto p10 = AND(ca1, cb0);
    auto p11 = AND(ca1, cb1);
    auto p12 = AND(ca1, cb2);
    auto p13 = AND(ca1, cb3);
    auto p20 = AND(ca2, cb0);
    auto p21 = AND(ca2, cb1);
    auto p22 = AND(ca2, cb2);
    auto p23 = AND(ca2, cb3);
    auto p30 = AND(ca3, cb0);
    auto p31 = AND(ca3, cb1);
    auto p32 = AND(ca3, cb2);
    auto p33 = AND(ca3, cb3);
    
    // Sum partial products (simplified ripple carry)
    // r0 = p00
    // r1 = p01 XOR p10
    // r2 = p02 XOR p11 XOR p20 XOR carry1
    // ...
    auto r0 = p00;
    auto s1 = XOR(p01, p10);
    auto c1 = AND(p01, p10);
    auto r1 = s1;
    
    auto s2a = XOR(p02, p11);
    auto c2a = AND(p02, p11);
    auto s2b = XOR(p20, c1);
    auto r2 = XOR(s2a, s2b);
    
    auto s3a = XOR(p03, p12);
    auto s3b = XOR(p21, p30);
    auto r3 = XOR(s3a, s3b);
    
    auto s4a = XOR(p13, p22);
    auto s4b = XOR(p31, c2a);
    auto r4 = XOR(s4a, s4b);
    
    auto s5a = XOR(p23, p32);
    auto r5 = XOR(s5a, c1);
    
    auto r6 = p33;
    
    std::cout << "Result bits: ";
    std::cout << dec(r6) << dec(r5) << dec(r4) << dec(r3) << dec(r2) << dec(r1) << dec(r0) << "\n";
    std::cout << "Expected: 00111100\n";
    
    return 0;
}
