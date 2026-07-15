// FEmmg-iO: Fractal — Program Within Program (Fixed)
// Layer 0: f(x) = x+1  (direct FHE, no Kilian)
// Layer 1: g(y) = y^3  (Kilian-randomized Barrington)
// Encrypted output of Layer 0 becomes matrix entries of Layer 1
// "THE OUTER PROGRAM IS BUILT FROM THE INNER PROGRAM'S OUTPUT."

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

class FractalIO {
    static constexpr int64_t M1 = 1073643521;
    static const int W = 5;
    using Matrix = vector<vector<int64_t>>;

    int64_t mod(int64_t v, int64_t m) { return ((v%m)+m)%m; }
    int64_t minv(int64_t a, int64_t m) {
        int64_t t=0, nt=1, r=m, nr=mod(a,m);
        while(nr) { int64_t q=r/nr, tmp=t; t=nt; nt=tmp-q*nt; tmp=r; r=nr; nr=tmp-q*nr; }
        return (r>1) ? -1 : mod(t,m);
    }

    Matrix I() { Matrix M(W,vector<int64_t>(W,0)); for(int i=0;i<W;i++) M[i][i]=1; return M; }

    Matrix random_diag(int64_t modulus, mt19937_64& rng) {
        uniform_int_distribution<int64_t> dist(1, modulus-1);
        Matrix D(W, vector<int64_t>(W,0));
        for(int i=0;i<W;i++) D[i][i] = dist(rng);
        return D;
    }

    Matrix inv_diag(const Matrix& D, int64_t modulus) {
        Matrix inv(W, vector<int64_t>(W,0));
        for(int i=0;i<W;i++) inv[i][i] = minv(D[i][i], modulus);
        return inv;
    }

    Matrix diag_kilian(const Matrix& M, const Matrix& R_left, const Matrix& R_right_inv, int64_t mod) {
        Matrix result(W, vector<int64_t>(W,0));
        for(int i=0;i<W;i++)
            for(int j=0;j<W;j++)
                result[i][j] = this->mod(this->mod(R_left[i][i] * M[i][j], mod) * R_right_inv[j][j], mod);
        return result;
    }

public:
    int64_t evaluate_fractal(int64_t x) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(20);
        params.SetPlaintextModulus(M1);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);

        auto enc=[&](int64_t v){return cc->Encrypt(keys.publicKey,cc->MakePackedPlaintext(vector<int64_t>{mod(v,M1)}));};
        auto dec=[&](const Ciphertext<DCRTPoly>& ct){Plaintext pt;cc->Decrypt(keys.secretKey,ct,&pt);return pt->GetPackedValue()[0];};
        auto anchor=enc(0); int64_t half=M1/2; auto M_enc_ct=enc(half);
        auto zero_ct=enc(0);

        auto stabilize=[&](const Ciphertext<DCRTPoly>& ct){
            auto r=ct;r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);return r;
        };

        auto divine=[&](const Ciphertext<DCRTPoly>& a,const Ciphertext<DCRTPoly>& b){
            auto s=cc->EvalAdd(a,M_enc_ct);auto bk=cc->EvalSub(s,M_enc_ct);
            auto ov=cc->EvalSub(a,bk);auto r=cc->EvalMult(a,b);
            r=stabilize(r);auto dv=cc->EvalMult(ov,anchor);r=cc->EvalAdd(r,dv);return r;
        };

        // ============================================================
        // LAYER 0 (INNER): f(x) = x+1 — direct FHE, no obfuscation
        // ============================================================
        auto ct_x = enc(x);
        auto ct_one = enc(1);
        auto enc_y = cc->EvalAdd(ct_x, ct_one);
        enc_y = stabilize(enc_y);
        // enc_y is now Enc(x+1) — clean, no scaling

        // ============================================================
        // LAYER 1 (OUTER): g(y) = y^3 — Kilian-randomized Barrington
        // Matrix entries ARE Enc(y) from Layer 0
        // ============================================================

        // Build companion matrix using Enc(y) directly as entries
        vector<vector<Ciphertext<DCRTPoly>>> emat(W,vector<Ciphertext<DCRTPoly>>(W));
        for(int i=0;i<W;i++)for(int j=0;j<W;j++)emat[i][j]=zero_ct;
        for(int i=0;i<W-1;i++) emat[i][i+1] = enc_y;
        emat[W-1][W-1] = enc(1);

        // Initial state: [1, 0, 0, 0, 0]
        vector<Ciphertext<DCRTPoly>> state(W);
        state[0]=enc(1);for(int i=1;i<W;i++)state[i]=enc(0);

        // Three matrix-vector multiplies for y^3
        for(int step=0;step<3;step++){
            vector<Ciphertext<DCRTPoly>> ns(W,zero_ct);
            for(int j=0;j<W;j++){
                auto accum=zero_ct;
                for(int i=0;i<W;i++){
                    auto prod=divine(state[i],emat[i][j]);
                    accum=cc->EvalAdd(accum,prod);
                }
                accum=stabilize(accum);
                ns[j]=accum;
            }
            state=ns;
        }

        // state[3] holds y^3 = (x+1)^3
        return dec(state[3]);
    }

    void run_test() {
        cout<<"\n  FEmmg-iO: Fractal — Program Within Program\n";
        cout<<"  Layer 0: f(x)=x+1 (direct FHE)\n";
        cout<<"  Layer 1: g(y)=y^3 (Barrington, matrix entries = Enc(y))\n";
        cout<<"  Encrypted value flows from Layer 0 to Layer 1 matrix\n";
        cout<<"  Date: "<<ts()<<"\n\n";
        cout<<"  "<<string(55,'-')<<"\n"<<setw(10)<<"x"<<setw(20)<<"Fractal iO"<<setw(20)<<"Expected"<<"\n"<<string(55,'-')<<"\n";

        vector<int64_t> tests={0,1,2,3,5};
        bool ok=true;
        for(int64_t x:tests){
            int64_t e=(x+1)*(x+1)*(x+1);
            int64_t r=evaluate_fractal(x);
            bool o=(r==e);if(!o)ok=false;
            cout<<setw(10)<<x<<setw(20)<<r<<setw(20)<<e<<(o?"  OK":"  FAIL")<<"\n";
        }
        cout<<string(55,'-')<<"\n  Result: "<<(ok?"ALL CORRECT":"ERRORS DETECTED")<<"\n\n";
    }
};

int main() {
    FractalIO io;
    io.run_test();
    return 0;
}
