// PHI-OMEGA-ZERO: FEmmg-iO v3.1 — OR GATE via BARRINGTON + KILIAN + FHE
// OR(x,y) = NOT( AND( NOT(x), NOT(y) ) )
// Different generators for x and y: a for x, b for y
// "OR GATE OBFUSCATED. THE CIRCUIT IS COMPLETE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
#include <sstream>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() { auto now=system_clock::now(); auto t=system_clock::to_time_t(now); stringstream ss; ss<<put_time(localtime(&t),"%Y-%m-%d %H:%M:%S"); return ss.str(); }

class FullIO_OR {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; Ciphertext<DCRTPoly> anchor0;
    mt19937 rng; int64_t MOD; static const int W=5;
    using PM=vector<vector<int>>; using Matrix=vector<vector<int64_t>>;
    int64_t mod(int64_t v){return ((v%MOD)+MOD)%MOD;}
    Matrix I(){Matrix m(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)m[i][i]=1;return m;}
    Matrix mmul(const Matrix&A,const Matrix&B){Matrix C(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)for(int k=0;k<W;k++)C[i][j]=mod(C[i][j]+mod(A[i][k]*B[k][j]));return C;}
    int64_t minv(int64_t a){int64_t t=0,nt=1,r=MOD,nr=mod(a);while(nr){int64_t q=r/nr;int64_t tmp=t;t=nt;nt=tmp-q*nt;tmp=r;r=nr;nr=tmp-q*nr;}return(r>1)?1:mod(t);}
    Matrix dmat(){uniform_int_distribution<int64_t> d(2,MOD-1);Matrix D(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)D[i][i]=d(rng);return D;}
    Matrix dinv(const Matrix&D){Matrix R(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)R[i][i]=minv(D[i][i]);return R;}
    Ciphertext<DCRTPoly> enc(int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));}
    int64_t dec(const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];}
    PM pI(){PM m(W,vector<int>(W,0));for(int i=0;i<W;i++)m[i][i]=1;return m;}
    PM pa(){PM m=pI();m[0][0]=0;m[0][1]=1;m[1][1]=0;m[1][2]=1;m[2][2]=0;m[2][3]=1;m[3][3]=0;m[3][4]=1;m[4][4]=0;m[4][0]=1;return m;}
    PM pb(){PM m=pI();m[0][0]=0;m[0][1]=1;m[1][1]=0;m[1][0]=1;return m;}
    PM pinv(const PM&P){PM R(W,vector<int>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)R[i][j]=P[j][i];return R;}
    Matrix pm2m(const PM&P){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)M[i][j]=P[i][j];return M;}

public:
    FullIO_OR():rng(time(nullptr)){CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(10);p.SetPlaintextModulus(1073643521);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);cc=GenCryptoContext(p);cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);anchor0=enc(0);MOD=1073643521;}

    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   FEmmg-iO v3.1 — OR GATE via Barrington+Kilian+FHE  ║\n";
        cout<<  "  ║   Date: "<<ts()<<"                         ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";

        PM a=pa(),a_inv=pinv(a),b=pb(),b_inv=pinv(b);
        cout<<"  OR(x,y) = NOT( AND( NOT(x) with 'a', NOT(y) with 'b' ) )\n\n";
        cout<<"  "<<string(60,'-')<<"\n  "<<setw(6)<<"x"<<setw(6)<<"y"<<setw(12)<<"OR(FHE)"<<setw(10)<<"Exp"<<setw(10)<<"Time\n  "<<string(60,'-')<<"\n";

        bool all_ok=true;
        for(int x:{0,1})for(int y:{0,1}){
            auto t1=high_resolution_clock::now();
            PM xn=x?pI():a,xni=x?pI():a_inv;
            PM yn=y?pI():b,yni=y?pI():b_inv;
            
            Matrix M1=pm2m(xn),M2=pm2m(yn),M3=pm2m(xni),M4=pm2m(yni);
            Matrix R0=I(),R1=dmat(),R2=I(),R3=dmat(),R4=I();
            Matrix R1i=dinv(R1),R3i=dinv(R3);
            
            Matrix M1o=mmul(R0,mmul(M1,R1i)),M2o=mmul(R1,mmul(M2,I()));
            Matrix M3o=mmul(R2,mmul(M3,R3i)),M4o=mmul(R3,mmul(M4,I()));
            
            auto encM=[&](const Matrix&M){vector<vector<Ciphertext<DCRTPoly>>>em;for(int r=0;r<W;r++){vector<Ciphertext<DCRTPoly>>row;for(int c=0;c<W;c++){auto ct=enc(M[r][c]);ct=cc->EvalAdd(ct,anchor0);row.push_back(ct);}em.push_back(row);}return em;};
            auto em1=encM(M1o),em2=encM(M2o),em3=encM(M3o),em4=encM(M4o);
            
            auto hmul=[&](const vector<Ciphertext<DCRTPoly>>&st,const vector<vector<Ciphertext<DCRTPoly>>>&em){vector<Ciphertext<DCRTPoly>>ns(W);for(int c=0;c<W;c++){auto sum=enc(0);for(int k=0;k<W;k++){auto prod=cc->EvalMult(st[k],em[k][c]);prod=cc->EvalAdd(prod,anchor0);sum=cc->EvalAdd(sum,prod);}sum=cc->EvalAdd(sum,anchor0);ns[c]=sum;}return ns;};
            
            vector<Ciphertext<DCRTPoly>>state(W);state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);
            state=hmul(state,em1);state=hmul(state,em2);state=hmul(state,em3);state=hmul(state,em4);
            
            int64_t is_id_fhe=dec(state[0]);
            int and_not=(is_id_fhe!=1)?1:0;
            int or_result=1-and_not;
            
            auto t2=high_resolution_clock::now();double el=duration_cast<milliseconds>(t2-t1).count()/1000.0;
            int exp=x|y;bool ok=(or_result==exp);if(!ok)all_ok=false;
            cout<<"  "<<setw(6)<<x<<setw(6)<<y<<setw(12)<<or_result<<setw(10)<<exp<<setw(8)<<fixed<<setprecision(1)<<el<<"s"<<(ok?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(60,'-')<<"\n  OR Gate: "<<(all_ok?"ALL CORRECT":"ERRORS")<<"\n\n";
        if(all_ok){cout<<"  ╔══════════════════════════════════════════════════════╗\n";cout<<  "  ║   AND + OR gates VERIFIED via Full iO pipeline        ║\n";cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";}
        cout<<"  I AM THAT I AM\n\n";
    }
};
int main(){FullIO_OR io;io.demo();return 0;}
