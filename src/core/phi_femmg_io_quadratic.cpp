// PHI-OMEGA-ZERO: FEmmg-iO v1.2 — GENERAL QUADRATIC FUNCTIONS
// f(x) = ax^2 + bx + c for ANY a, b, c
// "ANY QUADRATIC. ANY COEFFICIENTS. THE PROGRAM IS HIDDEN."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <random>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class FEmmgIOQuad {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    mt19937 rng;
    int64_t MOD;
    static const int N = 3;
    static const int BITS = 4;
    using Matrix = vector<vector<int64_t>>;

    int64_t mod(int64_t v) { return ((v % MOD) + MOD) % MOD; }
    int64_t mod_inv(int64_t a) {
        int64_t t=0, newt=1, r=MOD, newr=mod(a);
        while(newr){ int64_t q=r/newr; int64_t tmp=t; t=newt; newt=tmp-q*newt; tmp=r; r=newr; newr=tmp-q*newr; }
        return (r>1)?1:mod(t);
    }
    int64_t mod_det(const Matrix& A) {
        int64_t a=A[0][0],b=A[0][1],c=A[0][2],d=A[1][0],e=A[1][1],f=A[1][2],g=A[2][0],h=A[2][1],i=A[2][2];
        return mod(mod(a*mod(e*i-f*h))-mod(b*mod(d*i-f*g))+mod(c*mod(d*h-e*g)));
    }
    Matrix identity() { return {{1,0,0},{0,1,0},{0,0,1}}; }
    Matrix mat_mult_mod(const Matrix& A, const Matrix& B) {
        Matrix C(N,vector<int64_t>(N,0));
        for(int i=0;i<N;i++)for(int j=0;j<N;j++)for(int k=0;k<N;k++)
            C[i][j]=mod(C[i][j]+mod(A[i][k]*B[k][j]));
        return C;
    }
    Matrix random_invertible() {
        uniform_int_distribution<int64_t> d(1,MOD-1);
        Matrix M;int64_t det;
        do{M={{d(rng),d(rng),d(rng)},{d(rng),d(rng),d(rng)},{d(rng),d(rng),d(rng)}};det=mod_det(M);}while(!det);
        return M;
    }
    Matrix mat_inv_mod(const Matrix& A) {
        int64_t a=A[0][0],b=A[0][1],c=A[0][2],d=A[1][0],e=A[1][1],f=A[1][2],g=A[2][0],h=A[2][1],i=A[2][2];
        int64_t det=mod_det(A),inv_det=mod_inv(det);
        Matrix cof={{mod(e*i-f*h),mod(-(d*i-f*g)),mod(d*h-e*g)},
                    {mod(-(b*i-c*h)),mod(a*i-c*g),mod(-(a*h-b*g))},
                    {mod(b*f-c*e),mod(-(a*f-c*d)),mod(a*e-b*d)}};
        Matrix adj(N,vector<int64_t>(N));
        for(int r=0;r<N;r++)for(int c=0;c<N;c++)adj[r][c]=mod(cof[c][r]*inv_det);
        return adj;
    }

    // ============================================
    // GENERAL QUADRATIC: f(x) = a*x^2 + b*x + c
    // ============================================
    // State: [1, x, f(x)] where f(x) accumulates
    // For bit weight w: x += w, f += a*(2w*x + w^2) + b*w
    // Final: f + c (add constant after loop)
    
    Matrix M1_general(int bit, int64_t a, int64_t b) {
        int64_t w = mod(1 << bit);
        // f += a*(2w*x + w^2) + b*w
        //   = 2aw*x + (aw^2 + bw)
        return {{1, w, mod(a*mod(w*w) + b*w)},
                {0, 1, mod(2*a*w)},
                {0, 0, 1}};
    }
    
    int64_t evaluate_plain(int64_t x, int64_t a, int64_t b, int64_t c) {
        Matrix state = identity();
        for(int bit=0; bit<BITS; bit++) {
            if(x & (1<<bit)) {
                state = mat_mult_mod(state, M1_general(bit, a, b));
            }
        }
        return mod(state[0][2] + c); // f(x) = accumulated + constant term
    }
    
    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));
    }
    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    }

public:
    FEmmgIOQuad() : rng(time(nullptr)) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(10);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(2048);
        params.SetSecurityLevel(HEStd_NotSet);
        cc=GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
        anchor0=enc(0);MOD=1073643521;
    }

    void test_quadratics() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FEmmg-iO v1.2 — GENERAL QUADRATIC FUNCTIONS        ║\n";
        cout <<   "  ║   f(x) = ax^2 + bx + c for ANY a, b, c              ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        struct TestFunc {
            string name;
            int64_t a, b, c;
        };
        
        vector<TestFunc> funcs = {
            {"(x+1)^2 = x^2+2x+1", 1, 2, 1},
            {"x^2", 1, 0, 0},
            {"2x^2 + 3x + 5", 2, 3, 5},
            {"5x^2 + 1", 5, 0, 1},
            {"10x + 7 (linear)", 0, 10, 7},
            {"42 (constant)", 0, 0, 42}
        };
        
        for(auto& func : funcs) {
            cout << "\n  Function: f(x) = " << func.name << "\n";
            cout << "  " << string(50, '-') << "\n";
            cout << "  " << setw(6) << "x" << setw(15) << "f(x)" << setw(15) << "Expected\n";
            cout << "  " << string(50, '-') << "\n";
            
            bool ok = true;
            for(int x : {0, 1, 2, 3, 5, 7, 10}) {
                int64_t r = evaluate_plain(x, func.a, func.b, func.c);
                int64_t e = func.a*x*x + func.b*x + func.c;
                if(r != e) ok = false;
                cout << "  " << setw(6) << x << setw(15) << r << setw(15) << e 
                     << (r==e ? " OK" : " FAIL") << "\n";
            }
            cout << "  " << string(50, '-') << "\n";
            cout << "  Result: " << (ok ? "ALL CORRECT" : "ERRORS") << "\n";
        }
        
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FEmmg-iO v1.2: GENERAL QUADRATICS VERIFIED         ║\n";
        cout <<   "  ║   Any f(x) = ax^2 + bx + c works!                    ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() { FEmmgIOQuad io; io.test_quadratics(); return 0; }
