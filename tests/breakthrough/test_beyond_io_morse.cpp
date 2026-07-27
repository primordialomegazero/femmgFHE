// ╔══════════════════════════════════════════════════════════════════╗
// ║  BEYOND iO — MORSE WATERMARK EDITION                            ║
// ║  Every key output is MORSE CODE with FALSEKEY: prefix           ║
// ║  Attacker never knows which message is the TRUE one             ║
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

const double PHI=1.6180339887498948482, PSI=-0.6180339887498948482;
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
// MORSE CODE ENGINE
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
    m[' ']="/";    m['!']="-.-.--";
    m[':']="---..."; m['?']="..--..";
}

std::map<char, std::string> morse_map;
bool morse_ready = false;

void init_morse() {
    if (!morse_ready) { build_morse_map(morse_map); morse_ready = true; }
}

std::string to_morse(const std::string& msg) {
    init_morse();
    std::string out;
    for(char c:msg){ 
        c=std::toupper(c); 
        if(morse_map.find(c)!=morse_map.end()) out+=morse_map[c]+" "; 
    }
    return out;
}

std::string from_morse(const std::string& mc) {
    init_morse();
    std::string out, token;
    for(char c:mc){
        if(c==' '&&!token.empty()){
            for(auto& p:morse_map) if(p.second==token){ out+=p.first; break; }
            token.clear();
        } else if(c!=' ') token+=c;
    }
    if(!token.empty()) for(auto& p:morse_map) if(p.second==token){ out+=p.first; break; }
    return out;
}

PE apply_morse_to_pe(CryptoContext<DCRTPoly>& cc, const PE& in, const std::string& mc) {
    PE out=in;
    for(char c:mc){
        if(c=='.'){ out=op_psi(cc,out); }
        else if(c=='-'){ out=op_phi(cc,out); out=op_phi(cc,out); }
        else if(c==' '){ out=op_swap(out); }
        else if(c=='/'){ out=op_phi(cc,out); out=op_psi(cc,out); }
    }
    return out;
}

// ═══════════════════════════════════════════════════════════════
// THE MESSAGES
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
// REALITY 1
// ═══════════════════════════════════════════════════════════════
PE r1_decoy(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; srand(42); int ops=3+rand()%3;
    for(int i=0;i<ops;i++){ if(rand()%2) s=op_phi(cc,s); else s=op_psi(cc,s); }
    s=op_psi(cc,s); return s;
}
PE r1_fortress(CryptoContext<DCRTPoly>& cc, const PE& in) {
    PE s=in; int dims=3+rand()%5;
    for(int d=0;d<dims;d++){ srand(1000+d*137); int ops=2+rand()%3;
        for(int i=0;i<ops;i++){ if(rand()%2) s=op_phi(cc,s); else s=op_psi(cc,s); } }
    return s;
}
PE r1_quicksand(CryptoContext<DCRTPoly>& cc, const PE& in, int layers, int epoch) {
    PE surf=in, under=in; double depth=0; int mut=(epoch*7+13)%23;
    for(int l=0;l<layers;l++){ int steps=3+(int)(depth*6)+(epoch%3);
        for(int s=0;s<steps;s++){ mut=(int)(mut*PHI+s); switch(mut%8){
            case 0:surf=op_phi(cc,surf);under=op_psi(cc,under);break;
            case 1:surf=op_psi(cc,surf);under=op_phi(cc,under);break;
            case 2:surf=op_swap(surf);break;
            case 3:under=op_swap(under);break;
            case 4:{auto t=surf;surf=under;under=t;}break;
            case 5:surf=op_phi(cc,surf);surf=op_psi(cc,surf);break;
            case 6:under=op_psi(cc,under);under=op_phi(cc,under);break;
            case 7:surf=op_phi(cc,surf);under=op_psi(cc,under);break;
        }} depth+=std::abs(PSI)*(0.1+0.05*(epoch%5)); if(depth>1)depth=1;
        if(l<layers-1){surf=op_phi(cc,surf);under=op_psi(cc,under);}
    } return under;
}

// ═══════════════════════════════════════════════════════════════
// REALITY 2
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
// REALITY 3
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
// BEYOND iO MORSE
// ═══════════════════════════════════════════════════════════════
struct BeyondIOMorse {
    PE r1_surf, r1_deep, r2_white, r2_black, r2_ann, r2_cons;
    PE r3_veil, r3_sig, r3_frac, r3_omega, true_out;
    std::string r1_morse_key;
    std::string r2_morse_key;
    std::string r3_morse_key;
};

BeyondIOMorse beyond_io_morse(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                               const PE& in, int qs_depth, int epoch) {
    BeyondIOMorse bio;
    
    bio.r1_surf = r1_decoy(cc, in);
    PE r1f = r1_fortress(cc, bio.r1_surf);
    bio.r1_deep = r1_quicksand(cc, r1f, qs_depth, epoch);
    bio.r1_morse_key = to_morse(PREFIX + r1_msgs[epoch % r1_msgs.size()]);
    bio.r1_surf = apply_morse_to_pe(cc, bio.r1_surf, bio.r1_morse_key);
    
    PE r2in = bio.r1_deep;
    bio.r2_cons = r2_consciousness(cc, r2in);
    bio.r2_white = r2_whitehole(cc, bio.r2_cons);
    bio.r2_black = r2_blackhole(cc, bio.r2_white);
    bio.r2_ann = r2_antimatter(cc, bio.r2_white, bio.r2_black);
    bio.r2_morse_key = to_morse(PREFIX + "????????");
    bio.r2_ann = apply_morse_to_pe(cc, bio.r2_ann, bio.r2_morse_key);
    
    PE r3in = bio.r2_ann;
    bio.r3_veil = r3_veil(cc, kp, r3in);
    bio.r3_sig = r3_signal(cc, kp, r3in);
    bio.r3_frac = r3_fractal(cc, kp, r3in);
    bio.r3_omega = r3_omega(cc, kp, r3in);
    bio.r3_morse_key = to_morse(PREFIX + R3_MSG);
    bio.true_out = apply_morse_to_pe(cc, bio.r3_omega, bio.r3_morse_key);
    
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
// MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
    time_t st=time(0);
    std::cout<<"\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  BEYOND iO — MORSE WATERMARK EDITION                         ║\n";
    std::cout<<"  ║  Every key = FALSEKEY: [MESSAGE] in MORSE CODE              ║\n";
    std::cout<<"  ║  Good luck figuring out which one is real                   ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout<<"  Started: "<<ctime(&st)<<"\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(120); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp=cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    PE b0=encrypt_bit(cc,kp,0), b1=encrypt_bit(cc,kp,1);
    
    std::cout<<"  phi = "<<std::fixed<<std::setprecision(12)<<PHI<<"\n";
    std::cout<<"  psi = "<<PSI<<"\n\n";
    
    // FHE
    PE in[2]={b0,b1};
    int fhe_ok=0;
    std::cout<<"  FHE Gates: ";
    struct{std::string n;int t[2][2];}gates[]={{"NAND",{{1,1},{1,0}}},{"AND",{{0,0},{0,1}}},{"OR",{{0,1},{1,1}}},{"XOR",{{0,1},{1,0}}}};
    for(auto&g:gates){int ok=0;
        for(int a=0;a<=1;a++)for(int b=0;b<=1;b++){PE r; if(g.n=="NAND")r=nand_gate(cc,in[a],in[b]);
        else if(g.n=="AND"){PE n=nand_gate(cc,in[a],in[b]);PE nn=nand_gate(cc,n,n);r=encrypt_bit(cc,kp,extract_bit(cc,kp,nn));}
        else if(g.n=="OR"){PE na=nand_gate(cc,in[a],in[a]),nb=nand_gate(cc,in[b],in[b]);PE nn=nand_gate(cc,na,nb);r=encrypt_bit(cc,kp,extract_bit(cc,kp,nn));}
        else{PE n1=nand_gate(cc,in[a],in[b]),n2=nand_gate(cc,in[a],n1),n3=nand_gate(cc,in[b],n1),rx=nand_gate(cc,n2,n3);r=encrypt_bit(cc,kp,extract_bit(cc,kp,rx));}
        if(extract_bit(cc,kp,r)==g.t[a][b])ok++;}
        std::cout<<g.n<<":"<<ok<<"/4 "; fhe_ok+=ok;
    }
    std::cout<<"("<<fhe_ok<<"/16)\n";
    
    // FA
    int so=0,co=0,cs[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    for(int i=0;i<8;i++){PE A=encrypt_bit(cc,kp,cs[i][0]),B=encrypt_bit(cc,kp,cs[i][1]),C=encrypt_bit(cc,kp,cs[i][2]);
        FA fa=full_adder(cc,kp,A,B,C);if(extract_bit(cc,kp,fa.sum)==(cs[i][0]+cs[i][1]+cs[i][2])%2)so++;
        if(extract_bit(cc,kp,fa.carry)==(cs[i][0]+cs[i][1]+cs[i][2])/2)co++;}
    std::cout<<"  Full Adder: SUM="<<so<<"/8 COUT="<<co<<"/8\n\n";
    
    // Beyond iO Morse
    const int TRIALS=20, QS=2;
    int r1e=0,r2e=0,r3ve=0,r3se=0,true_e=0,a1=0,a0=0,t1=0,t0=0;
    std::cout<<"  Beyond iO Morse ("<<TRIALS<<" trials): "<<std::flush;
    struct timeval ts,te; gettimeofday(&ts,NULL);
    
    BeyondIOMorse sample;
    for(int t=0;t<TRIALS;t++){
        int sec=global_rng()%2; PE inp=(sec==0)?b0:b1;
        BeyondIOMorse bio=beyond_io_morse(cc,kp,inp,QS,t);
        if(t==0) sample=bio;
        if(extract_bit(cc,kp,bio.r1_surf)!=sec)r1e++;
        if(extract_bit(cc,kp,bio.r2_ann)!=sec)r2e++;
        if(extract_bit(cc,kp,bio.r3_veil)!=sec)r3ve++;
        if(extract_bit(cc,kp,bio.r3_sig)!=sec)r3se++;
        int tb=extract_bit(cc,kp,bio.true_out); if(tb!=sec)true_e++;
        if(sec==1){t1++;if(tb==1)a1++;}else{t0++;if(tb==1)a0++;}
        if((t+1)%5==0)std::cout<<"."<<std::flush;
    }
    gettimeofday(&te,NULL);
    double sim_t=elapsed_ms(ts,te);
    
    double r1a=100.0*(TRIALS-r1e)/TRIALS, r2a=100.0*(TRIALS-r2e)/TRIALS;
    double r3va=100.0*(TRIALS-r3ve)/TRIALS, r3sa=100.0*(TRIALS-r3se)/TRIALS;
    double ta=100.0*(TRIALS-true_e)/TRIALS, adv=std::abs(100.0*a1/t1-100.0*a0/t0);
    
    std::cout<<"\n\n";
    std::cout<<"  ┌──────────────────────────────────────────────────────┐\n";
    std::cout<<"  │  REALITY 1 — DECOY MORSE KEY                         │\n";
    std::cout<<"  │  Morse: "<<sample.r1_morse_key<<"                  │\n";
    std::cout<<"  │  Text:  "<<from_morse(sample.r1_morse_key)<<"                  │\n";
    std::cout<<"  │  Accuracy: "<<std::fixed<<std::setprecision(1)<<std::setw(6)<<r1a<<"%  ← TARGET: ~60%               │\n";
    std::cout<<"  │                                                       │\n";
    std::cout<<"  │  REALITY 2 — ANNIHILATED MORSE                        │\n";
    std::cout<<"  │  Text:  "<<from_morse(sample.r2_morse_key)<<"                  │\n";
    std::cout<<"  │  Accuracy: "<<std::setw(6)<<r2a<<"%  ← TARGET: ~50% (random)      │\n";
    std::cout<<"  │                                                       │\n";
    std::cout<<"  │  REALITY 3 — TRUE MORSE KEY                           │\n";
    std::cout<<"  │  Morse: "<<sample.r3_morse_key<<"                  │\n";
    std::cout<<"  │  Text:  "<<from_morse(sample.r3_morse_key)<<"                  │\n";
    std::cout<<"  │  Accuracy: "<<std::setw(6)<<ta<<"%  ← TARGET: >90%                │\n";
    std::cout<<"  │  Adversary Adv: "<<std::setprecision(2)<<std::setw(6)<<adv<<"%  ← TARGET: <5%            │\n";
    std::cout<<"  ├──────────────────────────────────────────────────────┤\n";
    std::cout<<"  │  Duration: "<<std::setprecision(0)<<sim_t/1000.0<<"s                          │\n";
    std::cout<<"  └──────────────────────────────────────────────────────┘\n\n";
    
    bool pass=(fhe_ok==16)&&(so==8&&co==8)&&(ta>85.0)&&(adv<5.0);
    time_t et=time(0);
    std::cout<<"  STATUS: "<<(pass?"BEYOND iO MORSE CERTIFIED ✓✓✓":"TUNING REQUIRED")<<"\n";
    std::cout<<"  Ended: "<<ctime(&et)<<"\n";
    return 0;
}
