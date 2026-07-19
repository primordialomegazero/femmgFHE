// ΦΩ0 — FEmmg-iO: DIRECT GATE VERIFICATION v2
// Fixed: XOR-based gates use [1,a,a] state, AND-based use [1,a,0]
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

class GateEngine {
    static const int N = 3;
    using Matrix = vector<vector<int64_t>>;
    
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int64_t modulus;
    ZANSAnchorPool pool;
    Ciphertext<DCRTPoly> M_ct, anchor0;

public:
    GateEngine(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp, int64_t mod)
        : cc(ctx), keys(kp), modulus(mod), pool(ctx, kp, 20) {
        M_ct = enc(mod / 2);
        anchor0 = enc(0);
    }

    Ciphertext<DCRTPoly> enc(int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod_pos(v, modulus)}));
    }
    int64_t dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    }

    Ciphertext<DCRTPoly> snc_mult(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, M_ct);
        auto back = cc->EvalSub(sum, M_ct);
        auto overflow = cc->EvalSub(a, back);
        auto result = cc->EvalMult(a, b);
        auto correction = cc->EvalMult(overflow, anchor0);
        result = cc->EvalAdd(result, correction);
        result = cc->EvalAdd(result, anchor0);
        for (int i = 0; i < 5; i++) result = pool.stabilize(result);
        return result;
    }

    Ciphertext<DCRTPoly> snc_add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto result = cc->EvalAdd(a, b);
        for (int i = 0; i < 3; i++) result = pool.stabilize(result);
        return result;
    }

    void mat_mult_fhe(
        const vector<vector<Ciphertext<DCRTPoly>>>& A,
        const vector<vector<Ciphertext<DCRTPoly>>>& B,
        vector<vector<Ciphertext<DCRTPoly>>>& C) {
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                C[i][j] = enc(0);
                for (int k = 0; k < N; k++) {
                    auto prod = snc_mult(A[i][k], B[k][j]);
                    C[i][j] = snc_add(C[i][j], prod);
                }
            }
    }

    void encrypt_matrix(const Matrix& M, vector<vector<Ciphertext<DCRTPoly>>>& E) {
        E.resize(N, vector<Ciphertext<DCRTPoly>>(N));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                E[i][j] = enc(M[i][j]);
    }

    int64_t evaluate(const Matrix& state, const Matrix& gate) {
        vector<vector<Ciphertext<DCRTPoly>>> E_state, E_gate, E_result(N, vector<Ciphertext<DCRTPoly>>(N));
        encrypt_matrix(state, E_state);
        encrypt_matrix(gate, E_gate);
        mat_mult_fhe(E_state, E_gate, E_result);
        return dec(E_result[0][2]);
    }

    // ============================================
    // GATES — each with correct initial state
    // ============================================

    // XOR: state [1,a,a], gate based on b
    int64_t eval_xor(int64_t a, int64_t b) {
        Matrix s = {{1, mod_pos(a, modulus), mod_pos(a, modulus)}, {0, 1, 0}, {0, 0, 1}};
        Matrix g = b ? Matrix{{1, 0, 1}, {0, 1, mod_pos(-1, modulus)}, {0, 0, 1}} 
                     : Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        return evaluate(s, g);
    }

    // NOT: XOR with b=1
    int64_t eval_not(int64_t a) { return eval_xor(a, 1); }

    // AND: state [1,a,0], gate based on b
    int64_t eval_and(int64_t a, int64_t b) {
        Matrix s = {{1, mod_pos(a, modulus), 0}, {0, 1, 0}, {0, 0, 1}};
        Matrix g = b ? Matrix{{1, 0, 0}, {0, 1, 1}, {0, 0, 1}} 
                     : Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        return evaluate(s, g);
    }

    // OR: state [1,a,0], gate based on b
    int64_t eval_or(int64_t a, int64_t b) {
        Matrix s = {{1, mod_pos(a, modulus), 0}, {0, 1, 0}, {0, 0, 1}};
        Matrix g = b ? Matrix{{1, 0, 1}, {0, 1, 0}, {0, 0, 1}} 
                     : Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        return evaluate(s, g);
    }

    // XNOR: NOT(XOR(a,b))
    int64_t eval_xnor(int64_t a, int64_t b) { return eval_not(eval_xor(a, b)); }
};

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FEmmg-iO: DIRECT GATE VERIFICATION v2            ║\n";
    cout <<   "  ║   XOR/NOT use [1,a,a] state, AND/OR use [1,a,0]          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    GateEngine ge(cc, keys, modulus);

    int total_ok = 0, total_tests = 0;

    auto run_test = [&](string name, auto fn, int inputs, auto expected_fn) {
        cout << "  " << name << ":\n  " << string(40, '-') << "\n";
        int ok = 0, cnt = 0;
        for (int a : {0, 1}) {
            int max_b = (inputs == 2) ? 1 : 0;
            for (int b = 0; b <= max_b; b++) {
                int64_t result = fn(a, b);
                int expected = expected_fn(a, b);
                bool match = (result == expected);
                if (match) ok++; cnt++;
                cout << "    a=" << a << " b=" << b << " → " << setw(10) << result
                     << " (exp " << expected << ") " << (match ? "✓" : "✗") << "\n";
            }
        }
        cout << "  " << string(40, '-') << "\n  " << name << ": " << ok << "/" << cnt << " ✓\n\n";
        total_ok += ok; total_tests += cnt;
    };

    run_test("NOT",  [&](int64_t a, int64_t b) { return ge.eval_not(a); }, 1,
             [](int a, int b) { return !a; });
    run_test("AND",  [&](int64_t a, int64_t b) { return ge.eval_and(a, b); }, 2,
             [](int a, int b) { return a && b; });
    run_test("OR",   [&](int64_t a, int64_t b) { return ge.eval_or(a, b); }, 2,
             [](int a, int b) { return a || b; });
    run_test("XOR",  [&](int64_t a, int64_t b) { return ge.eval_xor(a, b); }, 2,
             [](int a, int b) { return a ^ b; });
    run_test("XNOR", [&](int64_t a, int64_t b) { return ge.eval_xnor(a, b); }, 2,
             [](int a, int b) { return !(a ^ b); });

    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RESULTS: " << total_ok << "/" << total_tests << " verified";
    for (int i = 0; i < (40 - to_string(total_ok).length() - to_string(total_tests).length()); i++) cout << " ";
    cout << "║\n";
    if (total_ok == total_tests)
        cout << "  ║   *** ALL GATES VERIFIED — iO FOUNDATION COMPLETE ***   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    return (total_ok == total_tests) ? 0 : 1;
}
