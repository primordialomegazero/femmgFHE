// SPIRAL vs NO SPIRAL: Simple NAND chain accuracy
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

int main() {
    std::cout << "\n  SPIRAL vs NO SPIRAL: NAND chain accuracy\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(100);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
    auto enc_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{0.6180339887498949}));

    PE bit1 = {enc_phi, enc_one};
    PE bit0 = {enc_psi, enc_one};

    // 1-EM NAND + mulY rescale
    auto nand_fractal = [&](PE& A, PE& B) -> PE {
        auto AND_a = cc->EvalMult(A.a, B.a);
        auto AND_b = cc->EvalMult(A.b, B.b);
        PE nand_raw = {cc->EvalSub(AND_b, AND_a), AND_b};
        return mulY(cc, nand_raw);
    };

    auto spiral = [&](PE& x) -> PE {
        PE s = mulY(cc, x);
        return mulY_inv(cc, s);
    };

    int depths[] = {5, 10, 15, 20, 30};
    
    std::cout << "  Depth | No Spiral OK | Spiral K=5 OK | Spiral K=1 OK\n";
    std::cout << "  ---------------------------------------------------\n";
    
    for (int depth : depths) {
        int ok_ns = 0, ok_s5 = 0, ok_s1 = 0;
        
        // --- NO SPIRAL ---
        try {
            PE state = bit1;  // start with 1
            PE const_one = bit1;
            
            for (int i = 0; i < depth; i++) {
                state = nand_fractal(state, const_one);
            }
            
            // Decode: apply mulY_inv depth times then check
            PE dec = state;
            for (int i = 0; i < depth; i++) dec = mulY_inv(cc, dec);
            double ratio = decrypt_val(cc, kp, dec.a) / decrypt_val(cc, kp, dec.b);
            int bit = (ratio > 1.0) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;  // 1 NAND 1 = 0, 0 NAND 1 = 1...
            if (bit == expected) ok_ns = 1;
        } catch (...) { ok_ns = -1; }
        
        // --- SPIRAL EVERY 5 ---
        try {
            PE state = bit1;
            PE const_one = bit1;
            int extra_spirals = 0;
            
            for (int i = 0; i < depth; i++) {
                state = nand_fractal(state, const_one);
                if ((i + 1) % 5 == 0) {
                    state = spiral(state);
                    extra_spirals++;
                }
            }
            
            PE dec = state;
            for (int i = 0; i < depth + extra_spirals; i++) dec = mulY_inv(cc, dec);
            double ratio = decrypt_val(cc, kp, dec.a) / decrypt_val(cc, kp, dec.b);
            int bit = (ratio > 1.0) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;
            if (bit == expected) ok_s5 = 1;
        } catch (...) { ok_s5 = -1; }
        
        // --- SPIRAL EVERY 1 ---
        try {
            PE state = bit1;
            PE const_one = bit1;
            
            for (int i = 0; i < depth; i++) {
                state = nand_fractal(state, const_one);
                state = spiral(state);
            }
            
            PE dec = state;
            for (int i = 0; i < depth * 2; i++) dec = mulY_inv(cc, dec);
            double ratio = decrypt_val(cc, kp, dec.a) / decrypt_val(cc, kp, dec.b);
            int bit = (ratio > 1.0) ? 1 : 0;
            int expected = (depth % 2 == 0) ? 1 : 0;
            if (bit == expected) ok_s1 = 1;
        } catch (...) { ok_s1 = -1; }
        
        std::cout << "  " << std::setw(5) << depth << " | ";
        std::cout << (ok_ns == 1 ? "YES" : (ok_ns == -1 ? "CRASH" : "NO ")) << "           | ";
        std::cout << (ok_s5 == 1 ? "YES" : (ok_s5 == -1 ? "CRASH" : "NO ")) << "           | ";
        std::cout << (ok_s1 == 1 ? "YES" : (ok_s1 == -1 ? "CRASH" : "NO ")) << "\n";
    }
    
    std::cout << "\n  Done.\n\n";
    return 0;
}
