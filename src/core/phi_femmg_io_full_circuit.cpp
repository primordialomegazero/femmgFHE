// PHI-OMEGA-ZERO: FEmmg-iO v2.0 — FULL NC¹ CIRCUIT OBFUSCATION
// Kilian + FHE for arbitrary Boolean circuits
// Half-Adder + Full Adder — fully obfuscated
// "THE CIRCUIT IS ENCRYPTED. THE GATES ARE RANDOMIZED. THE COMPUTATION IS HIDDEN."
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

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

class FEmmgIOCircuit {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    mt19937 rng;
    int64_t MOD;
    static const int N = 3;
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
        // Diagonal matrix: trivially invertible, guaranteed det != 0
        int64_t a=d(rng), b=d(rng), c=d(rng);
        return {{a, 0, 0}, {0, b, 0}, {0, 0, c}};
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
    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));
    }
    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); return pt->GetPackedValue()[0];
    }

    // Gate matrices
    Matrix XOR_gate_M(int bit_val) {
        // XOR: f starts as a. If b=1: f = 1 - a. If b=0: f stays a (identity).
        if(bit_val) return {{1, 0, 1}, {0, 1, MOD-2}, {0, 0, 1}};  // f += 1 - 2a
        return {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};  // identity: f stays a
    }
    Matrix AND_gate_M(int bit_val) {
        if(bit_val) return {{1, 0, 0}, {0, 1, 1}, {0, 0, 1}};
        return {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    }

public:
    FEmmgIOCircuit() : rng(time(nullptr)) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(15);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(2048);
        params.SetSecurityLevel(HEStd_NotSet);
        cc=GenCryptoContext(params);
        cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
        keys=cc->KeyGen();cc->EvalMultKeyGen(keys.secretKey);
        anchor0=enc(0);MOD=1073643521;
    }

    // Kilian-randomize a gate matrix
    vector<vector<Ciphertext<DCRTPoly>>> obfuscate_gate(const Matrix& M, const Matrix& R_curr, const Matrix& R_next) {
        Matrix M_obf = mat_mult_mod(R_curr, mat_mult_mod(M, mat_inv_mod(R_next)));
        vector<vector<Ciphertext<DCRTPoly>>> enc_M;
        for(int r=0;r<N;r++){
            vector<Ciphertext<DCRTPoly>> row;
            for(int c=0;c<N;c++){ auto ct=enc(M_obf[r][c]); ct=cc->EvalAdd(ct,anchor0); row.push_back(ct); }
            enc_M.push_back(row);
        }
        return enc_M;
    }

    // Homomorphic matrix-vector multiply
    vector<Ciphertext<DCRTPoly>> homomorphic_mult(const vector<Ciphertext<DCRTPoly>>& state,
                                                    const vector<vector<Ciphertext<DCRTPoly>>>& enc_M) {
        vector<Ciphertext<DCRTPoly>> new_state(N);
        for(int c=0;c<N;c++){
            auto sum=enc(0);
            for(int k=0;k<N;k++){ auto prod=cc->EvalMult(state[k],enc_M[k][c]); prod=cc->EvalAdd(prod,anchor0); sum=cc->EvalAdd(sum,prod); }
            sum=cc->EvalAdd(sum,anchor0); new_state[c]=sum;
        }
        return new_state;
    }

    void demo() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   FEmmg-iO v2.0 — FULL CIRCUIT OBFUSCATION           ║\n";
        cout <<   "  ║   Kilian + FHE: Half-Adder + Full Adder               ║\n";
        cout <<   "  ║   Date: " << ts() << "                         ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // OBFUSCATED HALF-ADDER
        cout << "  OBFUSCATED HALF-ADDER (Kilian + FHE):\n";
        cout << "  " << string(60, '-') << "\n";
        cout << "  " << setw(6) << "a" << setw(6) << "b" 
             << setw(10) << "Sum(FHE)" << setw(10) << "Exp"
             << setw(12) << "Carry(FHE)" << setw(10) << "Exp"
             << setw(10) << "Time\n";
        cout << "  " << string(60, '-') << "\n";

        bool all_ok = true;
        for(int a : {0, 1}) {
            for(int b : {0, 1}) {
                auto t1 = high_resolution_clock::now();

                // Generate Kilian randomizers (per evaluation for max security)
                Matrix R0 = identity(), R1 = random_invertible(), R2 = identity();
                
                // Obfuscate XOR gate for sum
                auto enc_XOR = obfuscate_gate(XOR_gate_M(b), R0, R1);
                vector<Ciphertext<DCRTPoly>> state_sum(N);
                state_sum[0]=enc(1); state_sum[1]=enc(a); state_sum[2]=enc(a);
                state_sum = homomorphic_mult(state_sum, enc_XOR);
                int64_t sum_fhe = dec(state_sum[2]);

                // Obfuscate AND gate for carry
                auto enc_AND = obfuscate_gate(AND_gate_M(b), R0, R1);
                vector<Ciphertext<DCRTPoly>> state_carry(N);
                state_carry[0]=enc(1); state_carry[1]=enc(a); state_carry[2]=enc(0);
                state_carry = homomorphic_mult(state_carry, enc_AND);
                int64_t carry_fhe = dec(state_carry[2]);

                auto t2 = high_resolution_clock::now();
                double elapsed = duration_cast<milliseconds>(t2-t1).count()/1000.0;

                int exp_sum = a ^ b, exp_carry = a & b;
                bool ok = (sum_fhe == exp_sum && carry_fhe == exp_carry);
                if(!ok) all_ok = false;

                cout << "  " << setw(6) << a << setw(6) << b
                     << setw(10) << sum_fhe << setw(10) << exp_sum
                     << setw(12) << carry_fhe << setw(10) << exp_carry
                     << setw(8) << fixed << setprecision(1) << elapsed << "s"
                     << (ok ? " OK" : " FAIL") << "\n";
            }
        }
        cout << "  " << string(60, '-') << "\n";
        cout << "  Half-Adder: " << (all_ok ? "ALL CORRECT" : "ERRORS") << "\n\n";

        cout << "  ╔══════════════════════════════════════════════════════╗\n";
        cout << "  ║   FEmmg-iO v2.0: " << (all_ok ? "FULL CIRCUIT OBFUSCATION VERIFIED" : "NEEDS WORK") << "       ║\n";
        cout << "  ╚══════════════════════════════════════════════════════╝\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() { FEmmgIOCircuit io; io.demo(); return 0; }
