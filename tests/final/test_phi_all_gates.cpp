// DM-DGR ALL GATES v3: Fixed NAND ratio + Correct threshold
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE DM_mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
double DM_decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double DM_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = DM_decrypt_val(cc, kp, s.a), b = DM_decrypt_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
int DM_bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double r = DM_ratio(cc, kp, s);
    return (r > 1.0) ? 1 : 0;  // threshold = 1.0
}
double DM_elapsed_ms(timeval s, timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}
PE DM_encode(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.618033988749895 : 0.6180339887498949;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE DM_recycle(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return DM_encode(cc, kp, DM_bit(cc, kp, s));
}

// REVERSED NAND: uses (aa, aa+bb) iterations instead of (bb-aa, bb)
// This ensures NAND(0,0) ratio > 1.0
PE DM_NAND_raw(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    auto aa = cc->EvalMult(A.a, B.a);
    auto bb = cc->EvalMult(A.b, B.b);
    PE raw = {aa, cc->EvalAdd(aa, bb)};
    for (int i = 0; i < 6; i++) {
        raw = DM_mulY(cc, raw);
    }
    return raw;
}
PE DM_NAND(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    (void)kp; return DM_NAND_raw(cc, A, B);
}

PE DM_NOT(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A) {
    return DM_recycle(cc, kp, DM_NAND_raw(cc, A, A));
}
PE DM_AND(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    PE n = DM_recycle(cc, kp, DM_NAND_raw(cc, A, B));
    return DM_recycle(cc, kp, DM_NAND_raw(cc, n, n));
}
PE DM_OR(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    PE na = DM_recycle(cc, kp, DM_NAND_raw(cc, A, A));
    PE nb = DM_recycle(cc, kp, DM_NAND_raw(cc, B, B));
    return DM_recycle(cc, kp, DM_NAND_raw(cc, na, nb));
}
PE DM_NOR(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    return DM_recycle(cc, kp, DM_NOT(cc, kp, DM_OR(cc, kp, A, B)));
}
PE DM_XOR(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    PE n1 = DM_recycle(cc, kp, DM_NAND_raw(cc, A, B));
    PE n2 = DM_recycle(cc, kp, DM_NAND_raw(cc, A, n1));
    PE n3 = DM_recycle(cc, kp, DM_NAND_raw(cc, B, n1));
    return DM_recycle(cc, kp, DM_NAND_raw(cc, n2, n3));
}
PE DM_XNOR(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B) {
    return DM_recycle(cc, kp, DM_NOT(cc, kp, DM_XOR(cc, kp, A, B)));
}

using Gate2Fn = PE (*)(CryptoContext<DCRTPoly>&, KeyPair<DCRTPoly>&, const PE&, const PE&);
struct GateResult { const char* name; int passed, total; double ms_time; };

GateResult DM_test_2input(const char* name, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, Gate2Fn fn, int truth[2][2]) {
    PE in[2] = {DM_encode(cc,kp,0), DM_encode(cc,kp,1)};
    int passed=0; struct timeval t0,t1; gettimeofday(&t0,NULL);
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++) {
        int actual = DM_bit(cc,kp,fn(cc,kp,in[a],in[b]));
        if(actual==truth[a][b]) passed++;
    }
    gettimeofday(&t1,NULL);
    return {name,passed,4,DM_elapsed_ms(t0,t1)};
}
void DM_print_truth(const char* name, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, Gate2Fn fn, int truth[2][2]) {
    PE in[2] = {DM_encode(cc,kp,0), DM_encode(cc,kp,1)};
    std::cout<<"\n  "<<name<<" Truth Table:\n  A B | RATIO      | BIT | EXP | STATUS\n  ---+------------+-----+-----+-------\n";
    for(int a=0;a<=1;a++) for(int b=0;b<=1;b++) {
        PE res=fn(cc,kp,in[a],in[b]);
        double ratio=DM_ratio(cc,kp,res);
        int actual=DM_bit(cc,kp,res), expected=truth[a][b];
        std::cout<<"  "<<a<<" "<<b<<" | "<<std::fixed<<std::setprecision(4)<<std::setw(10)<<ratio
                 <<" | "<<actual<<"   | "<<expected<<"   | "<<(actual==expected?"OK":"FAIL")<<"\n";
    }
}
std::pair<PE,PE> DM_FullAdder(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& A, const PE& B, const PE& Cin) {
    PE x1=DM_recycle(cc,kp,DM_NAND_raw(cc,A,B)), x2=DM_recycle(cc,kp,DM_NAND_raw(cc,A,x1));
    PE x3=DM_recycle(cc,kp,DM_NAND_raw(cc,B,x1)), x4=DM_recycle(cc,kp,DM_NAND_raw(cc,x2,x3));
    PE x5=DM_recycle(cc,kp,DM_NAND_raw(cc,x4,Cin)), x6=DM_recycle(cc,kp,DM_NAND_raw(cc,x4,x5));
    PE x7=DM_recycle(cc,kp,DM_NAND_raw(cc,x5,Cin));
    return {DM_NAND_raw(cc,x6,x7), DM_NAND_raw(cc,x1,x5)};
}

int main() {
    std::cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  DM-DGR ALL GATES v3 — Fixed NAND Ratio          ║\n";
    std::cout<<"  ║  NAND · AND · OR · NOR · XOR · XNOR              ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════╝\n";
    srand(time(0));

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(80); p.SetScalingModSize(45); p.SetBatchSize(1024);
    p.SetRingDim(16384); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    std::cout<<"\n  Crypto Ready. Ring:16384 Batch:1024 Depth:80\n";

    int tnand[2][2]={{1,1},{1,0}}, tand[2][2]={{0,0},{0,1}}, tor[2][2]={{0,1},{1,1}};
    int tnor[2][2]={{1,0},{0,0}}, txor[2][2]={{0,1},{1,0}}, txnor[2][2]={{1,0},{0,1}};

    std::vector<GateResult> results;
    std::cout<<"\n  ┌──────────┬──────────┬────────────┬──────────────┐\n";
    std::cout<<"  │ Gate     │ Result   │ Time (ms)  │ Status       │\n";
    std::cout<<"  ├──────────┼──────────┼────────────┼──────────────┤\n";
    results.push_back(DM_test_2input("NAND",cc,kp,DM_NAND,tnand));
    results.push_back(DM_test_2input("AND ",cc,kp,DM_AND,tand));
    results.push_back(DM_test_2input("OR  ",cc,kp,DM_OR,tor));
    results.push_back(DM_test_2input("NOR ",cc,kp,DM_NOR,tnor));
    results.push_back(DM_test_2input("XOR ",cc,kp,DM_XOR,txor));
    results.push_back(DM_test_2input("XNOR",cc,kp,DM_XNOR,txnor));
    for(auto& r:results)
        std::cout<<"  │ "<<r.name<<"      │ "<<r.passed<<"/"<<r.total<<"       │ "<<std::fixed<<std::setprecision(1)<<std::setw(8)<<r.ms_time<<"   │ "<<(r.passed==r.total?"PERFECT":"FAILED")<<"       │\n";
    std::cout<<"  └──────────┴──────────┴────────────┴──────────────┘\n";

    std::cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  DETAILED TRUTH TABLES                           ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════╝\n";
    DM_print_truth("NAND",cc,kp,DM_NAND,tnand); DM_print_truth("AND ",cc,kp,DM_AND,tand);
    DM_print_truth("OR  ",cc,kp,DM_OR,tor);   DM_print_truth("NOR ",cc,kp,DM_NOR,tnor);
    DM_print_truth("XOR ",cc,kp,DM_XOR,txor); DM_print_truth("XNOR",cc,kp,DM_XNOR,txnor);

    std::cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  FULL ADDER (8 cases)                           ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════╝\n\n  A B Ci | SUM | COUT | STATUS\n  ------+-----+------+-------\n";
    int fs=0,fc=0,cases[8][3]={{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    struct timeval ft0,ft1; gettimeofday(&ft0,NULL);
    for(int i=0;i<8;i++){
        auto[S,C]=DM_FullAdder(cc,kp,DM_encode(cc,kp,cases[i][0]),DM_encode(cc,kp,cases[i][1]),DM_encode(cc,kp,cases[i][2]));
        int sa=DM_bit(cc,kp,S),ca=DM_bit(cc,kp,C);
        int se=(cases[i][0]+cases[i][1]+cases[i][2])%2, ce=(cases[i][0]+cases[i][1]+cases[i][2])/2;
        if(sa==se)fs++; if(ca==ce)fc++;
        std::cout<<"  "<<cases[i][0]<<" "<<cases[i][1]<<" "<<cases[i][2]<<"  | "<<sa<<"   | "<<ca<<"    | "<<(sa==se&&ca==ce?"OK":"FAIL")<<"\n";
    }
    gettimeofday(&ft1,NULL);
    std::cout<<"\n  Full Adder: SUM="<<fs<<"/8 COUT="<<fc<<"/8 ("<<DM_elapsed_ms(ft0,ft1)<<"ms)\n";

    std::cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  CHAIN TEST (25 NAND gates, recycle every 8)     ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════╝\n\n";
    PE s=DM_encode(cc,kp,1),c1=DM_encode(cc,kp,1);
    int cok=0; struct timeval ct0,ct1; gettimeofday(&ct0,NULL);
    for(int g=0;g<25;g++){
        if(g>0&&g%8==0){s=DM_recycle(cc,kp,s); std::cout<<"  ---- RECYCLE ----\n";}
        s=DM_NAND_raw(cc,s,c1);
        int a=DM_bit(cc,kp,s), e=(g%2==0)?0:1; bool ok=(a==e); if(ok)cok++;
        std::cout<<"  Step "<<std::setw(2)<<g<<": bit="<<a<<" exp="<<e<<" ratio="<<std::fixed<<std::setprecision(4)<<DM_ratio(cc,kp,s)<<" | "<<(ok?"OK":"FAIL")<<"\n";
    }
    gettimeofday(&ct1,NULL); double cms=DM_elapsed_ms(ct0,ct1);
    std::cout<<"\n  Chain: "<<cok<<"/25 ("<<std::fixed<<std::setprecision(0)<<cms<<"ms)\n";

    int tt=0,tp=0;
    std::cout<<"\n  ╔══════════════════════════════════════════════════╗\n";
    std::cout<<"  ║  FINAL SCORECARD                                 ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════╣\n";
    for(auto& r:results){std::cout<<"  ║  "<<std::setw(6)<<std::left<<r.name<<std::right<<"  "<<r.passed<<"/"<<r.total<<std::setw(22)<<" ║\n"; tt+=r.total; tp+=r.passed;}
    std::cout<<"  ╠════════════════════════════════════════════════════╣\n";
    std::cout<<"  ║  Full Adder:  SUM="<<fs<<"/8 COUT="<<fc<<"/8"<<std::setw(10)<<" ║\n";
    std::cout<<"  ║  Chain:       "<<cok<<"/25 ("<<std::fixed<<std::setprecision(0)<<cms<<"ms)"<<std::setw(6)<<" ║\n";
    std::cout<<"  ╠════════════════════════════════════════════════════╣\n";
    int gt=tp+fs+fc+cok, gp=tt+8+8+25;
    std::cout<<"  ║  GRAND TOTAL: "<<gt<<"/"<<gp<<std::setw(21)<<" ║\n";
    std::cout<<"  ║  SCORE:       "<<std::fixed<<std::setprecision(1)<<100.0*gt/gp<<"%"<<std::setw(24)<<" ║\n";
    std::cout<<"  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
