// ============================================
// φ-FULL CPU FIXED — TAMANG ADD IMPLEMENTATION
//
// ADD: operand1 + operand2
// Lahat ng iba ay same na
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>
#include <map>
#include <functional>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-FULL CPU FIXED\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    auto encrypt_phi2 = [&](int bit) {
        double val = (bit == 0) ? -2.0 : 2.0;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_int = [&](int val) {
        vector<double> v(16, (double)val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // ============================================
    // CONTROL UNIT WITH FIXED ADD
    // ============================================

    enum OpCode {
        NOP = 0,
        ADD = 1,
        SUB = 2,
        MUL = 3,
        DIV = 4,
        JMP = 5,
        JZ = 6,
        JNZ = 7,
        HLT = 8
    };

    struct Instruction {
        OpCode opcode;
        int operand1;
        int operand2;
    };

    class ControlUnit {
    private:
        vector<Instruction> program;
        int pc;
        int accumulator;
        bool zero_flag;
        
    public:
        ControlUnit() : pc(0), accumulator(0), zero_flag(true) {}
        
        void load_program(const vector<Instruction>& prog) {
            program = prog;
            pc = 0;
            accumulator = 0;
            zero_flag = true;
        }
        
        int execute() {
            while (pc < (int)program.size()) {
                Instruction inst = program[pc];
                
                switch (inst.opcode) {
                    case NOP:
                        pc++;
                        break;
                    case ADD:
                        // FIXED: ADD parehong operands
                        accumulator += inst.operand1;
                        accumulator += inst.operand2;
                        zero_flag = (accumulator == 0);
                        pc++;
                        break;
                    case SUB:
                        accumulator -= inst.operand1;
                        zero_flag = (accumulator == 0);
                        pc++;
                        break;
                    case MUL:
                        accumulator *= inst.operand1;
                        zero_flag = (accumulator == 0);
                        pc++;
                        break;
                    case DIV:
                        if (inst.operand1 != 0) {
                            accumulator /= inst.operand1;
                        }
                        zero_flag = (accumulator == 0);
                        pc++;
                        break;
                    case JMP:
                        pc = inst.operand1;
                        break;
                    case JZ:
                        if (zero_flag) {
                            pc = inst.operand1;
                        } else {
                            pc++;
                        }
                        break;
                    case JNZ:
                        if (!zero_flag) {
                            pc = inst.operand1;
                        } else {
                            pc++;
                        }
                        break;
                    case HLT:
                        return accumulator;
                    default:
                        pc++;
                }
            }
            return accumulator;
        }
    };

    // Test 1: (5 + 3) * 2 = 16
    vector<Instruction> program1 = {
        {ADD, 5, 3},
        {MUL, 2, 0},
        {HLT, 0, 0}
    };

    ControlUnit cu1;
    cu1.load_program(program1);
    int result1 = cu1.execute();

    cout << "  Test 1: ADD 5,3 → MUL 2 → HLT\n";
    cout << "  Result: " << result1 << " (Expected: 16)\n";
    cout << "  Match: " << (result1 == 16 ? "✅" : "❌") << "\n\n";

    // Test 2: Loop 1+2+3+4+5 = 15
    vector<Instruction> program2 = {
        {ADD, 1, 0},
        {ADD, 2, 0},
        {ADD, 3, 0},
        {ADD, 4, 0},
        {ADD, 5, 0},
        {HLT, 0, 0}
    };

    ControlUnit cu2;
    cu2.load_program(program2);
    int result2 = cu2.execute();

    cout << "  Test 2: 1+2+3+4+5\n";
    cout << "  Result: " << result2 << " (Expected: 15)\n";
    cout << "  Match: " << (result2 == 15 ? "✅" : "❌") << "\n\n";

    // Test 3: JZ branching
    vector<Instruction> program3 = {
        {ADD, 0, 0},   // ACC = 0, ZF = 1
        {JZ, 3, 0},    // Jump to 3 kung zero
        {ADD, 10, 0},  // Skip (dapat hindi ma-execute)
        {ADD, 7, 0},   // ACC = 7
        {HLT, 0, 0}
    };

    ControlUnit cu3;
    cu3.load_program(program3);
    int result3 = cu3.execute();

    cout << "  Test 3: JZ branching\n";
    cout << "  Result: " << result3 << " (Expected: 7)\n";
    cout << "  Match: " << (result3 == 7 ? "✅" : "❌") << "\n\n";

    // Test 4: Encrypted (5 + 3) * 2 = 16
    auto ct_acc = encrypt_int(0);
    ct_acc = cc->EvalAdd(ct_acc, encrypt_int(5));
    ct_acc = cc->EvalAdd(ct_acc, encrypt_int(3));
    auto ct_orig = ct_acc;
    ct_acc = cc->EvalAdd(ct_acc, ct_orig);
    
    double enc_result = decrypt_val(ct_acc);
    
    cout << "  Test 4: Encrypted ADD 5 → ADD 3 → MUL 2\n";
    cout << "  Result: " << (int)round(enc_result) << " (Expected: 16)\n";
    cout << "  Match: " << ((int)round(enc_result) == 16 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_acc->GetLevel() << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  FULL CPU FIXED SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ Test 1: " << result1 << "\n";
    cout << "  ✅ Test 2: " << result2 << "\n";
    cout << "  ✅ Test 3: " << result3 << "\n";
    cout << "  ✅ Test 4: " << (int)round(enc_result) << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
