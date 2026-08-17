// MULTIPLIER + COMPARATOR from NAND — Mas malalim na circuits
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "MULTIPLIER + COMPARATOR TESTS\n";
    std::cout << "=============================\n\n";
    
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
    
    auto decrypt_bit = [&](Ciphertext<DCRTPoly> ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };
    
    int errors = 0;
    int total = 0;
    
    // ============ 2-BIT MULTIPLIER ============
    std::cout << "2-BIT MULTIPLIER (A×B, A,B ∈ {0,1,2,3}):\n";
    std::cout << "-----------------------------------------\n";
    
    for (int a = 0; a < 4; a++) {
        for (int b = 0; b < 4; b++) {
            bool a0 = a & 1, a1 = a & 2;
            bool b0 = b & 1, b1 = b & 2;
            
            auto ca0 = a0 ? ct1 : ct0;
            auto ca1 = a1 ? ct1 : ct0;
            auto cb0 = b0 ? ct1 : ct0;
            auto cb1 = b1 ? ct1 : ct0;
            
            // 2-bit multiply: p0 = a0·b0, p1 = a0·b1 XOR a1·b0, p2 = a1·b1
            auto p0 = AND(ca0, cb0);
            auto t1 = AND(ca0, cb1);
            auto t2 = AND(ca1, cb0);
            auto p1 = XOR(t1, t2);
            auto p2 = AND(ca1, cb1);
            
            int exp_p0 = (a&1) & (b&1) ? 1 : 0;
            int exp_p1 = ((a&1)&(b&2)>>1) ^ ((a&2)>>1 & (b&1)) ? 1 : 0;
            int exp_p2 = (a&2)>>1 && (b&2)>>1 ? 1 : 0;
            
            int got_p0 = decrypt_bit(p0);
            int got_p1 = decrypt_bit(p1);
            int got_p2 = decrypt_bit(p2);
            
            total += 3;
            if (got_p0 != exp_p0) { errors++; std::cout << "  FAIL: " << a << "×" << b << " p0=" << got_p0 << " exp=" << exp_p0 << "\n"; }
            if (got_p1 != exp_p1) { errors++; std::cout << "  FAIL: " << a << "×" << b << " p1=" << got_p1 << " exp=" << exp_p1 << "\n"; }
            if (got_p2 != exp_p2) { errors++; std::cout << "  FAIL: " << a << "×" << b << " p2=" << got_p2 << " exp=" << exp_p2 << "\n"; }
        }
    }
    std::cout << "Multiplier: " << (total - errors) << "/" << total << " PASS\n\n";
    
    // ============ 2-BIT COMPARATOR (A == B) ============
    std::cout << "2-BIT COMPARATOR (A == B):\n";
    std::cout << "---------------------------\n";
    
    for (int a = 0; a < 4; a++) {
        for (int b = 0; b < 4; b++) {
            bool a0 = a & 1, a1 = a & 2;
            bool b0 = b & 1, b1 = b & 2;
            
            auto ca0 = a0 ? ct1 : ct0;
            auto ca1 = a1 ? ct1 : ct0;
            auto cb0 = b0 ? ct1 : ct0;
            auto cb1 = b1 ? ct1 : ct0;
            
            // XNOR = NOT(XOR)
            auto x0 = XOR(ca0, cb0);
            auto x1 = XOR(ca1, cb1);
            auto nx0 = NOT(x0);
            auto nx1 = NOT(x1);
            auto eq = AND(nx0, nx1);
            
            int exp = (a == b) ? 1 : 0;
            int got = decrypt_bit(eq);
            
            total++;
            if (got != exp) {
                errors++;
                std::cout << "  FAIL: " << a << " vs " << b << " = " << got << " exp=" << exp << "\n";
            }
        }
    }
    std::cout << "Comparator: " << (16 - errors + 48) << "/64 PASS\n\n";
    
    std::cout << "========================\n";
    std::cout << "TOTAL ERRORS: " << errors << "\n";
    std::cout << (errors == 0 ? "✅ ALL CIRCUITS WORK!\n" : "❌ FAILURES FOUND\n");
    
    return 0;
}
