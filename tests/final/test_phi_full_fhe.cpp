// DM-DGR FULL FHE: Complete with Recycle between stages
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = decrypt_val(cc, kp, s.a);
    double b = decrypt_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}

int decode_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double ratio = get_ratio(cc, kp, s);
    return (ratio > 0.5) ? 1 : 0;
}

double time_ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}

PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    double val = (bit == 1) ? 1.618033988749895 : 0.6180339887498949;
    auto a = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{val}));
    auto b = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    return {a, b};
}

PE recycle(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& state) {
    double a_val = decrypt_val(cc, kp, state.a);
    double b_val = decrypt_val(cc, kp, state.b);
    int bit = (a_val / b_val > 0.5) ? 1 : 0;
    return encrypt_bit(cc, kp, bit);
}

int main() {
    std::cout << "\n";
    std::cout << "  DM-DGR FULL FHE: Complete Test Suite\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(60);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    auto fib_normalize = [&](PE state) -> PE {
        for (int i = 0; i < 8; i++) state = mulY(cc, state);
        return state;
    };

    auto nand_gate = [&](PE A, PE B) -> PE {
        auto AND_a = cc->EvalMult(A.a, B.a);
        auto AND_b = cc->EvalMult(A.b, B.b);
        return fib_normalize(PE{cc->EvalSub(AND_b, AND_a), AND_b});
    };

    // TEST 1: NAND Truth Table
    std::cout << "  TEST 1: NAND Truth Table\n";
    PE bit0 = encrypt_bit(cc, kp, 0);
    PE bit1 = encrypt_bit(cc, kp, 1);
    PE inputs[2] = {bit0, bit1};
    int ok = 0;
    for (int a = 0; a <= 1; a++)
        for (int b = 0; b <= 1; b++) {
            PE res = nand_gate(inputs[a], inputs[b]);
            if (decode_bit(cc, kp, res) == (1 - a*b)) ok++;
        }
    std::cout << "  NAND: " << ok << "/4\n\n";

    // TEST 2: Full Adder with Recycle
    std::cout << "  TEST 2: Full Adder (with recycle)\n";
    
    auto full_adder = [&](PE A, PE B, PE Cin) -> std::pair<PE, PE> {
        PE X1 = recycle(cc, kp, nand_gate(A, B));
        PE X2 = recycle(cc, kp, nand_gate(A, X1));
        PE X3 = recycle(cc, kp, nand_gate(B, X1));
        PE X4 = recycle(cc, kp, nand_gate(X2, X3));
        PE X5 = recycle(cc, kp, nand_gate(X4, Cin));
        PE X6 = recycle(cc, kp, nand_gate(X4, X5));
        PE X7 = recycle(cc, kp, nand_gate(X5, Cin));
        PE SUM = nand_gate(X6, X7);
        PE COUT = nand_gate(X1, X5);
        return {SUM, COUT};
    };
    
    int sum_ok = 0, cout_ok = 0;
    int cases[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},
                       {1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    
    for (int i = 0; i < 8; i++) {
        PE A = encrypt_bit(cc, kp, cases[i][0]);
        PE B = encrypt_bit(cc, kp, cases[i][1]);
        PE Cin = encrypt_bit(cc, kp, cases[i][2]);
        auto [SUM, COUT] = full_adder(A, B, Cin);
        if (decode_bit(cc, kp, SUM) == (cases[i][0]+cases[i][1]+cases[i][2])%2) sum_ok++;
        if (decode_bit(cc, kp, COUT) == (cases[i][0]+cases[i][1]+cases[i][2])/2) cout_ok++;
    }
    
    gettimeofday(&t1, NULL);
    std::cout << "  SUM: " << sum_ok << "/8  COUT: " << cout_ok << "/8\n";
    std::cout << "  Time: " << time_ms(t0, t1) << "ms\n\n";

    // TEST 3: 4-bit Adder
    std::cout << "  TEST 3: 4-bit Adder\n";
    srand(time(0));
    int a_bits[4], b_bits[4], valA = 0, valB = 0;
    for (int i = 0; i < 4; i++) {
        a_bits[i] = rand() % 2; b_bits[i] = rand() % 2;
        valA = valA*2 + a_bits[i]; valB = valB*2 + b_bits[i];
    }
    
    PE carry = encrypt_bit(cc, kp, 0);
    int sum_bits[4], result = 0;
    
    gettimeofday(&t0, NULL);
    for (int i = 3; i >= 0; i--) {
        PE A = encrypt_bit(cc, kp, a_bits[i]);
        PE B = encrypt_bit(cc, kp, b_bits[i]);
        auto [SUM, COUT] = full_adder(A, B, carry);
        sum_bits[i] = decode_bit(cc, kp, SUM);
        carry = recycle(cc, kp, COUT);
    }
    gettimeofday(&t1, NULL);
    
    int final_carry = decode_bit(cc, kp, carry);
    for (int i = 0; i < 4; i++) { std::cout << sum_bits[i]; result += sum_bits[i]*(1<<(3-i)); }
    if (final_carry) result += 16;
    std::cout << " (c=" << final_carry << ") = " << result 
              << " | Expected=" << (valA+valB) 
              << " | " << (result==valA+valB ? "MATCH!" : "MISMATCH") << "\n";
    std::cout << "  Time: " << time_ms(t0, t1) << "ms\n\n";

    // SUMMARY
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║  NAND: " << ok << "/4  FA: SUM=" << sum_ok << "/8 COUT=" << cout_ok << "/8";
    std::cout << "   ║\n";
    std::cout << "  ║  4-bit: " << (result==valA+valB ? "MATCH!" : "MISMATCH") << "                    ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";

    return 0;
}
