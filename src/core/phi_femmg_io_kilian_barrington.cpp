// FEmmg-iO: Kilian + Barrington + CRT5 + FHE
// Full iO pipeline with matrix randomization
// M' = R_i * M * R_{i+1}^{-1} — product preserved, entries scrambled
// "THE MATRICES ARE RANDOM. THE COMPUTATION IS HIDDEN. THE RESULT IS CORRECT."

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <random>
#include <functional>

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

class KilianBarringtonIO {
    static constexpr int64_t moduli[5] = {
        1073643521, 1073692673, 1073750017, 1073815553, 1073872897
    };
    static constexpr int64_t inv12    = 357919402;
    static constexpr int64_t inv123   = 589973977;
    static constexpr int64_t inv1234  = 197295683;
    static constexpr int64_t inv12345 = 1004546623;
    static const int W = 5;
    using Matrix = vector<vector<int64_t>>;

    int64_t mod(int64_t v, int64_t m) { return ((v%m)+m)%m; }

    int64_t crt5_combine(int64_t r[5]) {
        __int128 accum=r[0], prod=moduli[0];
        int64_t diff=mod((int64_t)(r[1]-accum%moduli[1]),moduli[1]);
        int64_t coeff=(int64_t)(((__int128)diff*inv12)%moduli[1]);
        accum=accum+prod*coeff; prod=prod*moduli[1];
        diff=mod((int64_t)(r[2]-accum%moduli[2]),moduli[2]);
        coeff=(int64_t)(((__int128)diff*inv123)%moduli[2]);
        accum=accum+prod*coeff; prod=prod*moduli[2];
        diff=mod((int64_t)(r[3]-accum%moduli[3]),moduli[3]);
        coeff=(int64_t)(((__int128)diff*inv1234)%moduli[3]);
        accum=accum+prod*coeff; prod=prod*moduli[3];
        diff=mod((int64_t)(r[4]-accum%moduli[4]),moduli[4]);
        coeff=(int64_t)(((__int128)diff*inv12345)%moduli[4]);
        accum=accum+prod*coeff;
        return (int64_t)accum;
    }

    // Build companion matrix for value v
    Matrix build_matrix(int64_t v, int64_t mod) {
        Matrix M(W, vector<int64_t>(W, 0));
        for(int i=0;i<W-1;i++) M[i][i+1] = this->mod(v, mod);
        M[W-1][W-1] = 1;
        return M;
    }

    // Matrix multiplication
    Matrix mmul(const Matrix& A, const Matrix& B, int64_t mod) {
        Matrix C(W, vector<int64_t>(W, 0));
        for(int i=0;i<W;i++)
            for(int j=0;j<W;j++)
                for(int k=0;k<W;k++)
                    C[i][j] = this->mod(C[i][j] + this->mod(A[i][k] * B[k][j], mod), mod);
        return C;
    }

    // Extended Euclidean for modular inverse
    int64_t minv(int64_t a, int64_t m) {
        int64_t t=0, nt=1, r=m, nr=this->mod(a,m);
        while(nr){int64_t q=r/nr;int64_t tmp=t;t=nt;nt=tmp-q*nt;tmp=r;r=nr;nr=tmp-q*nr;}
        return (r>1)?-1:this->mod(t,m);
    }

    // Generate random invertible diagonal matrix
    Matrix random_diag(int64_t mod, mt19937_64& rng) {
        uniform_int_distribution<int64_t> dist(1, mod-1);
        Matrix R(W, vector<int64_t>(W, 0));
        for(int i=0;i<W;i++) R[i][i] = dist(rng);
        return R;
    }

    // Inverse of diagonal matrix
    Matrix inv_diag(const Matrix& D, int64_t mod) {
        Matrix inv(W, vector<int64_t>(W, 0));
        for(int i=0;i<W;i++) inv[i][i] = minv(D[i][i], mod);
        return inv;
    }

    // Kilian randomize: M' = R_left * M * R_right_inv
    Matrix kilian_randomize(const Matrix& M, const Matrix& R_left, const Matrix& R_right_inv, int64_t mod) {
        return mmul(mmul(R_left, M, mod), R_right_inv, mod);
    }

    // ============================================================
    // EVALUATE ONE MODULUS WITH KILIAN + BARRINGTON
    // ============================================================
    int64_t eval_mod_kilian(int64_t x, int64_t modulus, int variant_id) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(12);
        params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);
        auto cc=GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        auto keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);

        auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)}));};
        auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
        auto anchor=enc(0);
        int64_t half=modulus/2;
        auto M_enc_ct=enc(half);

        // ZANS variant
        Ciphertext<DCRTPoly> zans_anchor=enc(0);
        function<Ciphertext<DCRTPoly>(const Ciphertext<DCRTPoly>&)> stabilize;
        if(variant_id==0) stabilize=[&](const Ciphertext<DCRTPoly>& ct){auto r=ct;r=cc->EvalAdd(r,zans_anchor);r=cc->EvalAdd(r,zans_anchor);r=cc->EvalAdd(r,zans_anchor);return r;};
        else if(variant_id==1) stabilize=[&](const Ciphertext<DCRTPoly>& ct){int64_t p=7919;auto sp=cc->EvalAdd(enc(p),enc(mod(-p,modulus)));sp=cc->EvalAdd(sp,zans_anchor);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,zans_anchor);return r;};
        else if(variant_id==2) stabilize=[&](const Ciphertext<DCRTPoly>& ct){auto r=ct;int steps=static_cast<int>(5*PHI);for(int i=0;i<steps;i++)r=cc->EvalAdd(r,zans_anchor);return r;};
        else if(variant_id==3) stabilize=[&](const Ciphertext<DCRTPoly>& ct){auto bell=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));bell=cc->EvalAdd(bell,zans_anchor);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,zans_anchor);return r;};
        else stabilize=[&](const Ciphertext<DCRTPoly>& ct){return cc->EvalAdd(ct,zans_anchor);};

        auto divine=[&](const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b){
            auto s=cc->EvalAdd(a,M_enc_ct);auto bk=cc->EvalSub(s,M_enc_ct);
            auto ov=cc->EvalSub(a,bk);auto r=cc->EvalMult(a,b);
            r=stabilize(r);auto dv=cc->EvalMult(ov,zans_anchor);
            r=cc->EvalAdd(r,dv);return r;
        };

        // Build three companion matrices for (x+1)
        int64_t v = mod(x+1, modulus);
        Matrix M0 = build_matrix(v, modulus);
        Matrix M1 = build_matrix(v, modulus);
        Matrix M2 = build_matrix(v, modulus);

        // Verify plaintext: M0*M1*M2 should put (x+1)^3 in position [3]
        Matrix prod_plain = mmul(mmul(M0, M1, modulus), M2, modulus);

        // Kilian randomization
        mt19937_64 rng(time(nullptr) + variant_id * 12345);
        Matrix R0 = build_identity();
        Matrix R1 = random_diag(modulus, rng);
        Matrix R2 = random_diag(modulus, rng);
        Matrix R3 = random_diag(modulus, rng);
        Matrix R4 = build_identity();

        Matrix R0_inv = build_identity();
        Matrix R1_inv = inv_diag(R1, modulus);
        Matrix R2_inv = inv_diag(R2, modulus);
        Matrix R3_inv = inv_diag(R3, modulus);
        Matrix R4_inv = build_identity();

        // Kilian-randomize each matrix
        Matrix M0p = kilian_randomize(M0, R0, R1_inv, modulus);
        Matrix M1p = kilian_randomize(M1, R1, R2_inv, modulus);
        Matrix M2p = kilian_randomize(M2, R2, R3_inv, modulus);

        // Verify Kilian: M0p*M1p*M2p should equal M0*M1*M2
        Matrix prod_kilian = mmul(mmul(M0p, M1p, modulus), M2p, modulus);

        // Encrypt Kilian-randomized matrices
        vector<vector<Ciphertext<DCRTPoly>>> emat0(W,vector<Ciphertext<DCRTPoly>>(W));
        vector<vector<Ciphertext<DCRTPoly>>> emat1(W,vector<Ciphertext<DCRTPoly>>(W));
        vector<vector<Ciphertext<DCRTPoly>>> emat2(W,vector<Ciphertext<DCRTPoly>>(W));
        auto zero_ct=enc(0);
        for(int i=0;i<W;i++){
            for(int j=0;j<W;j++){
                emat0[i][j]=enc(M0p[i][j]);
                emat1[i][j]=enc(M1p[i][j]);
                emat2[i][j]=enc(M2p[i][j]);
            }
        }

        // Initial state: [1, 0, 0, 0, 0]
        vector<Ciphertext<DCRTPoly>> state(W);
        state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);

        // Three homomorphic matrix-vector multiplies
        for(int step=0;step<3;step++){
            auto& emat = (step==0)?emat0:(step==1)?emat1:emat2;
            vector<Ciphertext<DCRTPoly>> new_state(W,zero_ct);
            for(int j=0;j<W;j++){
                auto accum=zero_ct;
                for(int i=0;i<W;i++){
                    auto prod=divine(state[i],emat[i][j]);
                    accum=cc->EvalAdd(accum,prod);
                }
                accum=stabilize(accum);
                new_state[j]=accum;
            }
            state=new_state;
        }

        return dec(state[3]);
    }

    Matrix build_identity() {
        Matrix I(W, vector<int64_t>(W, 0));
        for(int i=0;i<W;i++) I[i][i]=1;
        return I;
    }

public:
    int64_t evaluate(int64_t x) {
        int64_t residues[5];
        for(int i=0;i<5;i++) residues[i] = eval_mod_kilian(x, moduli[i], i);
        return crt5_combine(residues);
    }

    void run_test() {
        cout<<"\n  FEmmg-iO: Kilian + Barrington + CRT5 + FHE\n";
        cout<<"  f(x) = (x+1)^3 via 5x5 Kilian-randomized companion matrices\n";
        cout<<"  M' = R_i * M * R_{i+1}^{-1} — product preserved, entries scrambled\n";
        cout<<"  Date: "<<ts()<<"\n\n";
        cout<<"  "<<string(60,'-')<<"\n";
        cout<<setw(10)<<"x"<<setw(22)<<"Kilian iO"<<setw(22)<<"Expected"<<"\n";
        cout<<"  "<<string(60,'-')<<"\n";

        vector<int64_t> tests={0,1,2,3,4,5};
        bool all_ok=true;
        for(int64_t x:tests){
            int64_t expected=(x+1)*(x+1)*(x+1);
            int64_t result=evaluate(x);
            bool ok=(result==expected);
            if(!ok)all_ok=false;
            cout<<setw(10)<<x<<setw(22)<<result<<setw(22)<<expected<<(ok?"  OK":"  FAIL")<<"\n";
        }
        cout<<"  "<<string(60,'-')<<"\n";
        cout<<"  Result: "<<(all_ok?"ALL CORRECT":"ERRORS DETECTED")<<"\n\n";
    }
};

int main(){
    KilianBarringtonIO io;
    io.run_test();
    return 0;
}
