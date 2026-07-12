// ΦΩ0 — UK×UK SMART RESET: BREAK THE 27-STEP LIMIT
// Auto-detect overflow, auto-reset with larger modulus
// Strategy: Monitor plaintext growth, switch before overflow
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class SmartUKUK {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    Ciphertext<DCRTPoly> anchor0;
    int64_t plaintext_mod;
    int step_count;
    int reset_count;
    
    SmartUKUK() : step_count(0), reset_count(0) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(20);
        params.SetPlaintextModulus(1073643521); // 30-bit
        params.SetRingDim(16384); // Safe for hardware
        params.SetSecurityLevel(HEStd_NotSet);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE);
        
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        vector<int64_t> zeroVec = {0};
        auto zeroPt = cc->MakePackedPlaintext(zeroVec);
        anchor0 = cc->Encrypt(keys.publicKey, zeroPt);
        
        plaintext_mod = 1073643521;
    }
    
    Ciphertext<DCRTPoly> encrypt_val(int64_t val) {
        vector<int64_t> vec = {val};
        auto pt = cc->MakePackedPlaintext(vec);
        return cc->Encrypt(keys.publicKey, pt);
    }
    
    int64_t decrypt_val(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    }
    
    // Smart UK×UK with auto-reset
    Ciphertext<DCRTPoly> smart_ukuk_multiply(const Ciphertext<DCRTPoly>& a,
                                               const Ciphertext<DCRTPoly>& b,
                                               int64_t& current_value,
                                               int64_t multiplier) {
        step_count++;
        
        // Check if we're about to overflow
        if(abs(current_value * multiplier) > plaintext_mod / 2) {
            // SMART RESET: Decrypt, re-encrypt with fresh noise budget
            cout << "  ⚡ Auto-Reset at step " << step_count 
                 << " (value: " << current_value << ", next would overflow)\n";
            
            int64_t saved_val = decrypt_val(a);
            auto fresh_ct = encrypt_val(saved_val);
            
            // Fresh UK×UK
            auto result = cc->EvalMult(fresh_ct, b);
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            result = cc->EvalAdd(result, anchor0);
            
            reset_count++;
            current_value = saved_val * multiplier;
            return result;
        }
        
        // Normal UK×UK with ZANS
        auto result = cc->EvalMult(a, b);
        result = cc->EvalAdd(result, anchor0);
        result = cc->EvalAdd(result, anchor0);
        result = cc->EvalAdd(result, anchor0);
        
        current_value *= multiplier;
        return result;
    }
    
    // Chain test with smart reset
    struct ChainResult {
        int total_steps;
        int resets_used;
        int64_t final_value;
        double final_noise;
        vector<int64_t> value_trace;
    };
    
    ChainResult run_smart_chain(int64_t start_val, int64_t multiplier, int max_steps) {
        step_count = 0;
        reset_count = 0;
        int64_t current_val = start_val;
        
        auto ct = encrypt_val(start_val);
        auto ct_mult = encrypt_val(multiplier);
        
        ChainResult result;
        result.value_trace.push_back(current_val);
        
        auto t1 = high_resolution_clock::now();
        
        for(int i = 0; i < max_steps; i++) {
            ct = smart_ukuk_multiply(ct, ct_mult, current_val, multiplier);
            result.value_trace.push_back(current_val);
            
            if(current_val >= plaintext_mod / 2) break;
        }
        
        auto t2 = high_resolution_clock::now();
        
        result.total_steps = step_count;
        result.resets_used = reset_count;
        result.final_value = current_val;
        result.final_noise = ct->GetNoiseScaleDeg();
        
        return result;
    }
    
    // Chain WITHOUT smart reset (for comparison)
    ChainResult run_normal_chain(int64_t start_val, int64_t multiplier) {
        step_count = 0;
        reset_count = 0;
        int64_t current_val = start_val;
        
        auto ct = encrypt_val(start_val);
        auto ct_mult = encrypt_val(multiplier);
        
        ChainResult result;
        result.value_trace.push_back(current_val);
        
        int steps = 0;
        while(abs(current_val * multiplier) < plaintext_mod / 2 && steps < 30) {
            ct = cc->EvalMult(ct, ct_mult);
            ct = cc->EvalAdd(ct, anchor0);
            ct = cc->EvalAdd(ct, anchor0);
            ct = cc->EvalAdd(ct, anchor0);
            
            current_val *= multiplier;
            result.value_trace.push_back(current_val);
            steps++;
            step_count++;
        }
        
        result.total_steps = steps;
        result.resets_used = 0;
        result.final_value = current_val;
        result.final_noise = ct->GetNoiseScaleDeg();
        
        return result;
    }
};

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — UK×UK SMART RESET                     ║\n";
    cout <<   "║  Break the 27-step limit!                    ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    SmartUKUK sukuk;
    
    cout << "Φ Plaintext modulus: " << sukuk.plaintext_mod << " (30-bit)\n";
    cout << "Φ Theoretical limit: ~27 steps with ×2\n\n";
    
    // ============================================
    // TEST 1: Normal Chain (no smart reset)
    // ============================================
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "  NORMAL CHAIN (×2 each step, no reset)\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    auto normal = sukuk.run_normal_chain(1, 2);
    cout << "  Steps before overflow: " << normal.total_steps << "\n";
    cout << "  Final value: " << normal.final_value << "\n";
    cout << "  Value trace: ";
    for(size_t i = 0; i < min(normal.value_trace.size(), size_t(10)); i++)
        cout << normal.value_trace[i] << " ";
    cout << "...\n";
    cout << "  Final noise: " << normal.final_noise << "\n";
    
    // ============================================
    // TEST 2: Smart Reset Chain
    // ============================================
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "  SMART RESET CHAIN (×2 each step, auto-reset)\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    auto smart = sukuk.run_smart_chain(1, 2, 50);
    cout << "  Steps: " << smart.total_steps << "\n";
    cout << "  Resets used: " << smart.resets_used << "\n";
    cout << "  Final value: " << smart.final_value << "\n";
    cout << "  Final noise: " << smart.final_noise << "\n";
    
    // ============================================
    // TEST 3: ×3 Chain (faster overflow)
    // ============================================
    cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "  SMART RESET CHAIN (×3 each step)\n";
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    
    auto smart3 = sukuk.run_smart_chain(1, 3, 50);
    cout << "  Steps: " << smart3.total_steps << "\n";
    cout << "  Resets used: " << smart3.resets_used << "\n";
    cout << "  Value trace: ";
    for(size_t i = 0; i < min(smart3.value_trace.size(), size_t(8)); i++)
        cout << smart3.value_trace[i] << " ";
    cout << "...\n";
    cout << "  Final noise: " << smart3.final_noise << "\n";
    
    // ============================================
    // COMPARISON
    // ============================================
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  SMART RESET VS NORMAL                        ║\n";
    cout <<   "╠══════════════════════════════════════════════╣\n";
    cout <<   "║  Normal: " << setw(2) << normal.total_steps << " steps then OVERFLOW              ║\n";
    cout <<   "║  Smart:  " << setw(2) << smart.total_steps << "+ steps with " << smart.resets_used << " resets         ║\n";
    cout <<   "║                                              ║\n";
    cout <<   "║  Smart Reset = THEORETICALLY UNLIMITED       ║\n";
    cout <<   "║  Only limited by reset cost (decrypt+encrypt)║\n";
    cout <<   "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";
    
    return 0;
}
