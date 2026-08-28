// ============================================
// φ-LOG SPACE FHE — TEST SUITE
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class PhiLogSpaceTest {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    Ciphertext<DCRTPoly> encrypt_log(double value) {
        double log_phi_value = log(value) / LN_PHI;
        vector<double> val(1, log_phi_value);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    double decrypt_value(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);
    }
    
public:
    PhiLogSpaceTest() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(30);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(1);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
    }
    
    void run_all() {
        cout << "========================================\n";
        cout << "  φ-LOG SPACE FHE — TEST SUITE\n";
        cout << "========================================\n\n";
        
        int success = 0;
        int total = 0;
        
        // TEST 1: MULTIPLICATION
        cout << "TEST 1: MULTIPLICATION\n";
        cout << "----------------------\n";
        
        vector<pair<double, double>> mult_tests = {
            {3.0, 7.0}, {2.0, 5.0}, {11.0, 13.0}, {2.5, 3.5}, {0.5, 0.25}
        };
        
        for (auto& [a, b] : mult_tests) {
            auto ct_a = encrypt_log(a);
            auto ct_b = encrypt_log(b);
            auto ct_result = cc->EvalAdd(ct_a, ct_b);
            
            double result = decrypt_value(ct_result);
            double expected = a * b;
            bool match = abs(result - expected) < expected * 0.01;
            
            total++;
            if (match) success++;
            
            cout << "  " << fixed << setprecision(2) << a << " × " << b
                 << " = " << setprecision(4) << result
                 << " (expected " << expected << ") "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        // TEST 2: DIVISION
        cout << "\nTEST 2: DIVISION\n";
        cout << "----------------\n";
        
        vector<pair<double, double>> div_tests = {
            {10.0, 3.0}, {100.0, 7.0}, {21.0, 7.0}
        };
        
        for (auto& [a, b] : div_tests) {
            auto ct_a = encrypt_log(a);
            auto ct_b = encrypt_log(b);
            auto neg_b = cc->EvalNegate(ct_b);
            auto ct_result = cc->EvalAdd(ct_a, neg_b);
            
            double result = decrypt_value(ct_result);
            double expected = a / b;
            bool match = abs(result - expected) < expected * 0.01;
            
            total++;
            if (match) success++;
            
            cout << "  " << fixed << setprecision(1) << a << " / " << b
                 << " = " << setprecision(4) << result
                 << " (expected " << expected << ") "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        // TEST 3: CHAINED
        cout << "\nTEST 3: CHAINED (2×3×5×7)\n";
        cout << "-------------------------\n";
        
        auto ct = encrypt_log(2.0);
        ct = cc->EvalAdd(ct, encrypt_log(3.0));
        ct = cc->EvalAdd(ct, encrypt_log(5.0));
        ct = cc->EvalAdd(ct, encrypt_log(7.0));
        
        double chained = decrypt_value(ct);
        double expected_chain = 210.0;
        bool chain_match = abs(chained - expected_chain) < expected_chain * 0.01;
        
        total++;
        if (chain_match) success++;
        
        cout << "  2 × 3 × 5 × 7 = " << chained
             << " (expected 210) " << (chain_match ? "✅" : "❌") << "\n";
        
        cout << "\n========================================\n";
        cout << "  RESULTS: " << success << "/" << total << " ✅\n";
        cout << "========================================\n";
    }
};

int main() {
    PhiLogSpaceTest test;
    test.run_all();
    return 0;
}
