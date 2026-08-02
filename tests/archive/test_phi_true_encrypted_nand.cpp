// True Encrypted NAND: All PE operations, no intermediate decryption
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2=cc->EvalMult(x.a,y.b), a2b1=cc->EvalMult(y.a,x.b), b1b2=cc->EvalMult(x.b,y.b);
    return {cc->EvalAdd(a1b2,a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a,y.a), cc->EvalMult(x.b,y.b)};
}
// Add constant C to ratio: R' = R + C
// new_a = a + C*b, new_b = b
PE ratio_add_constant(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& x, double C) {
    auto ptC=cc->MakeCKKSPackedPlaintext(std::vector<double>{C});
    auto ctC=cc->Encrypt(kp.publicKey,ptC);
    auto Cb=cc->EvalMult(x.b,ctC);
    return {cc->EvalAdd(x.a,Cb), x.b};
}
// Multiply ratio by constant K: R' = K * R
// new_a = K*a, new_b = b
PE ratio_mul_constant(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& x, double K) {
    auto ptK=cc->MakeCKKSPackedPlaintext(std::vector<double>{K});
    auto ctK=cc->Encrypt(kp.publicKey,ptK);
    return {cc->EvalMult(x.a,ctK), x.b};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double decode_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return decrypt_val(cc,kp,s.a)/decrypt_val(cc,kp,s.b) - 0.6180339887498949;
}
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit) {
    const double PSI=0.6180339887498949;
    double v=(bit==1)?1.0:0.0;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949;

// TRUE ENCRYPTED NAND: R(NAND) = PSI*(R(A)+R(B)+2) - R(A)*R(B)
PE nand_encrypted(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    // Step 1: sum = R(A) + R(B)
    PE sum_AB=ratio_add(cc,A,B);
    // Step 2: sum_plus_2 = R(A) + R(B) + 2
    PE sum_plus_2=ratio_add_constant(cc,kp,sum_AB,2.0);
    // Step 3: psi_sum = PSI * (R(A) + R(B) + 2)
    PE psi_sum=ratio_mul_constant(cc,kp,sum_plus_2,PSI);
    // Step 4: prod = R(A) * R(B)
    PE prod=ratio_mult(cc,A,B);
    // Step 5: NAND ratio = psi_sum - prod. Subtraction: R1 - R2 = R1 + (-R2)
    // To negate ratio: R' = 2*PSI - R. So -prod = 2*PSI - prod_ratio
    // psi_sum - prod = psi_sum + (2*PSI - prod) = psi_sum + neg(prod)
    PE neg_prod=ratio_add_constant(cc,kp,prod,-2.0*PSI); // Actually: -R = 2*PSI - R = add_constant(prod, 2*PSI - 2*R)?
    // Wait. To subtract: ratio_sub = ratio_add(A, neg(B))
    // neg(B) ratio = -R(B) = -(vB+PSI) = -vB - PSI
    // We need: ratio = -R = -(v+PSI) = -v - PSI = (-v-PSI, 1)
    // This doesn't work with our encoding since we encode v+PSI with v as the value
    // Alternative: use the algebraic identity directly
    // R(NAND) = PSI*sum_plus_2 - prod_ratio
    // = PSI*sum_plus_2 - prod_ratio
    // We can compute this as: PSI*sum_plus_2 + (-prod_ratio)
    // Where -prod_ratio can be achieved by: new_a = -prod.a, new_b stays
    // But CKKS doesn't have native negation... 
    // Actually CKKS EvalNegate exists? Let's use a workaround
    
    // Workaround: R(NAND) = PSI*(R(A)+R(B)+2) - R(A)*R(B)
    // = PSI*R(A) + PSI*R(B) + 2*PSI - R(A)*R(B)
    PE psi_A=ratio_mul_constant(cc,kp,A,PSI);
    PE psi_B=ratio_mul_constant(cc,kp,B,PSI);
    PE psi_A_plus_psi_B=ratio_add(cc,psi_A,psi_B);
    PE psi_sum_plus_2psi=ratio_add_constant(cc,kp,psi_A_plus_psi_B,2.0*PSI);
    // Now we need: psi_sum_plus_2psi - prod
    // In ratio space: (a1/b1) - (a2/b2) = (a1*b2 - a2*b1)/(b1*b2)
    auto num1=cc->EvalMult(psi_sum_plus_2psi.a,prod.b);
    auto num2=cc->EvalMult(prod.a,psi_sum_plus_2psi.b);
    auto den=cc->EvalMult(psi_sum_plus_2psi.b,prod.b);
    auto nand_a=cc->EvalSub(num1,num2);
    return {nand_a,den};
}

int main(){
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║   TRUE ENCRYPTED NAND: All PE Operations, No Decrypt              ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout<<"  === Encrypted NAND Truth Table ===\n";
    std::cout<<"  A B | Decoded NAND | Expected | Error\n";
    std::cout<<"  -----------------------------------------\n";
    
    for(int a=0;a<=1;a++){
        for(int b=0;b<=1;b++){
            PE enc_A=encrypt_bit(cc,kp,a), enc_B=encrypt_bit(cc,kp,b);
            PE enc_NAND=nand_encrypted(cc,kp,enc_A,enc_B);
            double decoded=decode_state(cc,kp,enc_NAND);
            double expected=1.0-a*b;
            std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(10)<<decoded
                 <<" | "<<expected<<"      | "<<std::scientific<<std::setprecision(1)<<std::abs(decoded-expected)<<"\n";
        }
    }
    
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  True Encrypted NAND: COMPLETE                                    ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
