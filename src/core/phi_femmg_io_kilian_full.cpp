// FEmmg-iO: Kilian + Barrington + CRT5 + Heterogeneous ZANS
// Full random invertible matrices, Gauss-Jordan inverse
// N matrices use N+1 randomizers, R0=I, R_N=I
// Product preserved: M0'*M1'*M2' = M^3

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <random>
#include <functional>
#include <cmath>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

const double PHI = 1.618033988749895;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

class KilianFullIO {
    static constexpr int64_t moduli[5] = {
        1073643521, 1073692673, 1073750017, 1073815553, 1073872897
    };
    static constexpr int64_t inv12=357919402, inv123=589973977, inv1234=197295683, inv12345=1004546623;
    static const int W=5;
    using Matrix=vector<vector<int64_t>>;

    int64_t mod(int64_t v,int64_t m){return((v%m)+m)%m;}
    int64_t minv(int64_t a,int64_t m){int64_t t=0,nt=1,r=m,nr=mod(a,m);while(nr){int64_t q=r/nr,tmp=t;t=nt;nt=tmp-q*nt;tmp=r;r=nr;nr=tmp-q*nr;}return(r>1)?-1:mod(t,m);}

    int64_t crt5_combine(int64_t r[5]){
        __int128 accum=r[0],prod=moduli[0];
        int64_t diff=mod((int64_t)(r[1]-accum%moduli[1]),moduli[1]),coeff=(int64_t)(((__int128)diff*inv12)%moduli[1]);
        accum=accum+prod*coeff;prod=prod*moduli[1];
        diff=mod((int64_t)(r[2]-accum%moduli[2]),moduli[2]);coeff=(int64_t)(((__int128)diff*inv123)%moduli[2]);
        accum=accum+prod*coeff;prod=prod*moduli[2];
        diff=mod((int64_t)(r[3]-accum%moduli[3]),moduli[3]);coeff=(int64_t)(((__int128)diff*inv1234)%moduli[3]);
        accum=accum+prod*coeff;prod=prod*moduli[3];
        diff=mod((int64_t)(r[4]-accum%moduli[4]),moduli[4]);coeff=(int64_t)(((__int128)diff*inv12345)%moduli[4]);
        accum=accum+prod*coeff;
        return(int64_t)accum;
    }

    Matrix build_companion(int64_t v,int64_t mod){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W-1;i++)M[i][i+1]=this->mod(v,mod);M[W-1][W-1]=1;return M;}
    Matrix I(){Matrix M(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)M[i][i]=1;return M;}
    Matrix mmul(const Matrix&A,const Matrix&B,int64_t mod){Matrix C(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)for(int k=0;k<W;k++)C[i][j]=this->mod(C[i][j]+this->mod(A[i][k]*B[k][j],mod),mod);return C;}

    Matrix random_invertible(int64_t mod,mt19937_64& rng){
        uniform_int_distribution<int64_t> d(1,mod-1);
        Matrix L(W,vector<int64_t>(W,0)),U(W,vector<int64_t>(W,0));
        for(int i=0;i<W;i++){L[i][i]=1;U[i][i]=d(rng);for(int j=0;j<i;j++)L[i][j]=d(rng);for(int j=i+1;j<W;j++)U[i][j]=d(rng);}
        return mmul(L,U,mod);
    }

    Matrix inverse(const Matrix&A,int64_t modulus){
        Matrix aug(W,vector<int64_t>(2*W,0));
        for(int i=0;i<W;i++){for(int j=0;j<W;j++)aug[i][j]=A[i][j];aug[i][W+i]=1;}
        for(int i=0;i<W;i++){int p=i;while(p<W&&aug[p][i]==0)p++;swap(aug[i],aug[p]);
            int64_t iv=minv(aug[i][i],modulus);for(int j=0;j<2*W;j++)aug[i][j]=this->mod(aug[i][j]*iv,modulus);
            for(int k=0;k<W;k++)if(k!=i&&aug[k][i]){int64_t f=aug[k][i];for(int j=0;j<2*W;j++)aug[k][j]=this->mod(aug[k][j]-this->mod(f*aug[i][j],modulus),modulus);}}
        Matrix R(W,vector<int64_t>(W,0));for(int i=0;i<W;i++)for(int j=0;j<W;j++)R[i][j]=aug[i][W+j];return R;
    }

    int64_t eval_mod_kilian(int64_t x,int64_t modulus,int variant_id){
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(15);params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096);params.SetSecurityLevel(HEStd_NotSet);
        auto cc=GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        auto keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);

        auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)}));};
        auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
        auto anchor=enc(0);int64_t half=modulus/2;auto M_enc_ct=enc(half);

        Ciphertext<DCRTPoly> zans_anchor=enc(0);
        function<Ciphertext<DCRTPoly>(const Ciphertext<DCRTPoly>&)> stabilize;
        if(variant_id==0)stabilize=[&](const Ciphertext<DCRTPoly>& ct){auto r=ct;r=cc->EvalAdd(r,zans_anchor);r=cc->EvalAdd(r,zans_anchor);r=cc->EvalAdd(r,zans_anchor);return r;};
        else if(variant_id==1)stabilize=[&](const Ciphertext<DCRTPoly>& ct){int64_t p=7919;auto sp=cc->EvalAdd(enc(p),enc(mod(-p,modulus)));sp=cc->EvalAdd(sp,zans_anchor);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,zans_anchor);return r;};
        else if(variant_id==2)stabilize=[&](const Ciphertext<DCRTPoly>& ct){auto r=ct;int st=static_cast<int>(5*PHI);for(int i=0;i<st;i++)r=cc->EvalAdd(r,zans_anchor);return r;};
        else if(variant_id==3)stabilize=[&](const Ciphertext<DCRTPoly>& ct){auto bell=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));bell=cc->EvalAdd(bell,zans_anchor);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,zans_anchor);return r;};
        else stabilize=[&](const Ciphertext<DCRTPoly>& ct){return cc->EvalAdd(ct,zans_anchor);};

        auto divine=[&](const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b){
            auto s=cc->EvalAdd(a,M_enc_ct);auto bk=cc->EvalSub(s,M_enc_ct);
            auto ov=cc->EvalSub(a,bk);auto r=cc->EvalMult(a,b);
            r=stabilize(r);auto dv=cc->EvalMult(ov,zans_anchor);r=cc->EvalAdd(r,dv);return r;
        };

        int64_t v=mod(x+1,modulus);
        Matrix M=build_companion(v,modulus);
        mt19937_64 rng(time(nullptr)+variant_id*12345);

        // 3 matrices, 4 randomizers: R0=I, R1, R2, R3=I
        Matrix R0=I(), R1=random_invertible(modulus,rng), R2=random_invertible(modulus,rng), R3=I();
        Matrix R0i=I(), R1i=inverse(R1,modulus), R2i=inverse(R2,modulus), R3i=I();

        // Kilian: M0'=R0*M*R1i, M1'=R1*M*R2i, M2'=R2*M*R3i
        Matrix M0p=mmul(mmul(R0,M,modulus),R1i,modulus);
        Matrix M1p=mmul(mmul(R1,M,modulus),R2i,modulus);
        Matrix M2p=mmul(mmul(R2,M,modulus),R3i,modulus);

        // Encrypt all entries
        vector<vector<Ciphertext<DCRTPoly>>> emat0(W,vector<Ciphertext<DCRTPoly>>(W));
        vector<vector<Ciphertext<DCRTPoly>>> emat1(W,vector<Ciphertext<DCRTPoly>>(W));
        vector<vector<Ciphertext<DCRTPoly>>> emat2(W,vector<Ciphertext<DCRTPoly>>(W));
        for(int i=0;i<W;i++)for(int j=0;j<W;j++){
            emat0[i][j]=enc(M0p[i][j]);emat1[i][j]=enc(M1p[i][j]);emat2[i][j]=enc(M2p[i][j]);
        }

        vector<Ciphertext<DCRTPoly>> state(W);state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);
        auto zero_ct=enc(0);

        for(int step=0;step<3;step++){
            auto& emat=(step==0)?emat0:(step==1)?emat1:emat2;
            vector<Ciphertext<DCRTPoly>> ns(W,zero_ct);
            for(int j=0;j<W;j++){auto accum=zero_ct;for(int i=0;i<W;i++){auto prod=divine(state[i],emat[i][j]);accum=cc->EvalAdd(accum,prod);}accum=stabilize(accum);ns[j]=accum;}
            state=ns;
        }
        return dec(state[3]);
    }

public:
    int64_t evaluate(int64_t x){int64_t r[5];for(int i=0;i<5;i++)r[i]=eval_mod_kilian(x,moduli[i],i);return crt5_combine(r);}

    void run_test(){
        cout<<"\n  FEmmg-iO: Kilian (Full Random) + Barrington + CRT5\n";
        cout<<"  f(x)=(x+1)^3 via Kilian-randomized 5x5 matrices\n";
        cout<<"  N matrices, N+1 randomizers, R0=I, R_N=I\n";
        cout<<"  Date: "<<ts()<<"\n\n";
        cout<<"  "<<string(55,'-')<<"\n"<<setw(10)<<"x"<<setw(20)<<"Kilian iO"<<setw(20)<<"Expected"<<"\n"<<string(55,'-')<<"\n";

        vector<int64_t> tests={0,1,2,3,5};
        bool ok=true;
        for(int64_t x:tests){int64_t e=(x+1)*(x+1)*(x+1);int64_t r=evaluate(x);bool o=(r==e);if(!o)ok=false;cout<<setw(10)<<x<<setw(20)<<r<<setw(20)<<e<<(o?"  OK":"  FAIL")<<"\n";}
        cout<<string(55,'-')<<"\n  Result: "<<(ok?"ALL CORRECT":"ERRORS DETECTED")<<"\n\n";
    }
};

int main(){KilianFullIO io;io.run_test();return 0;}
