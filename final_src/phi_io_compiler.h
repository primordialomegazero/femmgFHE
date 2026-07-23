// ΦΩ0 — PHI-IO COMPILER
// General circuit-to-obfuscated-program compiler
// Input: Two functionally equivalent arithmetic circuits
// Output: φ-obfuscated program (phi/psi split)
#ifndef PHI_IO_COMPILER_H
#define PHI_IO_COMPILER_H

#include <openfhe.h>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <memory>

namespace phi_io {

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

using namespace lbcrypto;

// ============================================
// GATE TYPES
// ============================================
enum class GateType { INPUT, CONSTANT, ADD, MULT };

struct Gate {
    GateType type;
    int id;
    int input_idx;           // For INPUT gates
    double constant_val;     // For CONSTANT gates
    int left_gate;           // For ADD/MULT gates
    int right_gate;          // For ADD/MULT gates
};

// ============================================
// CIRCUIT
// ============================================
struct Circuit {
    std::vector<Gate> gates;
    int output_gate;
    
    // Evaluate on a specific input (plaintext)
    double evaluate(const std::vector<double>& inputs) const {
        std::vector<double> values(gates.size());
        for (const auto& g : gates) {
            switch (g.type) {
                case GateType::INPUT:
                    values[g.id] = inputs[g.input_idx];
                    break;
                case GateType::CONSTANT:
                    values[g.id] = g.constant_val;
                    break;
                case GateType::ADD:
                    values[g.id] = values[g.left_gate] + values[g.right_gate];
                    break;
                case GateType::MULT:
                    values[g.id] = values[g.left_gate] * values[g.right_gate];
                    break;
            }
        }
        return values[output_gate];
    }
};

// ============================================
// OBFUSCATED GATE
// ============================================
// Each gate in the obfuscated program stores
// TWO encodings: one for φ-reality, one for ψ-reality

struct ObfuscatedGate {
    Ciphertext<DCRTPoly> a;  // φ-coefficient ciphertext
    Ciphertext<DCRTPoly> b;  // ψ-coefficient ciphertext
    int id;
    bool is_phi_real;        // Which reality stores which circuit
};

// ============================================
// THE COMPILER
// ============================================
class PhiIOCompiler {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    uint32_t slots;
    Ciphertext<DCRTPoly> zero_ct;
    Ciphertext<DCRTPoly> one_ct;

    Ciphertext<DCRTPoly> encrypt_val(double v) {
        std::vector<double> vec(slots, 0.0);
        vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }

public:
    PhiIOCompiler(uint32_t ringDim = 8192) : slots(ringDim / 2) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(ringDim);
        params.SetScalingModSize(50);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(80);
        cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        zero_ct = encrypt_val(0.0);
        one_ct = encrypt_val(1.0);
    }

    // ============================================
    // ENCODE: Create φ/ψ dual encoding
    // ============================================
    // Given a value that should appear as v0 in φ-reality
    // and v1 in ψ-reality, create the (a,b) ciphertext pair
    //
    // We need: a + b·φ = v0 and a + b·ψ = v1
    // Solve: b = (v0 - v1) / (φ - ψ)
    //        a = v0 - b·φ

    struct DualEncoding {
        Ciphertext<DCRTPoly> a;
        Ciphertext<DCRTPoly> b;
    };

    DualEncoding encode_dual(double v0, double v1) {
        double b_val = (v0 - v1) / (PHI - PSI);
        double a_val = v0 - b_val * PHI;
        return {encrypt_val(a_val), encrypt_val(b_val)};
    }

    // ============================================
    // HOMOMORPHIC OPERATIONS ON DUAL ENCODINGS
    // ============================================

    DualEncoding add_dual(const DualEncoding& x, const DualEncoding& y) {
        return {
            cc->EvalAdd(x.a, y.a),
            cc->EvalAdd(x.b, y.b)
        };
    }

    DualEncoding mult_dual(const DualEncoding& x, const DualEncoding& y) {
        // (a₁ + b₁X)(a₂ + b₂X) = a₁a₂ + b₁b₂ + (a₁b₂ + a₂b₁ + b₁b₂)X
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {
            cc->EvalAdd(ac, bd),
            cc->EvalAdd(cc->EvalAdd(ad, bc), bd)
        };
    }

    // ============================================
    // COMPILE: Circuit + Assignment → Obfuscated Program
    // ============================================
    // assignment: for each gate, which circuit's value goes to φ?
    //   assignment[gate_id] = true → C₀ value in φ, C₁ in ψ
    //   assignment[gate_id] = false → C₁ value in φ, C₀ in ψ
    //
    // The assignment is THE SECRET. Without it, you cannot
    // distinguish which circuit is which.

    std::vector<DualEncoding> compile(
        const Circuit& C0,
        const Circuit& C1,
        const std::vector<double>& input,
        const std::vector<bool>& assignment
    ) {
        // Verify functional equivalence
        double out0 = C0.evaluate(input);
        double out1 = C1.evaluate(input);
        if (std::abs(out0 - out1) > 1e-10) {
            throw std::runtime_error("Circuits are NOT functionally equivalent!");
        }

        // Must have same number of gates (pad if necessary)
        size_t n = C0.gates.size();
        if (C1.gates.size() != n) {
            throw std::runtime_error("Circuits must have same number of gates for now");
        }

        std::vector<double> vals0(n), vals1(n);
        std::vector<DualEncoding> encoded(n);

        // Evaluate both circuits in plaintext AND encode simultaneously
        for (size_t i = 0; i < n; i++) {
            const Gate& g0 = C0.gates[i];
            const Gate& g1 = C1.gates[i];
            
            // Compute plaintext values for both circuits
            switch (g0.type) {
                case GateType::INPUT:
                    vals0[i] = input[g0.input_idx];
                    break;
                case GateType::CONSTANT:
                    vals0[i] = g0.constant_val;
                    break;
                case GateType::ADD:
                    vals0[i] = vals0[g0.left_gate] + vals0[g0.right_gate];
                    break;
                case GateType::MULT:
                    vals0[i] = vals0[g0.left_gate] * vals0[g0.right_gate];
                    break;
            }
            
            switch (g1.type) {
                case GateType::INPUT:
                    vals1[i] = input[g1.input_idx];
                    break;
                case GateType::CONSTANT:
                    vals1[i] = g1.constant_val;
                    break;
                case GateType::ADD:
                    vals1[i] = vals1[g1.left_gate] + vals1[g1.right_gate];
                    break;
                case GateType::MULT:
                    vals1[i] = vals1[g1.left_gate] * vals1[g1.right_gate];
                    break;
            }

            // Encode based on assignment
            if (assignment[i]) {
                // φ gets C0 value, ψ gets C1 value
                encoded[i] = encode_dual(vals0[i], vals1[i]);
            } else {
                // φ gets C1 value, ψ gets C0 value
                encoded[i] = encode_dual(vals1[i], vals0[i]);
            }
        }

        return encoded;
    }

    // ============================================
    // EXTRACT OUTPUT
    // ============================================
    double extract_output(const DualEncoding& output, bool use_phi) {
        Plaintext pa, pb;
        cc->Decrypt(keys.secretKey, output.a, &pa);
        cc->Decrypt(keys.secretKey, output.b, &pb);
        pa->SetLength(slots);
        pb->SetLength(slots);
        double a = pa->GetRealPackedValue()[0];
        double b = pb->GetRealPackedValue()[0];
        return use_phi ? (a + b * PHI) : (a + b * PSI);
    }

    CryptoContext<DCRTPoly>& getContext() { return cc; }
    KeyPair<DCRTPoly>& getKeys() { return keys; }
    uint32_t getSlots() const { return slots; }
};

} // namespace phi_io
#endif
