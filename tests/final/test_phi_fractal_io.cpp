// FRACTAL iO: Complex PE Encoding + Rotated NAND
// Phase rotation as fractal perturbation, fixed points preserved
#include <iostream>
#include <iomanip>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <ctime>
#include "openfhe.h"
using namespace lbcrypto;

const double PSI=0.6180339887498949;
const double PI=3.141592653589793;

struct ComplexPE {
    Ciphertext<DCRTPoly> a_re, a_im; // a = a_re + i*a_im
    Ciphertext<DCRTPoly> b_re, b_im; // b = b_re + i*b_im
};

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}

// Complex multiply: (a+bi)*(c+di) = (ac-bd) + (ad+bc)i
void complex_mult(CryptoContext<DCRTPoly>& cc, 
                  const Ciphertext<DCRTPoly>& a_re, const Ciphertext<DCRTPoly>& a_im,
                  const Ciphertext<DCRTPoly>& b_re, const Ciphertext<DCRTPoly>& b_im,
                  Ciphertext<DCRTPoly>& out_re, Ciphertext<DCRTPoly>& out_im) {
    auto ac=cc->EvalMult(a_re,b_re), bd=cc->EvalMult(a_im,b_im);
    auto ad=cc->EvalMult(a_re,b_im), bc=cc->EvalMult(a_im,b_re);
    out_re=cc->EvalSub(ac,bd);
    out_im=cc->EvalAdd(ad,bc);
}

// Complex add
void complex_add(CryptoContext<DCRTPoly>& cc,
                 const Ciphertext<DCRTPoly>& a_re, const Ciphertext<DCRTPoly>& a_im,
                 const Ciphertext<DCRTPoly>& b_re, const Ciphertext<DCRTPoly>& b_im,
                 Ciphertext<DCRTPoly>& out_re, Ciphertext<DCRTPoly>& out_im) {
    out_re=cc->EvalAdd(a_re,b_re);
    out_im=cc->EvalAdd(a_im,b_im);
}

// Encode bit with random phase theta
ComplexPE encrypt_bit_complex(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int bit, double theta) {
    double v=(bit==1)?1.0:0.0;
    double re_val=(v+PSI)*cos(theta);
    double im_val=(v+PSI)*sin(theta);
    double b_re_val=cos(theta);
    double b_im_val=sin(theta);
    return {
        cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{re_val})),
        cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{im_val})),
        cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{b_re_val})),
        cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{b_im_val}))
    };
}

// Decode complex PE to real value: v = |ratio| - PSI
double decode_complex(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const ComplexPE& s) {
    double ar=decrypt_val(cc,kp,s.a_re), ai=decrypt_val(cc,kp,s.a_im);
    double br=decrypt_val(cc,kp,s.b_re), bi=decrypt_val(cc,kp,s.b_im);
    std::complex<double> ratio(ar,ai);
    std::complex<double> b(br,bi);
    std::complex<double> div=ratio/b;
    return std::abs(div)-PSI;
}

int to_bit(double v){return (v>0.5)?1:0;}

int main(){
    std::cout<<"\n  FRACTAL iO: Complex PE + Phase Rotation\n";
    std::cout<<"  Testing if NAND fixed points survive phase rotation\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    srand(time(0));
    
    // Pre-compute psi in complex form
    double psi_re=PSI, psi_im=0;
    auto ct_psi_re=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{psi_re}));
    auto ct_psi_im=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{psi_im}));
    auto ct_2psi_re=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{2.0*PSI}));
    auto ct_2psi_im=cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}));
    
    std::cout<<"  === Test: NAND via Complex Phase Rotation ===\n";
    std::cout<<"  Each bit encoded with random phase. NAND computed directly on complex values.\n";
    std::cout<<"  Key insight: NAND(a,b) = psi*(a+b+2) - a*b works on complex numbers!\n\n";
    
    std::cout<<"  A B | PhaseA PhaseB | Decoded | Expected | Match\n";
    std::cout<<"  --------------------------------------------------\n";
    
    int correct=0;
    for(int a=0;a<=1;a++){
        for(int b=0;b<=1;b++){
            double theta_a=(rand()%6283)/1000.0; // random 0 to 2*pi
            double theta_b=(rand()%6283)/1000.0;
            
            ComplexPE A=encrypt_bit_complex(cc,kp,a,theta_a);
            ComplexPE B=encrypt_bit_complex(cc,kp,b,theta_b);
            
            // NAND in complex domain: R(NAND) = psi*R(A) + psi*R(B) + 2*psi - R(A)*R(B)
            // Step 1: psi*A + psi*B
            Ciphertext<DCRTPoly> t1_re,t1_im,t2_re,t2_im,sum_re,sum_im;
            complex_mult(cc,ct_psi_re,ct_psi_im,A.a_re,A.a_im,t1_re,t1_im);
            complex_mult(cc,ct_psi_re,ct_psi_im,B.a_re,B.a_im,t2_re,t2_im);
            complex_add(cc,t1_re,t1_im,t2_re,t2_im,sum_re,sum_im);
            // Step 2: + 2*psi
            Ciphertext<DCRTPoly> term1_re=cc->EvalAdd(sum_re,ct_2psi_re);
            Ciphertext<DCRTPoly> term1_im=cc->EvalAdd(sum_im,ct_2psi_im);
            // Step 3: A*B
            Ciphertext<DCRTPoly> prod_re,prod_im;
            complex_mult(cc,A.a_re,A.a_im,B.a_re,B.a_im,prod_re,prod_im);
            // Step 4: term1 - prod
            ComplexPE result;
            result.a_re=cc->EvalSub(term1_re,prod_re);
            result.a_im=cc->EvalSub(term1_im,prod_im);
            result.b_re=A.b_re; result.b_im=A.b_im;
            
            double decoded=decode_complex(cc,kp,result);
            double expected=1.0-a*b;
            bool match=(std::abs(decoded-expected)<0.1);
            if(match)correct++;
            
            std::cout<<"  "<<a<<" "<<b<<" | "
                 <<std::fixed<<std::setprecision(2)<<theta_a<<" "<<theta_b<<" | "
                 <<std::fixed<<std::setprecision(6)<<decoded<<" | "
                 <<expected<<" | "<<(match?"YES":"NO")<<"\n";
        }
    }
    
    std::cout<<"\n  Fractal NAND: "<<correct<<"/4 correct\n";
    std::cout<<"  Fixed points "<<(correct==4?"PRESERVED under phase rotation!":"need work")<<"\n\n";
    
    std::cout<<"  FRACTAL iO: Phase rotation test complete\n\n";
    return 0;
}
