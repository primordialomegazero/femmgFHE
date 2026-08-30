// ============================================
// φ-NAND DEBUG — PAANO NAGBBUILD
//
// I-trace ang building process:
// NAND → NOT → AND → OR → XOR
//
// Makikita natin kung saan nasisira
//
// Author: Dan Fernandez / Primordial Omega Zero
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

class PhiNandDebug {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    
public:
    PhiNandDebug() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_256_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(6);
        cout << "========================================\n";
        cout << "  φ-NAND DEBUG\n";
        cout << "  Paano nagbbuild?\n";
        cout << "========================================\n\n";
    }
    
    vector<double> encode(double bit) {
        vector<double> dims(16, 0.0);
        double val = (bit == 0.0) ? PHI_INV : PHI;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    double get_avg(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        return sum / 16.0;
    }
    
    Ciphertext<DCRTPoly> encrypt(vector<double> dims) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dims);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    vector<complex<double>> decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    }
    
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> sub(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalSub(a, b);
    }
    
    void debug_00() {
        cout << "========================================\n";
        cout << "  DEBUG: A=0, B=0\n";
        cout << "========================================\n\n";
        
        auto a = encrypt(encode(0.0));
        auto b = encrypt(encode(0.0));
        
        // STEP 1: NAND
        auto sum_ab = add(a, b);
        auto phi_inv_ct = encrypt(encode(0.0));
        auto nand_ct = sub(sum_ab, phi_inv_ct);
        
        double sum_val = get_avg(decrypt(sum_ab));
        double nand_val = get_avg(decrypt(nand_ct));
        
        cout << "  a = φ⁻¹ = " << PHI_INV << "\n";
        cout << "  b = φ⁻¹ = " << PHI_INV << "\n";
        cout << "  sum = a + b = " << sum_val << "\n";
        cout << "  nand = sum - φ⁻¹ = " << nand_val << "\n\n";
        
        cout << "  EXPECTED NAND(0,0) = 1 (dapat φ o mas mataas)\n";
        cout << "  ACTUAL: " << nand_val << "\n\n";
        
        // STEP 2: NOT = NAND(a,a)
        auto not_ct = sub(add(a, a), phi_inv_ct);
        double not_val = get_avg(decrypt(not_ct));
        
        cout << "  NOT(0) = NAND(0,0) = " << not_val << "\n";
        cout << "  EXPECTED NOT(0) = 1\n\n";
        
        // STEP 3: AND = NOT(NAND)
        auto and_nand = sub(add(a, b), phi_inv_ct);
        auto and_not = sub(add(and_nand, and_nand), phi_inv_ct);
        double and_val = get_avg(decrypt(and_not));
        
        cout << "  AND(0,0) = NOT(NAND(0,0)) = " << and_val << "\n";
        cout << "  EXPECTED AND(0,0) = 0\n\n";
        
        cout << "  ANG PROBLEMA:\n";
        cout << "  - NAND(0,0) ay " << nand_val << " (dapat φ para sa 1)\n";
        cout << "  - Kaya NOT(0) ay mali rin\n";
        cout << "  - Kailangan ng tamang φ-reference\n\n";
    }
    
    void run() {
        debug_00();
    }
};

int main() {
    PhiNandDebug debug;
    debug.run();
    return 0;
}
