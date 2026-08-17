// 4-BIT MULTIPLIER — FIXED CARRY LOGIC
#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "4-BIT MULTIPLIER FIXED\n";
    std::cout << "=====================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(40);
    parameters.SetRingDim(65536);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    
    auto nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, ab);
    };
    auto NOT = [&](Ciphertext<DCRTPoly> a) { return nand(a, a); };
    auto AND = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { return NOT(nand(a, b)); };
    auto OR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) { return nand(NOT(a), NOT(b)); };
    auto XOR = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto n1 = nand(a, b);
        return nand(nand(a, n1), nand(b, n1));
    };
    
    // Full Adder: sum = XOR(XOR(a,b), cin), cout = OR(AND(a,b), AND(XOR(a,b), cin))
    auto full_adder = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, 
                           Ciphertext<DCRTPoly> cin) {
        auto xor_ab = XOR(a, b);
        auto sum = XOR(xor_ab, cin);
        auto ab_and = AND(a, b);
        auto xc_and = AND(xor_ab, cin);
        auto cout = OR(ab_and, xc_and);
        return std::make_pair(sum, cout);
    };
    
    auto dec = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };
    
    std::cout << "TESTING 4-BIT MULTIPLIER (5 cases):\n\n";
    
    std::vector<std::pair<int,int>> tests = {{10,6}, {3,5}, {7,9}, {15,15}, {12,11}};
    int passed = 0;
    
    for (auto [A, B] : tests) {
        bool a[4] = {(bool)(A&1), (bool)(A&2), (bool)(A&4), (bool)(A&8)};
        bool b[4] = {(bool)(B&1), (bool)(B&2), (bool)(B&4), (bool)(B&8)};
        
        Ciphertext<DCRTPoly> ca[4] = {a[0]?ct1:ct0, a[1]?ct1:ct0, a[2]?ct1:ct0, a[3]?ct1:ct0};
        Ciphertext<DCRTPoly> cb[4] = {b[0]?ct1:ct0, b[1]?ct1:ct0, b[2]?ct1:ct0, b[3]?ct1:ct0};
        
        // Partial products: pp[i][j] = a[i] * b[j]
        Ciphertext<DCRTPoly> pp[4][4];
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                pp[i][j] = AND(ca[i], cb[j]);
        
        // Ripple carry addition para sa bawat column
        // Column k: sum = XOR lahat ng pp[i][j] kung saan i+j=k, plus carry
        Ciphertext<DCRTPoly> carry = ct0;
        Ciphertext<DCRTPoly> result[8];
        
        // Column 0: pp[0][0]
        result[0] = pp[0][0];
        
        // Column 1: pp[0][1] + pp[1][0]
        auto fa1 = full_adder(pp[0][1], pp[1][0], ct0);
        result[1] = fa1.first;
        carry = fa1.second;
        
        // Column 2: pp[0][2] + pp[1][1] + pp[2][0] + carry
        auto fa2a = full_adder(pp[0][2], pp[1][1], carry);
        auto fa2b = full_adder(fa2a.first, pp[2][0], ct0);
        result[2] = fa2b.first;
        carry = OR(fa2a.second, fa2b.second);
        
        // Column 3: pp[0][3] + pp[1][2] + pp[2][1] + pp[3][0] + carry
        auto fa3a = full_adder(pp[0][3], pp[1][2], carry);
        auto fa3b = full_adder(fa3a.first, pp[2][1], ct0);
        auto fa3c = full_adder(fa3b.first, pp[3][0], ct0);
        result[3] = fa3c.first;
        carry = OR(OR(fa3a.second, fa3b.second), fa3c.second);
        
        // Column 4: pp[1][3] + pp[2][2] + pp[3][1] + carry
        auto fa4a = full_adder(pp[1][3], pp[2][2], carry);
        auto fa4b = full_adder(fa4a.first, pp[3][1], ct0);
        result[4] = fa4b.first;
        carry = OR(fa4a.second, fa4b.second);
        
        // Column 5: pp[2][3] + pp[3][2] + carry
        auto fa5 = full_adder(pp[2][3], pp[3][2], carry);
        result[5] = fa5.first;
        carry = fa5.second;
        
        // Column 6: pp[3][3] + carry
        auto fa6 = full_adder(pp[3][3], ct0, carry);
        result[6] = fa6.first;
        carry = fa6.second;
        
        // Column 7: carry
        result[7] = carry;
        
        // Decode result
        int computed = 0;
        for (int i = 0; i < 8; i++) {
            int bit = dec(result[i]);
            computed |= (bit << i);
        }
        
        int expected = A * B;
        bool ok = (computed == expected);
        
        std::cout << "  " << (ok ? "✅" : "❌") << " " << A << " × " << B 
                  << " = " << computed << " (expected " << expected << ")\n";
        if (ok) passed++;
    }
    
    std::cout << "\n  Result: " << passed << "/" << tests.size() << " passed\n";
    
    return 0;
}
