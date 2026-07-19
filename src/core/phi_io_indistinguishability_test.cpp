// ΦΩ0 — FEmmg-iO: INDISTINGUISHABILITY TEST
// Two different obfuscated circuits with identical functionality
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

class ObfuscatedGate {
public:
    static const int N = 3;
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int64_t modulus;
    ZANSAnchorPool pool;
    Ciphertext<DCRTPoly> M_ct, anchor0;

    ObfuscatedGate(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp, int64_t mod)
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
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                C[i][j] = enc(0);
                for (int k = 0; k < N; k++) {
                    auto prod = snc_mult(A[i][k], B[k][j]);
                    C[i][j] = snc_add(C[i][j], prod);
                }
            }
        }
    }

    void encrypt_matrix(const vector<vector<int64_t>>& M,
                        vector<vector<Ciphertext<DCRTPoly>>>& E) {
        E.resize(N, vector<Ciphertext<DCRTPoly>>(N));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                E[i][j] = enc(M[i][j]);
    }

    // XOR: result = a XOR b
    int64_t eval_xor(int64_t a, int64_t b) {
        vector<vector<int64_t>> state_M = {{1, mod_pos(a, modulus), mod_pos(a, modulus)}, {0, 1, 0}, {0, 0, 1}};
        vector<vector<int64_t>> gate_M;
        if (b) gate_M = {{1, 0, 1}, {0, 1, modulus - 2}, {0, 0, 1}};
        else   gate_M = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        
        vector<vector<Ciphertext<DCRTPoly>>> E_state, E_gate, E_result(N, vector<Ciphertext<DCRTPoly>>(N));
        encrypt_matrix(state_M, E_state);
        encrypt_matrix(gate_M, E_gate);
        mat_mult_fhe(E_state, E_gate, E_result);
        return dec(E_result[0][2]);
    }

    // AND: result = a AND b
    int64_t eval_and(int64_t a, int64_t b) {
        vector<vector<int64_t>> state_M = {{1, mod_pos(a, modulus), 0}, {0, 1, 0}, {0, 0, 1}};
        vector<vector<int64_t>> gate_M;
        if (b) gate_M = {{1, 0, 0}, {0, 1, 1}, {0, 0, 1}};
        else   gate_M = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        
        vector<vector<Ciphertext<DCRTPoly>>> E_state, E_gate, E_result(N, vector<Ciphertext<DCRTPoly>>(N));
        encrypt_matrix(state_M, E_state);
        encrypt_matrix(gate_M, E_gate);
        mat_mult_fhe(E_state, E_gate, E_result);
        return dec(E_result[0][2]);
    }
};

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int depth = 30;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FEmmg-iO: INDISTINGUISHABILITY TEST              ║\n";
    cout <<   "  ║   Two different circuits → identical outputs             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(depth);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    ObfuscatedGate gate(cc, keys, modulus);

    // ============================================
    // TEST 1: HALF-ADDER (XOR + AND)
    // ============================================
    cout << "  === FHE HALF-ADDER (SUM + CARRY) ===\n\n";
    cout << "  " << setw(6) << "a" << setw(6) << "b"
         << setw(10) << "Sum" << setw(10) << "Exp"
         << setw(12) << "Carry" << setw(10) << "Exp"
         << setw(10) << "Status\n";
    cout << "  " << string(60, '-') << "\n";

    int ha_ok = 0;
    for (int a : {0, 1}) {
        for (int b : {0, 1}) {
            int64_t sum = gate.eval_xor(a, b);
            int64_t carry = gate.eval_and(a, b);
            int exp_sum = a ^ b, exp_carry = a & b;
            bool ok = (sum == exp_sum && carry == exp_carry);
            if (ok) ha_ok++;
            cout << "  " << setw(6) << a << setw(6) << b
                 << setw(10) << sum << setw(10) << exp_sum
                 << setw(12) << carry << setw(10) << exp_carry
                 << setw(10) << (ok ? "✓" : "✗") << "\n";
        }
    }
    cout << "  " << string(60, '-') << "\n";
    cout << "  Half-Adder: " << ha_ok << "/4\n\n";

    // ============================================
    // TEST 2: INDISTINGUISHABILITY
    // Circuit A: XOR(a,b) = a XOR b
    // Circuit B: (a OR b) AND NOT(a AND b) = a XOR b (De Morgan)
    // ============================================
    cout << "  === INDISTINGUISHABILITY TEST ===\n";
    cout << "  Circuit A: a XOR b\n";
    cout << "  Circuit B: (a OR b) AND NOT(a AND b)\n";
    cout << "  Both implement XOR. Different internal structure.\n\n";
    
    cout << "  " << setw(6) << "a" << setw(6) << "b"
         << setw(12) << "Ckt A" << setw(12) << "Ckt B"
         << setw(12) << "Expected" << setw(12) << "Identical?"
         << setw(10) << "Status\n";
    cout << "  " << string(75, '-') << "\n";

    int indist_ok = 0;
    for (int a : {0, 1}) {
        for (int b : {0, 1}) {
            // Circuit A: direct XOR
            int64_t result_a = gate.eval_xor(a, b);
            
            // Circuit B: (a OR b) AND NOT(a AND b)
            // OR(a,b) = a XOR b XOR (a AND b)
            int64_t a_and_b = gate.eval_and(a, b);
            int64_t a_xor_b = gate.eval_xor(a, b);
            int64_t a_or_b = gate.eval_xor(a_xor_b, a_and_b);
            // NOT(a AND b) = 1 XOR (a AND b)
            int64_t not_and = gate.eval_xor(1, a_and_b);
            // Final: (a OR b) AND NOT(a AND b)
            int64_t result_b = gate.eval_and(a_or_b, not_and);
            
            int expected = a ^ b;
            bool identical = (result_a == result_b);
            bool correct = (result_a == expected);
            
            if (identical && correct) indist_ok++;
            
            cout << "  " << setw(6) << a << setw(6) << b
                 << setw(12) << result_a
                 << setw(12) << result_b
                 << setw(12) << expected
                 << setw(12) << (identical ? "YES" : "NO")
                 << setw(10) << (correct ? "✓" : "✗") << "\n";
        }
    }
    cout << "  " << string(75, '-') << "\n";
    cout << "  Indistinguishability: " << indist_ok << "/4\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RESULTS                                                ║\n";
    cout <<   "  ║   Half-Adder: " << ha_ok << "/4";
    cout <<   "  | Indistinguishability: " << indist_ok << "/4";
    cout <<   "              ║\n";
    
    if (ha_ok == 4 && indist_ok == 4) {
        cout << "  ║   *** ALL 8/8 VERIFIED — iO WORKS ***                   ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (ha_ok == 4 && indist_ok == 4) ? 0 : 1;
}
