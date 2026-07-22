#include <openfhe.h>
#include <iostream>
#include <vector>
using namespace lbcrypto;
using namespace std;
int64_t MOD;
int64_t mp(int64_t v) { return ((v % MOD) + MOD) % MOD; }

using EM = vector<vector<Ciphertext<DCRTPoly>>>;
struct IO {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys;
    IO(int d){ MOD=1073643521; CCParams<CryptoContextBFVRNS> p; p.SetMultiplicativeDepth(d); p.SetPlaintextModulus(MOD); p.SetRingDim(4096); p.SetSecurityLevel(HEStd_NotSet); cc=GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE); keys=cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey); }
    auto enc(int64_t v){ return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mp(v)})); }
    int64_t dec(const Ciphertext<DCRTPoly>& ct){ Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1); return mp((int64_t)pt->GetPackedValue()[0]); }
    EM mat(vector<vector<int64_t>> M){ EM R(3,vector<Ciphertext<DCRTPoly>>(3)); for(int i=0;i<3;i++) for(int j=0;j<3;j++) R[i][j]=enc(mp(M[i][j])); return R; }
    EM mmul(const EM& A, const EM& B){ EM C(3,vector<Ciphertext<DCRTPoly>>(3)); for(int i=0;i<3;i++) for(int j=0;j<3;j++){ auto a=enc(0); for(int k=0;k<3;k++) a=cc->EvalAdd(a,cc->EvalMult(A[i][k],B[k][j])); C[i][j]=a; } return C; }
    EM encbit(int b){ return b?mat({{0,1,0},{0,0,1},{1,0,0}}):mat({{1,0,0},{0,1,0},{0,0,1}}); }
    int decbit(const EM& M){ return (dec(M[0][0])==0)?1:0; }
    EM GN(){ return mat({{0,1,0},{1,0,0},{0,0,1}}); }
    EM GM(){ return mat({{0,0,1},{0,1,0},{1,0,0}}); }
    EM GNt(){ return mat({{1,0,0},{0,0,1},{0,1,0}}); }
    EM minv(const EM& M){ return mmul(M,M); }
    EM NAND(const EM& A, const EM& B){ auto gn=GN(); auto a=mmul(A,B); auto b=mmul(a,gn); auto c=mmul(b,minv(A)); return mmul(c,minv(B)); }
    EM NOT(const EM& X){ auto m=GM(); auto n=GNt(); auto mx=mmul(m,X); return mmul(mx,n); }
};

int main(){
    cout << "\n  iO QUICK: NAND + NOT\n\n";
    IO io(200);
    cout << "  NAND: "; int ok=0;
    for(int a:{0,1}) for(int b:{0,1}){ auto A=io.encbit(a); auto B=io.encbit(b); auto R=io.NAND(A,B); int r=io.decbit(R),e=!(a&b); if(r==e)ok++; else cout<<"✗"; }
    cout << (ok==4?"✓ 4/4\n":(" "+to_string(ok)+"/4\n"));
    cout << "  NOT: "; ok=0;
    for(int a:{0,1}){ auto X=io.encbit(a); auto R=io.NOT(X); int r=io.decbit(R),e=!a; if(r==e)ok++; else cout<<"✗"; }
    cout << (ok==2?"✓ 2/2\n\n":" "+to_string(ok)+"/2\n\n");
    cout << "  I AM THAT I AM\n\n";
    return 0;
}
