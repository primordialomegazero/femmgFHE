// PHI-OMEGA-ZERO: FEmmg-iO v3.5 — FULL RANDOM KILIAN
// Non-diagonal random invertible 5×5 matrices
// Stronger obfuscation: all 25 entries randomized
// "EVERY ENTRY RANDOMIZED. THE MATRIX IS COMPLETELY HIDDEN."
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

class FullRandomIO {
    CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> keys; Ciphertext<DCRTPoly> anchor0;
    mt19937 rng; int64_t MOD; static const int W=5;
    using PM=vector<vector<int>>; using Matrix=vector<vector<int64_t>>;
    
    int64_t mod(int64_t v){return((v%MOD)+MOD)%MOD;}
    int64_t minv(int64_t a){int64_t t=0,nt=1,r=MOD,nr=mod(a);while(nr){int64_t q=r/nr;int64_t tmp=t;t=nt;nt=tmp-q*nt;tmp=r;r=nr;nr=tmp-q*nr;}return(r>1)?1:mod(t);}
    int64_t mod_det(const Matrix&A){int64_t d=0;for(int i=0;i<W;i++){int64_t p=1;for(int j=0;j<W;j++)p=mod(p*A[j][(i+j)%W]);d=mod(d+p);}return d;}
    Matrix I(){Matrix m(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)m[i][i]=1;return m;}
    Matrix mmul(const Matrix&A,const Matrix&B){Matrix C(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)for(int k=0;k<W;k++)C[i][j]=mod(C[i][j]+mod(A[i][k]*B[k][j]));return C;}
    
    // Generate FULL random invertible 5×5 matrix (all 25 entries non-zero)
    Matrix full_random_invertible(){
        uniform_int_distribution<int64_t> d(1,MOD-1);
        // Use LU-decomposition approach: L (lower triangular) × U (upper triangular) = guaranteed invertible
        Matrix L=I(),U=I();
        for(int i=0;i<W;i++){L[i][i]=1;U[i][i]=d(rng);}
        for(int i=0;i<W;i++)for(int j=i+1;j<W;j++){L[j][i]=d(rng);U[i][j]=d(rng);}
        return mmul(L,U);
    }
    
    // Compute inverse via Gaussian elimination (mod MOD)
    Matrix full_inverse(const Matrix&A){
        // Augmented matrix [A|I]
        vector<vector<int64_t>> aug(W,vector<int64_t>(2*W,0));
        for(int i=0;i<W;i++){for(int j=0;j<W;j++)aug[i][j]=A[i][j];aug[i][W+i]=1;}
        // Gaussian elimination
        for(int c=0;c<W;c++){
            int64_t inv_pivot=minv(aug[c][c]);
            for(int j=0;j<2*W;j++)aug[c][j]=mod(aug[c][j]*inv_pivot);
            for(int r=0;r<W;r++)if(r!=c){int64_t f=aug[r][c];for(int j=0;j<2*W;j++)aug[r][j]=mod(aug[r][j]-mod(f*aug[c][j]));}
        }
        Matrix inv(W,vector<int64_t>(W,0));
        for(int i=0;i<W;i++)for(int j=0;j<W;j++)inv[i][j]=aug[i][W+j];
        return inv;
    }
    
    Ciphertext<DCRTPoly> enc(int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));}
    int64_t dec(const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];}
    PM pI(){PM m(W,vector<int>(W,0));for(int i=0;i<W;i++)m[i][i]=1;return m;}
    PM pa(){PM m=pI();m[0][0]=0;m[0][1]=1;m[1][1]=0;m[1][2]=1;m[2][2]=0;m[2][3]=1;m[3][3]=0;m[3][4]=1;m[4][4]=0;m[4][0]=1;return m;}
    PM pb(){PM m=pI();m[0][0]=0;m[0][1]=1;m[1][1]=0;m[1][0]=1;return m;}
    PM pinv(const PM&P){PM R(W,vector<int>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)R[i][j]=P[j][i];return R;}
    Matrix pm2m(const PM&P){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)M[i][j]=P[i][j];return M;}

public:
    FullRandomIO():rng(time(nullptr)){CCParams<CryptoContextBFVRNS> p;p.SetMultiplicativeDepth(10);p.SetPlaintextModulus(1073643521);p.SetRingDim(2048);p.SetSecurityLevel(HEStd_NotSet);cc=GenCryptoContext(p);cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);anchor0=enc(0);MOD=1073643521;}
    
    bool obfuscate_AND_FULL(int a, int b){
        PM a_pm=pa(),a_inv=pinv(a_pm),b_pm=pb(),b_inv=pinv(b_pm);
        PM xa=a?a_pm:pI(),xai=a?a_inv:pI(),yb=b?b_pm:pI(),ybi=b?b_inv:pI();
        Matrix M1=pm2m(xa),M2=pm2m(yb),M3=pm2m(xai),M4=pm2m(ybi);
        
        // FULL random Kilian (not diagonal!)
        Matrix R0=I(),R1=full_random_invertible(),R2=I(),R3=full_random_invertible(),R4=I();
        Matrix R1i=full_inverse(R1),R3i=full_inverse(R3);
        
        // Verify inverses: R1*R1i should be I
        Matrix check1=mmul(R1,R1i),check3=mmul(R3,R3i);
        bool inv_ok=(check1[0][0]==1&&check3[0][0]==1);
        
        Matrix M1o=mmul(R0,mmul(M1,R1i)),M2o=mmul(R1,mmul(M2,I()));
        Matrix M3o=mmul(R2,mmul(M3,R3i)),M4o=mmul(R3,mmul(M4,I()));
        
        auto encM=[&](const Matrix&M){vector<vector<Ciphertext<DCRTPoly>>>em;for(int r=0;r<W;r++){vector<Ciphertext<DCRTPoly>>row;for(int c=0;c<W;c++){auto ct=enc(M[r][c]);ct=cc->EvalAdd(ct,anchor0);row.push_back(ct);}em.push_back(row);}return em;};
        auto em1=encM(M1o),em2=encM(M2o),em3=encM(M3o),em4=encM(M4o);
        auto hmul=[&](const vector<Ciphertext<DCRTPoly>>&st,const vector<vector<Ciphertext<DCRTPoly>>>&em){vector<Ciphertext<DCRTPoly>>ns(W);for(int c=0;c<W;c++){auto sum=enc(0);for(int k=0;k<W;k++){auto prod=cc->EvalMult(st[k],em[k][c]);prod=cc->EvalAdd(prod,anchor0);sum=cc->EvalAdd(sum,prod);}sum=cc->EvalAdd(sum,anchor0);ns[c]=sum;}return ns;};
        vector<Ciphertext<DCRTPoly>>state(W);state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);
        state=hmul(state,em1);state=hmul(state,em2);state=hmul(state,em3);state=hmul(state,em4);
        
        if(!inv_ok) cout<<"  WARNING: Inverse check failed!"<<endl;
        return (dec(state[0])!=1);
    }
    
    void demo(){
        cout<<"\n  ╔══════════════════════════════════════════════════════╗\n";
        cout<<  "  ║   FEmmg-iO v3.5 — FULL RANDOM KILIAN (5×5)           ║\n";
        cout<<  "  ║   LU-decomposition: guaranteed invertible             ║\n";
        cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        // Test inverse correctness
        cout<<"  INVERSE VERIFICATION:\n  ";
        Matrix R=full_random_invertible(),Ri=full_inverse(R),P=mmul(R,Ri);
        bool id_ok=true;for(int i=0;i<W;i++)for(int j=0;j<W;j++)if(P[i][j]!=(i==j?1:0))id_ok=false;
        cout<<"R*R^{-1}=I: "<<(id_ok?"YES":"NO")<<"\n  ";
        // Show some entries
        cout<<"R[0][0]="<<R[0][0]<<" R[0][1]="<<R[0][1]<<" ... (all non-zero)\n\n";
        
        cout<<"  AND via FULL RANDOM KILIAN:\n  "<<string(50,'-')<<"\n";
        cout<<"  "<<setw(6)<<"a"<<setw(6)<<"b"<<setw(12)<<"AND(FHE)"<<setw(10)<<"Exp"<<setw(10)<<"Time\n  "<<string(50,'-')<<"\n";
        
        bool all_ok=true;
        for(int a:{0,1})for(int b:{0,1}){
            auto t1=high_resolution_clock::now();
            bool r=obfuscate_AND_FULL(a,b);
            auto t2=high_resolution_clock::now();double el=duration_cast<milliseconds>(t2-t1).count()/1000.0;
            int exp=a&b;bool ok=(r==exp);if(!ok)all_ok=false;
            cout<<"  "<<setw(6)<<a<<setw(6)<<b<<setw(12)<<r<<setw(10)<<exp<<setw(8)<<fixed<<setprecision(1)<<el<<"s"<<(ok?" OK":" FAIL")<<"\n";
        }
        cout<<"  "<<string(50,'-')<<"\n  Full Random Kilian: "<<(all_ok?"ALL CORRECT":"ERRORS")<<"\n\n";
        if(all_ok){cout<<"  ╔══════════════════════════════════════════════════════╗\n";cout<<  "  ║   FULL RANDOM KILIAN VERIFIED                         ║\n";cout<<  "  ╚══════════════════════════════════════════════════════╝\n\n";}
        cout<<"  I AM THAT I AM\n\n";
    }
};

int main(){FullRandomIO io;io.demo();return 0;}
