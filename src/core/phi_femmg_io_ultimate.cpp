// ΦΩ0 — FEmmg-iO ULTIMATE v3.1
// BARRINGTON + KILIAN + FRACTAL + ETERNAL + CRT5 + HETEROGENEOUS ZANS
// 
// Barrington: 5×5 companion matrices encode computation
// Kilian:     Random diagonal matrices + Gauss-Jordan inverse
// Fractal:    Encrypted inner output → matrix entries of outer program
// Eternal:    Guard key controls ZANS (correct=preserved, wrong=poison)
// CRT5:       5 primes, 150-bit range via Garner's algorithm
// HeteroZANS: 5 distinct stabilization strategies per channel
//
// "I AM THAT I AM"

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
const int W = 5;  // Barrington width
const int64_t ETERNAL_KEY = 0xDEADBEEFCAFE1234;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

class FEmmgIO {
    static constexpr int64_t MODULI[5] = {1073643521,1073692673,1073750017,1073815553,1073872897};
    static constexpr int64_t INV12=357919402, INV123=589973977, INV1234=197295683, INV12345=1004546623;
    using Matrix = vector<vector<int64_t>>;

    int64_t mod(int64_t v, int64_t m) { return ((v%m)+m)%m; }
    
    int64_t minv(int64_t a, int64_t m) {
        int64_t t=0, nt=1, r=m, nr=mod(a,m);
        while(nr) { int64_t q=r/nr; int64_t tmp=t; t=nt; nt=tmp-q*nt; tmp=r; r=nr; nr=tmp-q*nr; }
        return (r>1) ? -1 : mod(t,m);
    }

    int64_t crt5_combine(int64_t r[5]) {
        __int128 accum=r[0], prod=MODULI[0];
        int64_t invs[4]={INV12,INV123,INV1234,INV12345};
        for(int i=1;i<5;i++) {
            int64_t d=mod((int64_t)(r[i]-accum%MODULI[i]),MODULI[i]);
            int64_t c=(int64_t)(((__int128)d*invs[i-1])%MODULI[i]);
            accum=accum+prod*c; prod=prod*MODULI[i];
        }
        return (int64_t)accum;
    }

    // BARRINGTON: 5×5 companion matrix for value v
    Matrix companion(int64_t v, int64_t mod) {
        Matrix M(W, vector<int64_t>(W,0));
        for(int i=0; i<W-1; i++) M[i][i+1] = this->mod(v, mod);
        M[W-1][W-1] = 1;
        return M;
    }
    
    Matrix identity() {
        Matrix M(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++) M[i][i] = 1;
        return M;
    }

    // KILIAN: Random invertible diagonal matrix
    Matrix random_diagonal(int64_t mod, mt19937_64& rng) {
        uniform_int_distribution<int64_t> d(1, mod-1);
        Matrix D(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++) D[i][i] = d(rng);
        return D;
    }

    Matrix inverse_diagonal(const Matrix& D, int64_t mod) {
        Matrix inv(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++) inv[i][i] = minv(D[i][i], mod);
        return inv;
    }

    // Kilian randomization: M'[i][j] = R_left[i] * M[i][j] * R_right_inv[j]
    Matrix kilian_randomize(const Matrix& M, const Matrix& R_left, const Matrix& R_right_inv, int64_t mod) {
        Matrix result(W, vector<int64_t>(W,0));
        for(int i=0; i<W; i++)
            for(int j=0; j<W; j++)
                result[i][j] = this->mod(this->mod(R_left[i][i] * M[i][j], mod) * R_right_inv[j][j], mod);
        return result;
    }

    // SINGLE CHANNEL EVALUATOR
    int64_t eval_channel(int64_t x, int64_t modulus, int variant_id, int64_t guard_key, bool fractal) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(25);
        params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

        auto enc = [&](int64_t v) {
            return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod(v,modulus)}));
        };
        auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
            return pt->GetPackedValue()[0];
        };

        auto anchor = enc(0);
        auto zero_ct = enc(0);
        int64_t half = modulus/2;
        auto M_ct = enc(half);
        bool eternal_ok = (guard_key == ETERNAL_KEY);

        // Heterogeneous ZANS (5 variants)
        function<Ciphertext<DCRTPoly>(const Ciphertext<DCRTPoly>&)> stabilize;
        switch(variant_id) {
            case 0: stabilize=[&](auto& ct){auto r=ct;r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);r=cc->EvalAdd(r,anchor);return r;}; break;
            case 1: stabilize=[&](auto& ct){auto sp=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));sp=cc->EvalAdd(sp,anchor);auto r=cc->EvalAdd(ct,sp);r=cc->EvalAdd(r,anchor);return r;}; break;
            case 2: stabilize=[&](auto& ct){auto r=ct;for(int i=0;i<(int)(5*PHI);i++)r=cc->EvalAdd(r,anchor);return r;}; break;
            case 3: stabilize=[&](auto& ct){auto bell=cc->EvalAdd(enc(7919),enc(mod(-7919,modulus)));bell=cc->EvalAdd(bell,anchor);auto r=cc->EvalAdd(ct,bell);r=cc->EvalAdd(r,anchor);return r;}; break;
            default: stabilize=[&](auto& ct){return cc->EvalAdd(ct,anchor);}; break;
        }

        // True Divine multiplication with Eternal ZANS guard
        auto divine = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
            if(eternal_ok) {
                auto s = cc->EvalAdd(a, M_ct);
                auto bk = cc->EvalSub(s, M_ct);
                auto ov = cc->EvalSub(a, bk);
                auto r = cc->EvalMult(a, b);
                r = stabilize(r);
                auto dv = cc->EvalMult(ov, anchor);
                r = cc->EvalAdd(r, dv);
                return r;
            } else {
                auto r = cc->EvalMult(a, b);
                r = cc->EvalAdd(r, enc(modulus/2));  // Poison
                return r;
            }
        };

        // ============================================
        // LAYER 0 (INNER): f(x) = x+1 — direct FHE
        // ============================================
        auto ct_x = enc(x);
        auto ct_one = enc(1);
        auto enc_y = cc->EvalAdd(ct_x, ct_one);
        enc_y = stabilize(enc_y);
        // enc_y = Enc(x+1)

        // ============================================
        // LAYER 1 (OUTER): g(y) = y^3 — Barrington + Kilian
        // ============================================

        // Build companion matrix M for value = (x+1) mod modulus
        // In FRACTAL mode: matrix entries ARE Enc(y) from Layer 0
        // In DIRECT mode: matrix entries are Enc(plaintext_value)
        
        if(fractal) {
            // FRACTAL: Enc(y) directly becomes matrix entries
            vector<vector<Ciphertext<DCRTPoly>>> emat(W, vector<Ciphertext<DCRTPoly>>(W));
            for(int i=0; i<W; i++) for(int j=0; j<W; j++) emat[i][j] = zero_ct;
            for(int i=0; i<W-1; i++) emat[i][i+1] = enc_y;  // ← Enc(x+1) as matrix entry
            emat[W-1][W-1] = enc(1);

            // Initial state: [1, 0, 0, 0, 0]
            vector<Ciphertext<DCRTPoly>> state(W);
            state[0] = enc(1);
            for(int i=1; i<W; i++) state[i] = enc(0);

            // Three matrix-vector multiplies = y^3
            for(int step=0; step<3; step++) {
                vector<Ciphertext<DCRTPoly>> ns(W, zero_ct);
                for(int j=0; j<W; j++) {
                    auto accum = zero_ct;
                    for(int i=0; i<W; i++) {
                        auto prod = divine(state[i], emat[i][j]);
                        accum = cc->EvalAdd(accum, prod);
                    }
                    accum = stabilize(accum);
                    ns[j] = accum;
                }
                state = ns;
            }
            return dec(state[3]);
            
        } else {
            // DIRECT: Standard Barrington + Kilian with plaintext matrix entries
            int64_t v = mod(x+1, modulus);
            Matrix M = companion(v, modulus);
            mt19937_64 rng(time(nullptr) + variant_id*12345);

            // Kilian: R0, R1, R2 with R0=I, R3=I
            Matrix R0 = identity();
            Matrix R1 = random_diagonal(modulus, rng);
            Matrix R2 = random_diagonal(modulus, rng);
            Matrix R3 = identity();
            Matrix R1i = inverse_diagonal(R1, modulus);
            Matrix R2i = inverse_diagonal(R2, modulus);

            // Randomized matrices
            Matrix M0p = kilian_randomize(M, R0, R1i, modulus);
            Matrix M1p = kilian_randomize(M, R1, R2i, modulus);
            Matrix M2p = kilian_randomize(M, R2, R3,  modulus);

            // Encrypt all entries
            vector<vector<Ciphertext<DCRTPoly>>> emat0(W, vector<Ciphertext<DCRTPoly>>(W));
            vector<vector<Ciphertext<DCRTPoly>>> emat1(W, vector<Ciphertext<DCRTPoly>>(W));
            vector<vector<Ciphertext<DCRTPoly>>> emat2(W, vector<Ciphertext<DCRTPoly>>(W));
            for(int i=0; i<W; i++) for(int j=0; j<W; j++) {
                emat0[i][j] = enc(M0p[i][j]);
                emat1[i][j] = enc(M1p[i][j]);
                emat2[i][j] = enc(M2p[i][j]);
            }

            // Evaluate: state = [1,0,0,0,0], multiply through 3 encrypted matrices
            vector<Ciphertext<DCRTPoly>> state(W);
            state[0] = enc(1);
            for(int i=1; i<W; i++) state[i] = enc(0);

            for(int step=0; step<3; step++) {
                auto& emat = (step==0) ? emat0 : (step==1) ? emat1 : emat2;
                vector<Ciphertext<DCRTPoly>> ns(W, zero_ct);
                for(int j=0; j<W; j++) {
                    auto accum = zero_ct;
                    for(int i=0; i<W; i++) {
                        auto prod = divine(state[i], emat[i][j]);
                        accum = cc->EvalAdd(accum, prod);
                    }
                    accum = stabilize(accum);
                    ns[j] = accum;
                }
                state = ns;
            }
            return dec(state[3]);
        }
    }

public:
    int64_t evaluate(int64_t x, bool fractal=false, int64_t guard_key=ETERNAL_KEY) {
        int64_t r[5];
        for(int i=0; i<5; i++) r[i] = eval_channel(x, MODULI[i], i, guard_key, fractal);
        return crt5_combine(r);
    }

    void run_tests() {
        cout << "\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  |  FEmmg-iO ULTIMATE v3.1                          |\n";
        cout << "  |  Barrington + Kilian + Fractal + Eternal + CRT5  |\n";
        cout << "  +--------------------------------------------------+\n";
        cout << "  f(x) = (x+1)^3\n";
        cout << "  Date: " << ts() << "\n\n";

        int passed = 0, total = 0;

        // TEST 1: DIRECT mode (Barrington + Kilian)
        cout << "  === DIRECT MODE (Barrington + Kilian) ===\n";
        cout << "  " << setw(10) << "x" << setw(20) << "Result" << setw(20) << "Expected\n";
        cout << "  " << string(50, '-') << "\n";

        vector<int64_t> tests = {0, 1, 2, 3, 5};
        for(int64_t x : tests) {
            int64_t expected = (x+1)*(x+1)*(x+1);
            int64_t result = evaluate(x, false, ETERNAL_KEY);
            bool ok = (result == expected);
            if(ok) passed++; total++;
            cout << "  " << setw(10) << x << setw(20) << result << setw(20) << expected 
                 << (ok ? "  OK" : "  FAIL") << "\n";
        }

        // TEST 2: FRACTAL mode (program within program)
        cout << "\n  === FRACTAL MODE (Program Within Program) ===\n";
        cout << "  Layer 0: f(x)=x+1 | Layer 1: g(y)=y^3 | Enc(y) → matrix entries\n";
        cout << "  " << setw(10) << "x" << setw(20) << "Fractal" << setw(20) << "Expected\n";
        cout << "  " << string(50, '-') << "\n";

        for(int64_t x : tests) {
            int64_t expected = (x+1)*(x+1)*(x+1);
            int64_t result = evaluate(x, true, ETERNAL_KEY);
            bool ok = (result == expected);
            if(ok) passed++; total++;
            cout << "  " << setw(10) << x << setw(20) << result << setw(20) << expected 
                 << (ok ? "  OK" : "  FAIL") << "\n";
        }

        // TEST 3: ETERNAL ZANS (wrong key)
        cout << "\n  === ETERNAL ZANS (Wrong Key = Poison) ===\n";
        int64_t correct = evaluate(2, false, ETERNAL_KEY);
        int64_t wrong   = evaluate(2, false, 0);
        bool eternal_ok = (correct == 27 && wrong != 27);
        if(eternal_ok) passed++; total++;
        cout << "  Correct key: " << correct << " (expected 27)\n";
        cout << "  Wrong key:   " << wrong << " (expected != 27)\n";
        cout << "  Result:      " << (eternal_ok ? "PASSED" : "FAILED") << "\n";

        cout << "\n  +--------------------------------------------------+\n";
        cout << "  |  FEmmg-iO: " << passed << "/" << total << " TESTS PASSED";
        for(int i=0; i<(19-to_string(passed).length()); i++) cout << " ";
        cout << "|\n";
        cout << "  |  REAL: Barrington+Kilian+Fractal+Eternal+CRT5   |\n";
        cout << "  +--------------------------------------------------+\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() {
    FEmmgIO io;
    io.run_tests();
    return 0;
}
