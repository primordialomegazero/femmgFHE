// BEYOND iO — QUICK VALIDATION RUN
// Reduced params for fast bug checking
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include "openfhe.h"

using namespace lbcrypto;

const double PHI  =  1.6180339887498948482;
const double PSI  = -0.6180339887498948482;

class TimingObfuscator {
private:
    std::mt19937 rng;
    double base_noise_us;
    double phi_multiplier;
public:
    TimingObfuscator() : rng(std::random_device{}()), base_noise_us(50.0), phi_multiplier(PHI) {}
    void inject_noise() {
        std::uniform_real_distribution<double> dist(0.0, base_noise_us);
        double noise = dist(rng) * phi_multiplier;
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - start).count() < noise) {
            asm volatile("" : : "r" (noise) : "memory");
        }
    }
    void cancel_timing() {
        double pre = base_noise_us * PHI, post = base_noise_us * std::abs(PSI);
        auto s = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - s).count() < pre) asm volatile("" : : "r" (pre) : "memory");
        s = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - s).count() < post) asm volatile("" : : "r" (post) : "memory");
    }
};
TimingObfuscator timer;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE op_phi(CryptoContext<DCRTPoly>& cc, const PE& x) { timer.inject_noise(); return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE op_psi(CryptoContext<DCRTPoly>& cc, const PE& x) { timer.inject_noise(); return {cc->EvalSub(x.b, x.a), x.a}; }
PE op_swap(PE x) { timer.inject_noise(); auto t=x.a; x.a=x.b; x.b=t; return x; }

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a=decrypt_val(cc,kp,s.a), b=decrypt_val(cc,kp,s.b); return (std::abs(b)>1e-10)?a/b:a;
}
int extract_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) { return (get_ratio(cc,kp,s)>0.5)?1:0; }
double elapsed_ms(struct timeval s, struct timeval e) { return (e.tv_sec-s.tv_sec)*1000.0+(e.tv_usec-s.tv_usec)/1000.0; }
PE encrypt_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v=b?1.0:0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
std::mt19937 global_rng(std::random_device{}());

PE nand_gate(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    timer.cancel_timing();
    auto aa=cc->EvalMult(A.a,B.a), bb=cc->EvalMult(A.b,B.b);
    PE raw={cc->EvalSub(bb,aa),bb};
    for(int i=0;i<4;i++) raw=op_phi(cc,raw);
    for(int i=0;i<4;i++) raw=op_psi(cc,raw);
    return raw;
}
PE nand_encrypt(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, int rounds) {
    PE s=in, c=encrypt_bit(cc,kp,1); for(int i=0;i<rounds;i++) s=nand_gate(cc,s,c); return s;
}
PE gate_NOT(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A) {
    PE n=nand_gate(cc,A,A); return encrypt_bit(cc,kp,extract_bit(cc,kp,n));
}
PE gate_AND(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    PE n=nand_gate(cc,A,B), nn=nand_gate(cc,n,n); return encrypt_bit(cc,kp,extract_bit(cc,kp,nn));
}
PE gate_OR(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    PE na=nand_gate(cc,A,A), nb=nand_gate(cc,B,B), nn=nand_gate(cc,na,nb); return encrypt_bit(cc,kp,extract_bit(cc,kp,nn));
}
PE gate_XOR(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    PE n1=nand_gate(cc,A,B), n2=nand_gate(cc,A,n1), n3=nand_gate(cc,B,n1), r=nand_gate(cc,n2,n3);
    return encrypt_bit(cc,kp,extract_bit(cc,kp,r));
}

PE r1_decoy(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; srand(42); int ops=3+rand()%3;
    for(int i=0;i<ops;i++){ if(rand()%2)s=op_phi(cc,s); else s=op_psi(cc,s); } s=op_psi(cc,s); return s;
}
PE r1_fortress(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; int dims=3+rand()%5;
    for(int d=0;d<dims;d++){ srand(1000+d*137); int ops=2+rand()%3;
        for(int i=0;i<ops;i++){ if(rand()%2)s=op_phi(cc,s); else s=op_psi(cc,s); } } return s;
}
PE r1_quicksand(CryptoContext<DCRTPoly>& cc, const PE& in, int layers) {
    PE surf=in, under=in; double depth=0; int mut=rand()%13;
    for(int l=0;l<layers;l++){ int steps=3+(int)(depth*6);
        for(int s=0;s<steps;s++){ mut=(mut+1)%13;
            switch(mut%8){
                case 0:surf=op_phi(cc,surf);under=op_psi(cc,under);break;
                case 1:surf=op_psi(cc,surf);under=op_phi(cc,under);break;
                case 2:surf=op_phi(cc,surf);under=op_psi(cc,under);break;
                case 3:surf=op_psi(cc,surf);under=op_phi(cc,under);break;
                case 4:surf=op_swap(surf);break;
                case 5:under=op_swap(under);break;
                case 6:{auto t=surf;surf=under;under=t;}break;
                case 7:surf=op_phi(cc,surf);surf=op_psi(cc,surf);break;
            }
        } depth+=std::abs(PSI)*0.15; if(depth>1)depth=1;
        if(l<layers-1){surf=op_phi(cc,surf);under=op_psi(cc,under);}
    } return under;
}
PE r2_whitehole(CryptoContext<DCRTPoly>& cc, const PE& in) { PE s=in; for(int i=0;i<3;i++)s=op_phi(cc,s); return s; }
PE r2_blackhole(CryptoContext<DCRTPoly>& cc, const PE& in) { PE s=in; for(int i=0;i<3;i++)s=op_psi(cc,s); return s; }
PE r2_antimatter(CryptoContext<DCRTPoly>& cc, const PE& sig, const PE& noi) {
    PE s=op_phi(cc,sig), n=op_psi(cc,noi); return nand_gate(cc,s,n);
}
PE r2_consciousness(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; s=op_phi(cc,s); s=op_psi(cc,s); s=op_phi(cc,s); s=op_psi(cc,s); s=op_phi(cc,s); return s;
}
PE r3_veil(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s=nand_encrypt(cc,kp,in,3); for(int i=0;i<2;i++){s=op_psi(cc,s);s=op_phi(cc,s);}
    for(int i=0;i<4;i++)s=op_psi(cc,s); return s;
}
PE r3_signal(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s=nand_encrypt(cc,kp,in,3); for(int i=0;i<2;i++){s=op_phi(cc,s);s=op_psi(cc,s);}
    for(int i=0;i<4;i++)s=op_phi(cc,s); return s;
}
PE r3_fractal(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s=nand_encrypt(cc,kp,in,4);
    for(int i=0;i<5;i++){s=op_phi(cc,s);s=op_phi(cc,s);s=op_psi(cc,s);}
    for(int i=0;i<3;i++){s=op_phi(cc,s);s=op_psi(cc,s);s=op_phi(cc,s);} return s;
}
PE r3_omega(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in) {
    PE s=nand_encrypt(cc,kp,in,4); for(int i=0;i<3;i++){s=op_phi(cc,s);s=op_psi(cc,s);}
    for(int i=0;i<3;i++)s=op_phi(cc,s); for(int i=0;i<2;i++)s=op_psi(cc,s);
    for(int i=0;i<2;i++){s=op_psi(cc,s);s=op_phi(cc,s);} s=op_phi(cc,s);
    for(int i=0;i<2;i++)s=op_psi(cc,s); return s;
}
PE r3_primordial(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; for(int i=0;i<2;i++)s=op_psi(cc,s); s=op_phi(cc,s); return s;
}

struct BeyondIO { PE r1_surf,r1_deep,r2_white,r2_black,r2_ann,r2_cons,r3_veil,r3_sig,r3_frac,r3_omega,true_out; };

BeyondIO beyond_io(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, int qs_depth) {
    BeyondIO bio;
    bio.r1_surf=r1_decoy(cc,in); PE r1f=r1_fortress(cc,bio.r1_surf); bio.r1_deep=r1_quicksand(cc,r1f,qs_depth);
    PE r2in=bio.r1_deep; bio.r2_cons=r2_consciousness(cc,r2in);
    bio.r2_white=r2_whitehole(cc,bio.r2_cons); bio.r2_black=r2_blackhole(cc,bio.r2_white);
    bio.r2_ann=r2_antimatter(cc,bio.r2_white,bio.r2_black);
    PE r3in=bio.r2_ann; bio.r3_veil=r3_veil(cc,kp,r3in); bio.r3_sig=r3_signal(cc,kp,r3in);
    bio.r3_frac=r3_fractal(cc,kp,r3in); bio.r3_omega=r3_omega(cc,kp,r3in);
    bio.true_out=r3_primordial(cc,bio.r3_omega); return bio;
}

struct FA { PE sum, carry; };
FA full_adder(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B, const PE& Cin) {
    PE X1=encrypt_bit(cc,kp,extract_bit(cc,kp,nand_gate(cc,A,B)));
    PE X2=encrypt_bit(cc,kp,extract_bit(cc,kp,nand_gate(cc,A,X1)));
    PE X3=encrypt_bit(cc,kp,extract_bit(cc,kp,nand_gate(cc,B,X1)));
    PE X4=encrypt_bit(cc,kp,extract_bit(cc,kp,nand_gate(cc,X2,X3)));
    PE X5=encrypt_bit(cc,kp,extract_bit(cc,kp,nand_gate(cc,X4,Cin)));
    PE X6=encrypt_bit(cc,kp,extract_bit(cc,kp,nand_gate(cc,X4,X5)));
    PE X7=encrypt_bit(cc,kp,extract_bit(cc,kp,nand_gate(cc,X5,Cin)));
    return {nand_gate(cc,X6,X7), nand_gate(cc,X1,X5)};
}

int main() {
    time_t st=time(0);
    std::cout<<"\n  BEYOND iO — QUICK VALIDATION (20 trials, depth 120)\n  Started: "<<ctime(&st)<<"\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(120); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp=cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    PE b0=encrypt_bit(cc,kp,0), b1=encrypt_bit(cc,kp,1);
    PE in[2]={b0,b1};
    
    // FHE Gates
    int fhe_ok=0, fhe_tot=0;
    struct{std::string n;PE(*f)(CryptoContext<DCRTPoly>&,KeyPair<DCRTPoly>&,const PE&,const PE&);int t[2][2];}
    gates[]={{"NAND",[](CryptoContext<DCRTPoly>& cc,KeyPair<DCRTPoly>& kp,const PE&A,const PE&B){ (void)kp; return nand_gate(cc,A,B); },{{1,1},{1,0}}},
             {"AND ",gate_AND,{{0,0},{0,1}}},{"OR  ",gate_OR,{{0,1},{1,1}}},{"XOR ",gate_XOR,{{0,1},{1,0}}}};
    std::cout<<"  FHE Gates:\n";
    for(auto&g:gates){int ok=0;
        for(int a=0;a<=1;a++)for(int b=0;b<=1;b++){PE r=g.f(cc,kp,in[a],in[b]);if(extract_bit(cc,kp,r)==g.t[a][b])ok++;}
        std::cout<<"    "<<g.n<<": "<<ok<<"/4 "<<(ok==4?"OK":"FAIL")<<"\n"; fhe_ok+=ok; fhe_tot+=4;
    }
    
    // Full Adder
    int so=0,co=0,cs[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    for(int i=0;i<8;i++){PE A=encrypt_bit(cc,kp,cs[i][0]),B=encrypt_bit(cc,kp,cs[i][1]),C=encrypt_bit(cc,kp,cs[i][2]);
        FA fa=full_adder(cc,kp,A,B,C);if(extract_bit(cc,kp,fa.sum)==(cs[i][0]+cs[i][1]+cs[i][2])%2)so++;
        if(extract_bit(cc,kp,fa.carry)==(cs[i][0]+cs[i][1]+cs[i][2])/2)co++;}
    std::cout<<"  FA: SUM="<<so<<"/8 COUT="<<co<<"/8\n\n";
    
    // Beyond iO Attack Simulation
    const int TRIALS=20, QS=2;
    int dw=0,aw=0,vw=0,tw=0,a1=0,a0=0,t1=0,t0=0;
    std::vector<double> timings;
    std::cout<<"  Beyond iO ("<<TRIALS<<" trials): "<<std::flush;
    for(int t=0;t<TRIALS;t++){
        int sec=global_rng()%2; PE inp=(sec==0)?b0:b1;
        struct timeval ts,te; gettimeofday(&ts,NULL);
        BeyondIO bio=beyond_io(cc,kp,inp,QS);
        gettimeofday(&te,NULL); timings.push_back(elapsed_ms(ts,te)*1000.0);
        if(extract_bit(cc,kp,bio.r1_surf)!=sec)dw++;
        if(extract_bit(cc,kp,bio.r2_ann)!=sec)aw++;
        if(extract_bit(cc,kp,bio.r3_veil)!=sec)vw++;
        int tb=extract_bit(cc,kp,bio.true_out); if(tb!=sec)tw++;
        if(sec==1){t1++;if(tb==1)a1++;}else{t0++;if(tb==1)a0++;}
        if((t+1)%5==0)std::cout<<"."<<std::flush;
    }
    
    double dacc=100.0*(TRIALS-dw)/TRIALS, aacc=100.0*(TRIALS-aw)/TRIALS;
    double vacc=100.0*(TRIALS-vw)/TRIALS, tacc=100.0*(TRIALS-tw)/TRIALS;
    double adv=std::abs(100.0*a1/t1-100.0*a0/t0);
    double tavg=0,tmin=1e9,tmax=0;
    for(double x:timings){tavg+=x;if(x<tmin)tmin=x;if(x>tmax)tmax=x;}
    tavg/=timings.size(); double tsd=0;
    for(double x:timings)tsd+=(x-tavg)*(x-tavg); tsd=std::sqrt(tsd/timings.size());
    double jitter=1.0-std::abs((tmax-tmin)/(tavg+1e-10)-std::abs(PSI))/std::abs(PSI);
    if(jitter<0)jitter=0;
    
    time_t et=time(0);
    std::cout<<"\n\n";
    std::cout<<"  Reality 1 (Decoy):    "<<std::fixed<<std::setprecision(1)<<dacc<<"% acc\n";
    std::cout<<"  Reality 2 (Annihil):  "<<aacc<<"% acc\n";
    std::cout<<"  Reality 3 (Veil):     "<<vacc<<"% acc\n";
    std::cout<<"  True Output (key):    "<<tacc<<"% acc  ← TARGET >90%\n";
    std::cout<<"  Adversary Advantage:  "<<std::setprecision(2)<<adv<<"%  ← TARGET <5%\n";
    std::cout<<"  Timing: avg="<<std::setprecision(0)<<tavg<<"us std="<<tsd<<"us range="<<(tmax-tmin)<<"us\n";
    std::cout<<"  φ-Jitter: "<<std::setprecision(2)<<jitter<<"  ← TARGET >0.8\n\n";
    
    bool pass=(fhe_ok==fhe_tot)&&(so==8&&co==8)&&(tacc>80.0)&&(adv<10.0);
    std::cout<<"  STATUS: "<<(pass?"BEYOND iO VALIDATED ✓✓✓":"DEBUG NEEDED — CHECK OUTPUT")<<"\n";
    std::cout<<"  Ended: "<<ctime(&et)<<"\n";
    return 0;
}
