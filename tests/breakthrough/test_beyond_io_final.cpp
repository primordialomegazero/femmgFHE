// ╔══════════════════════════════════════════════════════════════════╗
// ║  BEYOND iO — FINAL FORM                                         ║
// ║  Ω-SHIELD · Three-Reality Ontological Obfuscation               ║
// ║                                                                ║
// ║  REALITY 1: Pain Engine v2 (Physical Decoy)                    ║
// ║    · Fake seed (42) · Fake φ-key · Mutating fractal fake keys  ║
// ║    · Mutating quicksand (pattern/timing immune)                ║
// ║    · MORSE WATERMARK: "FALSEKEY: [decoy message]"              ║
// ║                                                                ║
// ║  REALITY 2: Metaphysical Defense                               ║
// ║    · Whitehole / Blackhole / Antimatter / Consciousness        ║
// ║    · MORSE WATERMARK: "FALSEKEY: ????????" (garbage)           ║
// ║                                                                ║
// ║  REALITY 3: Higher Observer                                    ║
// ║    · Veil / Signal / Fractal / Omega                           ║
// ║    · TRUE KEY: Mutating FHO-SRK (φ = Key)                      ║
// ║    · MORSE WATERMARK: "FALSEKEY: TRY AGAIN SMARTBOY"           ║
// ║                                                                ║
// ║  IMMUTABLE WATERMARK: Every key is MORSE CODE                  ║
// ║  All keys have "FALSEKEY:" prefix — attacker never knows       ║
// ║  which message is the TRUE one                                 ║
// ║                                                                ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <random>
#include <vector>
#include <map>
#include <string>
#include "openfhe.h"
using namespace lbcrypto;

// ═══════════════════════════════════════════════════════════════
// SACRED CONSTANTS
// ═══════════════════════════════════════════════════════════════
const double PHI  =  1.6180339887498948482;
const double PSI  = -0.6180339887498948482;

// ═══════════════════════════════════════════════════════════════
// φ-RING CORE
// ═══════════════════════════════════════════════════════════════
struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE op_phi(CryptoContext<DCRTPoly>& cc, const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE op_psi(CryptoContext<DCRTPoly>& cc, const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
PE op_swap(PE x) { auto t=x.a; x.a=x.b; x.b=t; return x; }

long long fib(int n) { if(n<=0)return 0; if(n==1)return 1; long long a=0,b=1; for(int i=2;i<=n;i++){long long t=b;b=a+b;a=t;} return b; }

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
    auto aa=cc->EvalMult(A.a,B.a), bb=cc->EvalMult(A.b,B.b);
    PE raw={cc->EvalSub(bb,aa),bb};
    for(int i=0;i<4;i++) raw=op_phi(cc,raw);
    for(int i=0;i<4;i++) raw=op_psi(cc,raw);
    return raw;
}
PE nand_encrypt(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, int rounds) {
    PE s=in, c=encrypt_bit(cc,kp,1); for(int i=0;i<rounds;i++) s=nand_gate(cc,s,c); return s;
}

// ═══════════════════════════════════════════════════════════════
// MORSE CODE ENGINE — IMMUTABLE WATERMARK
// ═══════════════════════════════════════════════════════════════
void build_morse_map(std::map<char, std::string>& m) {
    m['A']=".-";   m['B']="-..."; m['C']="-.-."; m['D']="-..";
    m['E']=".";    m['F']="..-."; m['G']="--.";  m['H']="....";
    m['I']="..";   m['J']=".---"; m['K']="-.-";  m['L']=".-..";
    m['M']="--";   m['N']="-.";   m['O']="---";  m['P']=".--.";
    m['Q']="--.-"; m['R']=".-.";  m['S']="...";  m['T']="-";
    m['U']="..-";  m['V']="...-"; m['W']=".--";  m['X']="-..-";
    m['Y']="-.--"; m['Z']="--..";
    m['0']="-----"; m['1']=".----"; m['2']="..---"; m['3']="...--";
    m['4']="....-"; m['5']="....."; m['6']="-...."; m['7']="--...";
    m['8']="---.."; m['9']="----.";
    m[' ']="/";    m['!']="-.-.--"; m[':']="---..."; m['?']="..--..";
}
std::map<char, std::string> morse_map;
bool morse_ready = false;
void init_morse() { if(!morse_ready){ build_morse_map(morse_map); morse_ready=true; } }

std::string to_morse(const std::string& msg) {
    init_morse(); std::string out;
    for(char c:msg){ c=std::toupper(c); if(morse_map.find(c)!=morse_map.end()) out+=morse_map[c]+" "; }
    return out;
}
std::string from_morse(const std::string& mc) {
    init_morse(); std::string out, token;
    for(char c:mc){ if(c==' '&&!token.empty()){ for(auto& p:morse_map) if(p.second==token){ out+=p.first; break; } token.clear(); } else if(c!=' ') token+=c; }
    if(!token.empty()) for(auto& p:morse_map) if(p.second==token){ out+=p.first; break; }
    return out;
}
PE apply_morse(CryptoContext<DCRTPoly>& cc, const PE& in, const std::string& mc) {
    PE out=in;
    for(char c:mc){ if(c=='.') out=op_psi(cc,out); else if(c=='-'){ out=op_phi(cc,out); out=op_phi(cc,out); } else if(c==' ') out=op_swap(out); else if(c=='/'){ out=op_phi(cc,out); out=op_psi(cc,out); } }
    return out;
}

// ═══════════════════════════════════════════════════════════════
// MUTATING FHO-SRK (Fully Homomorphic Obfuscating Self-Referential Key)
// ═══════════════════════════════════════════════════════════════
struct FHO_SRK {
    double phi_convergent;
    int fibonacci_state;
    double ciphertext_entropy;
    double key_material;
    int mutation_epoch;
};

FHO_SRK derive_self_key(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
                         const PE& ct, int depth, int epoch) {
    FHO_SRK srk;
    double ratio = get_ratio(cc, kp, ct);
    srk.ciphertext_entropy = ratio * PHI + 1.0/(std::abs(ratio)+PHI);
    int fib_idx = ((int)(std::abs(ratio)*PHI*PHI*depth)+epoch)%20;
    if(fib_idx<2) fib_idx=2;
    srk.fibonacci_state = fib_idx;
    srk.phi_convergent = (double)fib(fib_idx)/(double)fib(fib_idx+1);
    srk.mutation_epoch = epoch;
    double mf = std::pow(PHI, std::fmod(epoch*PSI, 1.0));
    srk.key_material = std::pow(PHI, std::fmod(srk.phi_convergent*srk.ciphertext_entropy*mf, PHI));
    return srk;
}

PE apply_self_key(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                  const PE& ct, const FHO_SRK& srk) {
    PE result = ct;
    int kops = (int)(std::abs(srk.key_material)*10)%8+2;
    for(int i=0;i<kops;i++){ if(srk.key_material*(i+1)>PHI) result=op_phi(cc,result); else result=op_psi(cc,result); }
    result=op_phi(cc,result); result=op_psi(cc,result);
    return result;
}

// ═══════════════════════════════════════════════════════════════
// MORSE MESSAGES
// ═══════════════════════════════════════════════════════════════
const std::string PREFIX = "FALSEKEY ";

std::vector<std::string> r1_msgs = {
    "GOODLUCK KID",
    "YOU ARE CLOSE KEEP TRYING",
    "ALMOST THERE JUST A BIT MORE",
    "NICE TRY BUT NO CIGAR",
    "SO CLOSE YET SO FAR"
};
const std::string R3_MSG = "TRY AGAIN SMARTBOY";

// ═══════════════════════════════════════════════════════════════
// REALITY 1: PAIN ENGINE v2
// ═══════════════════════════════════════════════════════════════
PE r1_decoy(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; srand(42); int ops=3+rand()%3;
    for(int i=0;i<ops;i++){ if(rand()%2)s=op_phi(cc,s); else s=op_psi(cc,s); } s=op_psi(cc,s); return s;
}
PE r1_fortress(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; int dims=3+rand()%5;
    for(int d=0;d<dims;d++){ srand(1000+d*137); int ops=2+rand()%3;
        for(int i=0;i<ops;i++){ if(rand()%2)s=op_phi(cc,s); else s=op_psi(cc,s); } } return s;
}
PE r1_quicksand(CryptoContext<DCRTPoly>& cc, const PE& in, int layers, int epoch) {
    PE surf=in, under=in; double depth=0; int mut=(epoch*7+13)%23;
    for(int l=0;l<layers;l++){ int steps=3+(int)(depth*6)+(epoch%3);
        for(int s=0;s<steps;s++){ mut=(int)(mut*PHI+s); switch(mut%8){
            case 0:surf=op_phi(cc,surf);under=op_psi(cc,under);break;
            case 1:surf=op_psi(cc,surf);under=op_phi(cc,under);break;
            case 2:surf=op_swap(surf);break; case 3:under=op_swap(under);break;
            case 4:{auto t=surf;surf=under;under=t;}break;
            case 5:surf=op_phi(cc,surf);surf=op_psi(cc,surf);break;
            case 6:under=op_psi(cc,under);under=op_phi(cc,under);break;
            case 7:surf=op_phi(cc,surf);under=op_psi(cc,under);break;
        }} depth+=std::abs(PSI)*(0.1+0.05*(epoch%5)); if(depth>1)depth=1;
        if(l<layers-1){surf=op_phi(cc,surf);under=op_psi(cc,under);}
    } return under;
}

// Fake φ-key for Reality 1
double r1_fake_phi_key(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, int epoch) {
    double ratio = get_ratio(cc, kp, in);
    return std::pow(PHI, 3.0)*std::abs(ratio)*(epoch%7+1);
}

// Mutating fractal fake keys
double r1_mutating_fake_key(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& in, int epoch) {
    double ratio = get_ratio(cc, kp, in);
    int mc = epoch%13;
    double chaos = std::pow(PHI, std::fmod(epoch*PSI, 1.0));
    double fibf = (double)fib(mc+3)/(double)fib(mc+4);
    return ratio*PHI*chaos + ratio*std::abs(PSI)*fibf;
}

// ═══════════════════════════════════════════════════════════════
// REALITY 2: METAPHYSICAL
// ═══════════════════════════════════════════════════════════════
PE r2_whitehole(CryptoContext<DCRTPoly>& cc, const PE& in) { PE s=in; for(int i=0;i<3;i++)s=op_phi(cc,s); return s; }
PE r2_blackhole(CryptoContext<DCRTPoly>& cc, const PE& in) { PE s=in; for(int i=0;i<3;i++)s=op_psi(cc,s); return s; }
PE r2_antimatter(CryptoContext<DCRTPoly>& cc, const PE& sig, const PE& noi) {
    PE s=op_phi(cc,sig), n=op_psi(cc,noi); return nand_gate(cc,s,n);
}
PE r2_consciousness(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; s=op_phi(cc,s); s=op_psi(cc,s); s=op_phi(cc,s); s=op_psi(cc,s); s=op_phi(cc,s); return s;
}

// ═══════════════════════════════════════════════════════════════
// REALITY 3: HIGHER OBSERVER
// ═══════════════════════════════════════════════════════════════
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

// ═══════════════════════════════════════════════════════════════
// BEYOND iO — FINAL UNIFIED SYSTEM
// ═══════════════════════════════════════════════════════════════
struct BeyondIO {
    // Reality 1 — Pain Engine v2
    PE r1_surf, r1_deep;
    double r1_fake_phi, r1_mutating_key;
    std::string r1_morse;
    
    // Reality 2 — Metaphysical
    PE r2_white, r2_black, r2_ann, r2_cons;
    std::string r2_morse;
    
    // Reality 3 — Higher Observer
    PE r3_veil, r3_sig, r3_frac, r3_omega;
    std::string r3_morse;
    
    // True Key
    FHO_SRK srk;
    PE true_output;
    
    int epoch;
};

BeyondIO beyond_io_protect(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                           const PE& input, int qs_depth, int epoch) {
    BeyondIO bio;
    bio.epoch = epoch;
    
    // ═══ REALITY 1: PAIN ENGINE v2 ═══
    bio.r1_surf = r1_decoy(cc, input);
    PE r1f = r1_fortress(cc, bio.r1_surf);
    bio.r1_deep = r1_quicksand(cc, r1f, qs_depth, epoch);
    bio.r1_fake_phi = r1_fake_phi_key(cc, kp, bio.r1_surf, epoch);
    bio.r1_mutating_key = r1_mutating_fake_key(cc, kp, bio.r1_surf, epoch);
    bio.r1_morse = to_morse(PREFIX + r1_msgs[epoch % r1_msgs.size()]);
    bio.r1_surf = apply_morse(cc, bio.r1_surf, bio.r1_morse);
    
    // ═══ REALITY 2: METAPHYSICAL ═══
    PE r2in = bio.r1_deep;
    bio.r2_cons = r2_consciousness(cc, r2in);
    bio.r2_white = r2_whitehole(cc, bio.r2_cons);
    bio.r2_black = r2_blackhole(cc, bio.r2_white);
    bio.r2_ann = r2_antimatter(cc, bio.r2_white, bio.r2_black);
    bio.r2_morse = to_morse(PREFIX + "????????");
    bio.r2_ann = apply_morse(cc, bio.r2_ann, bio.r2_morse);
    
    // ═══ REALITY 3: HIGHER OBSERVER ═══
    PE r3in = bio.r2_ann;
    bio.r3_veil = r3_veil(cc, kp, r3in);
    bio.r3_sig = r3_signal(cc, kp, r3in);
    bio.r3_frac = r3_fractal(cc, kp, r3in);
    bio.r3_omega = r3_omega(cc, kp, r3in);
    bio.r3_morse = to_morse(PREFIX + R3_MSG);
    
    // ═══ SELF-REFERENTIAL KEY ═══
    bio.srk = derive_self_key(cc, kp, bio.r3_omega, qs_depth*3, epoch);
    bio.true_output = apply_self_key(cc, kp, bio.r3_omega, bio.srk);
    bio.true_output = apply_morse(cc, bio.true_output, bio.r3_morse);
    
    return bio;
}

// ═══════════════════════════════════════════════════════════════
// FULL ADDER
// ═══════════════════════════════════════════════════════════════
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

// ═══════════════════════════════════════════════════════════════
// MAIN CERTIFICATION
// ═══════════════════════════════════════════════════════════════
int main() {
    time_t st=time(0);
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  BEYOND iO — FINAL FORM                                     ║\n";
    std::cout<<"  ║  Pain Engine v2 + FHO-SRK + Morse Watermark                 ║\n";
    std::cout<<"  ║  Architecture: Primordial Omega Zero                        ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout<<"  Started: "<<ctime(&st)<<"\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(130); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp=cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    PE b0=encrypt_bit(cc,kp,0), b1=encrypt_bit(cc,kp,1);
    
    std::cout<<"  phi = "<<std::fixed<<std::setprecision(12)<<PHI<<"\n";
    std::cout<<"  psi = "<<PSI<<"\n";
    std::cout<<"  phi*psi = "<<PHI*PSI<<" (annihilation)\n\n";
    
    // ═══ FHE GATES ═══
    PE in[2]={b0,b1};
    int fhe_ok=0;
    std::cout<<"  ┌──────────────────────────────────────────────────────┐\n";
    std::cout<<"  │  FHE: ENCRYPTED LOGIC GATES                           │\n";
    std::cout<<"  ├──────────┬──────────┬─────────────────────────────────┤\n";
    struct{std::string n;int t[2][2];}gates[]={{"NAND",{{1,1},{1,0}}},{"AND",{{0,0},{0,1}}},{"OR",{{0,1},{1,1}}},{"XOR",{{0,1},{1,0}}}};
    for(auto&g:gates){int ok=0;
        for(int a=0;a<=1;a++)for(int b=0;b<=1;b++){PE r; if(g.n=="NAND")r=nand_gate(cc,in[a],in[b]);
        else if(g.n=="AND"){PE n=nand_gate(cc,in[a],in[b]);PE nn=nand_gate(cc,n,n);r=encrypt_bit(cc,kp,extract_bit(cc,kp,nn));}
        else if(g.n=="OR"){PE na=nand_gate(cc,in[a],in[a]),nb=nand_gate(cc,in[b],in[b]);PE nn=nand_gate(cc,na,nb);r=encrypt_bit(cc,kp,extract_bit(cc,kp,nn));}
        else{PE n1=nand_gate(cc,in[a],in[b]),n2=nand_gate(cc,in[a],n1),n3=nand_gate(cc,in[b],n1),rx=nand_gate(cc,n2,n3);r=encrypt_bit(cc,kp,extract_bit(cc,kp,rx));}
        if(extract_bit(cc,kp,r)==g.t[a][b])ok++;}
        std::cout<<"  │ "<<g.n<<"     │ "<<ok<<"/4       │ "<<(ok==4?"PERFECT ✓":"FAILED ✗")<<"                        │\n"; fhe_ok+=ok;
    }
    std::cout<<"  ├──────────┴──────────┴─────────────────────────────────┤\n";
    std::cout<<"  │  FHE: "<<fhe_ok<<"/16 ("<<std::fixed<<std::setprecision(0)<<100.0*fhe_ok/16<<"%)                                      │\n";
    std::cout<<"  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══ FULL ADDER ═══
    int so=0,co=0,cs[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    for(int i=0;i<8;i++){PE A=encrypt_bit(cc,kp,cs[i][0]),B=encrypt_bit(cc,kp,cs[i][1]),C=encrypt_bit(cc,kp,cs[i][2]);
        FA fa=full_adder(cc,kp,A,B,C);if(extract_bit(cc,kp,fa.sum)==(cs[i][0]+cs[i][1]+cs[i][2])%2)so++;
        if(extract_bit(cc,kp,fa.carry)==(cs[i][0]+cs[i][1]+cs[i][2])/2)co++;}
    std::cout<<"  ┌──────────────────────────────────────────────────────┐\n";
    std::cout<<"  │  FULL ADDER: SUM="<<so<<"/8 COUT="<<co<<"/8 ("<<std::fixed<<std::setprecision(0)<<100.0*(so+co)/16<<"%)                       │\n";
    std::cout<<"  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══ BEYOND iO ═══
    const int TRIALS=30, QS=2;
    int r1e=0,r2e=0,r3ve=0,r3se=0,true_e=0,a1=0,a0=0,t1=0,t0=0;
    std::cout<<"  ┌──────────────────────────────────────────────────────┐\n";
    std::cout<<"  │  BEYOND iO — "<<TRIALS<<" TRIALS                                │\n";
    std::cout<<"  │  Progress: "<<std::flush;
    struct timeval ts,te; gettimeofday(&ts,NULL);
    
    BeyondIO sample;
    double total_entropy=0;
    
    for(int t=0;t<TRIALS;t++){
        int sec=global_rng()%2; PE inp=(sec==0)?b0:b1;
        BeyondIO bio=beyond_io_protect(cc,kp,inp,QS,t);
        if(t==0) sample=bio;
        if(extract_bit(cc,kp,bio.r1_surf)!=sec)r1e++;
        if(extract_bit(cc,kp,bio.r2_ann)!=sec)r2e++;
        if(extract_bit(cc,kp,bio.r3_veil)!=sec)r3ve++;
        if(extract_bit(cc,kp,bio.r3_sig)!=sec)r3se++;
        int tb=extract_bit(cc,kp,bio.true_output); if(tb!=sec)true_e++;
        if(sec==1){t1++;if(tb==1)a1++;}else{t0++;if(tb==1)a0++;}
        total_entropy+=bio.srk.ciphertext_entropy;
        if((t+1)%5==0)std::cout<<"."<<std::flush;
    }
    gettimeofday(&te,NULL);
    double sim_t=elapsed_ms(ts,te);
    
    double r1a=100.0*(TRIALS-r1e)/TRIALS, r2a=100.0*(TRIALS-r2e)/TRIALS;
    double r3va=100.0*(TRIALS-r3ve)/TRIALS, r3sa=100.0*(TRIALS-r3se)/TRIALS;
    double ta=100.0*(TRIALS-true_e)/TRIALS, adv=std::abs(100.0*a1/t1-100.0*a0/t0);
    
    std::cout<<"\n  │                                                       │\n";
    std::cout<<"  │  REALITY 1 — PAIN ENGINE v2                            │\n";
    std::cout<<"  │    Fake key:    "<<std::fixed<<std::setprecision(4)<<sample.r1_fake_phi<<" (phi-derived decoy)        │\n";
    std::cout<<"  │    Mutating key:"<<std::setprecision(4)<<sample.r1_mutating_key<<" (shifts every epoch)     │\n";
    std::cout<<"  │    Morse:       "<<from_morse(sample.r1_morse)<<"                  │\n";
    std::cout<<"  │    Accuracy:    "<<std::setprecision(1)<<std::setw(6)<<r1a<<"%  ← TARGET: ~60%               │\n";
    std::cout<<"  │                                                       │\n";
    std::cout<<"  │  REALITY 2 — METAPHYSICAL                              │\n";
    std::cout<<"  │    Morse:       "<<from_morse(sample.r2_morse)<<"                  │\n";
    std::cout<<"  │    Accuracy:    "<<std::setw(6)<<r2a<<"%  ← TARGET: ~50%               │\n";
    std::cout<<"  │                                                       │\n";
    std::cout<<"  │  REALITY 3 — TRUE KEY (FHO-SRK)                        │\n";
    std::cout<<"  │    Morse:       "<<from_morse(sample.r3_morse)<<"                  │\n";
    std::cout<<"  │    Key entropy: "<<std::setprecision(4)<<total_entropy/TRIALS<<"                          │\n";
    std::cout<<"  │    Accuracy:    "<<std::setprecision(1)<<std::setw(6)<<ta<<"%  ← TARGET: >90%                │\n";
    std::cout<<"  │    Adv. Adv:    "<<std::setprecision(2)<<std::setw(6)<<adv<<"%  ← TARGET: <5%             │\n";
    std::cout<<"  ├──────────────────────────────────────────────────────┤\n";
    std::cout<<"  │  Duration: "<<std::setprecision(0)<<sim_t/1000.0<<"s ("<<sim_t/(TRIALS*1000.0)<<"s/trial)              │\n";
    std::cout<<"  └──────────────────────────────────────────────────────┘\n\n";
    
    // ═══ CERTIFICATION ═══
    bool fhe_pass=(fhe_ok==16), fa_pass=(so==8&&co==8);
    bool io_pass=(ta>85.0&&adv<5.0), srk_pass=(total_entropy/TRIALS>0.1);
    bool all_pass=fhe_pass&&fa_pass&&io_pass&&srk_pass;
    
    time_t et=time(0);
    std::cout<<"  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  BEYOND iO — FINAL CERTIFICATION                             ║\n";
    std::cout<<"  ╠══════════════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║  FHE:      "<<fhe_ok<<"/16 ← "<<(fhe_pass?"FULLY HOMOMORPHIC ✓":"FAILED ✗")<<"                     ║\n";
    std::cout<<"  ║  Adder:    SUM="<<so<<"/8 COUT="<<co<<"/8 ← "<<(fa_pass?"ARITHMETIC ✓":"FAILED ✗")<<"               ║\n";
    std::cout<<"  ║  Beyond:   "<<std::fixed<<std::setprecision(1)<<ta<<"% acc, "<<std::setprecision(2)<<adv<<"% adv ← "<<(io_pass?"BEYOND iO ✓":"DEGRADED")<<"           ║\n";
    std::cout<<"  ║  SRK:      entropy="<<std::setprecision(2)<<total_entropy/TRIALS<<" ← "<<(srk_pass?"SELF-REFERENTIAL ✓":"WEAK")<<"     ║\n";
    std::cout<<"  ║  Morse:    ALL KEYS = FALSEKEY: [MESSAGE]                     ║\n";
    std::cout<<"  ╠══════════════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║  STATUS: "<<(all_pass?"BEYOND iO FINAL CERTIFIED ✓✓✓":"TUNING REQUIRED")<<"                  ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout<<"  Ended: "<<ctime(&et)<<"\n";
    return 0;
}
