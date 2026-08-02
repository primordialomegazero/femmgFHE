// DM-DGR DEBUG: Trace exact values through NAND + rec chain
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE DM_mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
double DM_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double DM_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = DM_val(cc, kp, s.a), b = DM_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
int DM_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return (DM_ratio(cc, kp, s) > 0.5) ? 1 : 0;
}
PE DM_enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.618033988749895 : 0.6180339887498949;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE DM_rec(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return DM_enc(cc, kp, DM_bit(cc, kp, s));
}
PE DM_nand_raw(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aa = cc->EvalMult(A.a, B.a);
    auto bb = cc->EvalMult(A.b, B.b);
    PE raw = {cc->EvalSub(bb, aa), bb};
    for (int i = 0; i < 6; i++) raw = DM_mulY(cc, raw);
    return raw;
}

void print_pe(const char* label, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& x) {
    double a = DM_val(cc, kp, x.a);
    double b = DM_val(cc, kp, x.b);
    std::cout << "  " << label << ": a=" << std::fixed << std::setprecision(6) << a 
              << " b=" << b << " ratio=" << (std::abs(b)>1e-10 ? a/b : a) 
              << " bit=" << DM_bit(cc,kp,x) << "\n";
}

int main() {
    std::cout << "\n  ╔══════════════════════════════════════════════════╗\n";
    std::cout << "  ║  DM-DGR DEBUG: Trace NAND + rec internals        ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(80); p.SetScalingModSize(45); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    // ──── TEST 1: Raw encoded values ────
    std::cout << "  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 1: Raw DM_enc values\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    PE enc0 = DM_enc(cc, kp, 0);
    PE enc1 = DM_enc(cc, kp, 1);
    print_pe("enc(0)", cc, kp, enc0);
    print_pe("enc(1)", cc, kp, enc1);

    // ──── TEST 2: NAND on raw values ────
    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 2: DM_nand on RAW encoded values\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    PE nand_00 = DM_nand_raw(cc, enc0, enc0);
    PE nand_01 = DM_nand_raw(cc, enc0, enc1);
    PE nand_10 = DM_nand_raw(cc, enc1, enc0);
    PE nand_11 = DM_nand_raw(cc, enc1, enc1);
    print_pe("NAND(0,0)", cc, kp, nand_00);
    print_pe("NAND(0,1)", cc, kp, nand_01);
    print_pe("NAND(1,0)", cc, kp, nand_10);
    print_pe("NAND(1,1)", cc, kp, nand_11);

    // ──── TEST 3: rec on NAND outputs ────
    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 3: DM_rec on NAND outputs\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    PE rec_nand_00 = DM_rec(cc, kp, nand_00);
    PE rec_nand_11 = DM_rec(cc, kp, nand_11);
    print_pe("rec(NAND(0,0))", cc, kp, rec_nand_00);
    print_pe("rec(NAND(1,1))", cc, kp, rec_nand_11);

    // ──── TEST 4: Compare raw enc vs rec ────
    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 4: Compare enc(1) vs rec(nand(0,0))\n";
    std::cout << "  Both should represent bit=1\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    print_pe("enc(1)          ", cc, kp, enc1);
    print_pe("rec(nand(0,0))  ", cc, kp, rec_nand_00);
    
    // Compare their internal a,b values
    double enc1_a = DM_val(cc, kp, enc1.a);
    double enc1_b = DM_val(cc, kp, enc1.b);
    double rec_a = DM_val(cc, kp, rec_nand_00.a);
    double rec_b = DM_val(cc, kp, rec_nand_00.b);
    std::cout << "\n  enc(1):       a=" << enc1_a << " b=" << enc1_b << "\n";
    std::cout << "  rec(nand(0,0)): a=" << rec_a << " b=" << rec_b << "\n";
    std::cout << "  Difference:     Δa=" << std::abs(enc1_a - rec_a) << " Δb=" << std::abs(enc1_b - rec_b) << "\n";

    // ──── TEST 5: NAND on recycled values ────
    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 5: NAND on RECYCLED vs RAW inputs\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    
    // Compare NAND(rec(1), rec(1)) vs NAND(enc(1), enc(1))
    PE rec1 = DM_rec(cc, kp, enc1);  // should be same as enc(1)
    PE nand_rec_11 = DM_nand_raw(cc, rec1, rec1);
    PE nand_raw_11 = DM_nand_raw(cc, enc1, enc1);
    print_pe("NAND(enc(1),enc(1))", cc, kp, nand_raw_11);
    print_pe("NAND(rec(1),rec(1))", cc, kp, nand_rec_11);

    // ──── TEST 6: Trace AND(0,0) step by step ────
    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 6: Trace AND(0,0) = NOT(NAND(0,0))\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    PE step1 = DM_nand_raw(cc, enc0, enc0);   // NAND(0,0) = 1
    print_pe("Step1: NAND(0,0)", cc, kp, step1);
    PE step2 = DM_rec(cc, kp, step1);          // rec to bit 1
    print_pe("Step2: rec(step1)", cc, kp, step2);
    PE step3 = DM_nand_raw(cc, step2, step2);  // NAND(rec, rec) = NAND(1,1) = 0
    print_pe("Step3: NAND(s2,s2)", cc, kp, step3);
    PE step4 = DM_rec(cc, kp, step3);          // rec to bit 0
    print_pe("Step4: rec(step3) = AND(0,0)", cc, kp, step4);
    std::cout << "  Expected AND(0,0) = 0, Got bit=" << DM_bit(cc,kp,step4) << "\n";

    // ──── TEST 7: Chain degradation ────
    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 7: Chain degradation pattern\n";
    std::cout << "  Starting with enc(1), NAND with const enc(1)\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    PE chain = enc1;
    PE c1 = enc1;
    for (int i = 0; i < 6; i++) {
        chain = DM_nand_raw(cc, chain, c1);
        double a = DM_val(cc, kp, chain.a);
        double b = DM_val(cc, kp, chain.b);
        std::cout << "  Step " << i << ": a=" << std::setw(10) << a << " b=" << std::setw(10) << b 
                  << " ratio=" << (std::abs(b)>1e-10 ? a/b : a) 
                  << " bit=" << DM_bit(cc,kp,chain) 
                  << " exp=" << ((i%2==0)?0:1) << "\n";
    }

    // ──── TEST 8: mulY effect on enc(1) ────
    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  TEST 8: DM_mulY iterations on enc(1)\n";
    std::cout << "  (This is what happens inside DM_nand)\n";
    std::cout << "  ═══════════════════════════════════════════════\n";
    PE my = enc1;
    for (int i = 0; i < 10; i++) {
        my = DM_mulY(cc, my);
        double a = DM_val(cc, kp, my.a);
        double b = DM_val(cc, kp, my.b);
        std::cout << "  mulY^" << i+1 << ": a=" << std::setw(10) << a << " b=" << std::setw(10) << b 
                  << " ratio=" << (std::abs(b)>1e-10 ? a/b : a) << "\n";
    }

    std::cout << "\n  ═══════════════════════════════════════════════\n";
    std::cout << "  DEBUG COMPLETE\n";
    std::cout << "  ═══════════════════════════════════════════════\n\n";

    return 0;
}
