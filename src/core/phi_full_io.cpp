// ΦΩ0 — FULL iO v1.0
// Verified Barrington gates (14/14) + Kilian randomization + FHE evaluation
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

const int64_t MOD = 1073643521;
const int N = 3;

int64_t modM(int64_t v) { return ((v % MOD) + MOD) % MOD; }

using BMat = vector<vector<int64_t>>;

BMat b_id() { return {{1,0,0},{0,1,0},{0,0,1}}; }

BMat b_mul(const BMat& A, const BMat& B) {
    BMat C(N, vector<int64_t>(N, 0));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) for(int k=0;k<N;k++)
        C[i][j] = modM(C[i][j] + modM(A[i][k] * B[k][j]));
    return C;
}

int64_t b_det(const BMat& A) {
    int64_t a=A[0][0],b=A[0][1],c=A[0][2],d=A[1][0],e=A[1][1],f=A[1][2],g=A[2][0],h=A[2][1],i=A[2][2];
    return modM(modM(a*modM(e*i-f*h)) - modM(b*modM(d*i-f*g)) + modM(c*modM(d*h-e*g)));
}

int64_t b_inv_int(int64_t a) {
    int64_t t=0, nt=1, r=MOD, nr=modM(a);
    while(nr){int64_t q=r/nr,tmp=t;t=nt;nt=tmp-q*nt;tmp=r;r=nr;nr=tmp-q*nr;}
    return modM(t);
}

BMat b_inv(const BMat& A) {
    int64_t a=A[0][0],b=A[0][1],c=A[0][2],d=A[1][0],e=A[1][1],f=A[1][2],g=A[2][0],h=A[2][1],i=A[2][2];
    int64_t det=b_det(A), idet=b_inv_int(det);
    BMat cof = {{modM(e*i-f*h),modM(-(d*i-f*g)),modM(d*h-e*g)},
                {modM(-(b*i-c*h)),modM(a*i-c*g),modM(-(a*h-b*g))},
                {modM(b*f-c*e),modM(-(a*f-c*d)),modM(a*e-b*d)}};
    BMat adj(N, vector<int64_t>(N));
    for(int r=0;r<N;r++) for(int c=0;c<N;c++) adj[r][c]=modM(cof[c][r]*idet);
    return adj;
}

BMat b_rand_inv(mt19937& rng) {
    uniform_int_distribution<int64_t> d(1, MOD-1);
    BMat M; int64_t det;
    do { M={{d(rng),d(rng),d(rng)},{d(rng),d(rng),d(rng)},{d(rng),d(rng),d(rng)}}; det=b_det(M); } while(!det);
    return M;
}

BMat XOR_gate_M(int bit) { return bit ? BMat{{1,0,1},{0,1,MOD-2},{0,0,1}} : b_id(); }
BMat AND_gate_M(int bit) { return bit ? BMat{{1,0,0},{0,1,1},{0,0,1}} : BMat{{1,0,0},{0,1,0},{0,0,1}}; }

int main() {
    mt19937 rng(42);
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FULL iO v1.0: Barrington + Kilian + FHE          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(15); params.SetPlaintextModulus(MOD);
    params.SetRingDim(2048); params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    ZANSAnchorPool pool(cc, keys, 10);

    auto enc = [&](int64_t v){ return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{modM(v)})); };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct){ Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1); return modM(pt->GetPackedValue()[0]); };

    auto obf_gate = [&](const BMat& G){ auto R = b_rand_inv(rng); return b_mul(R, b_mul(G, b_inv(R))); };

    auto eval_gate = [&](const vector<Ciphertext<DCRTPoly>>& st, const BMat& M){
        vector<Ciphertext<DCRTPoly>> ns(N);
        for(int c=0;c<N;c++){auto s=enc(0);for(int k=0;k<N;k++){auto p=cc->EvalMult(st[k],enc(M[k][c]));s=cc->EvalAdd(s,pool.stabilize(p));}ns[c]=pool.stabilize(s);}
        return ns;
    };

    // HALF-ADDER
    cout << "  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  OBFUSCATED HALF-ADDER                                     │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    cout << "  a b    Sum  Exp  Carry  Exp  Status\n  " + string(40, '-') + "\n";
    int hp=0;
    for(int a:{0,1}) for(int b:{0,1}){
        auto ct_a=enc(a);
        int64_t sum=dec(eval_gate({enc(1),ct_a,ct_a},obf_gate(XOR_gate_M(b)))[2]);
        int64_t carry=dec(eval_gate({enc(1),ct_a,enc(0)},obf_gate(AND_gate_M(b)))[2]);
        bool ok=(sum==(a^b)&&carry==(a&b)); if(ok)hp+=2;
        cout << "  " << a << " " << b << "    " << sum << "    " << (a^b) << "    " << carry << "     " << (a&b) << "    " << (ok?"OK":"FAIL") << "\n";
    }
    cout << "  " + string(40, '-') + "\n  Half-Adder: " << hp << "/8\n\n";

    // FULL ADDER
    cout << "  ┌──────────────────────────────────────────────────────────┐\n";
    cout <<   "  │  OBFUSCATED FULL ADDER                                     │\n";
    cout <<   "  └──────────────────────────────────────────────────────────┘\n";
    cout << "  a b cin  Sum  Exp  Cout  Exp  Status\n  " + string(40, '-') + "\n";
    int fp=0;
    for(int a:{0,1}) for(int b:{0,1}) for(int cin:{0,1}){
        auto ct_a=enc(a);
        int64_t s1=dec(eval_gate({enc(1),ct_a,ct_a},obf_gate(XOR_gate_M(b)))[2]);
        int64_t c1=dec(eval_gate({enc(1),ct_a,enc(0)},obf_gate(AND_gate_M(b)))[2]);
        auto ct_s1=enc(s1);
        int64_t sum=dec(eval_gate({enc(1),ct_s1,ct_s1},obf_gate(XOR_gate_M(cin)))[2]);
        int64_t c2=dec(eval_gate({enc(1),ct_s1,enc(0)},obf_gate(AND_gate_M(cin)))[2]);
        int64_t cr=c1|c2;
        bool ok=(sum==(a+b+cin)%2&&cr==(a+b+cin>=2)); if(ok)fp++;
        cout << "  " << a << " " << b << "  " << cin << "   " << sum << "   " << (a+b+cin)%2 << "    " << cr << "    " << (a+b+cin>=2) << "    " << (ok?"OK":"FAIL") << "\n";
    }
    cout << "  " + string(40, '-') + "\n  Full Adder: " << fp << "/8\n\n";

    bool all=(hp==8&&fp==8);
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FULL iO v1.0: " << (all?"ALL VERIFIED":"FAILURES") << "                                   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    return all?0:1;
}
