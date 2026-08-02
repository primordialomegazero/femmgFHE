// BUDGET CYCLE: b-component as level reservoir
// EvalMult consumes a-level, b stays fresh via addition
// mulY swaps: fresh b becomes new a

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
    std::cout << "\n";
    std::cout << "  BUDGET CYCLE: b-component as level reservoir\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // Standard encoding
    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
    
    PE state = {enc_phi, enc_one};
    
    std::cout << "  Initial: a level=" << state.a->GetLevel() 
              << " b level=" << state.b->GetLevel() << "\n\n";
    
    // Strategy: 
    // Instead of both a and b consuming levels via EvalMult,
    // keep b as "reservoir" via addition
    
    std::cout << "  Standard NAND (both a,b consume via EvalMult):\n";
    PE test = state;
    for (int i = 0; i < 5; i++) {
        auto new_a = cc->EvalMult(test.a, test.a);  // level +1
        auto new_b = cc->EvalMult(test.b, test.b);  // level +1
        test = {new_a, new_b};
        std::cout << "  Step " << i << ": a level=" << test.a->GetLevel()
                  << " b level=" << test.b->GetLevel() << "\n";
    }
    
    std::cout << "\n  Budget-cycle attempt: use EvalAdd for b\n";
    PE cycle = {enc_phi, enc_one};
    
    for (int i = 0; i < 10; i++) {
        // AND: multiply a's, ADD b's (not multiply!)
        auto new_a = cc->EvalMult(cycle.a, cycle.a);      // level +1
        auto new_b = cc->EvalAdd(cycle.b, cycle.b);       // level stays!
        
        cycle = {new_a, new_b};
        
        std::cout << "  Step " << i << ": a level=" << cycle.a->GetLevel()
                  << " b level=" << cycle.b->GetLevel();
        
        // Now swap via mulY: fresh b becomes new a!
        cycle = mulY(cc, cycle);
        std::cout << "  after mulY: a level=" << cycle.a->GetLevel()
                  << " (from b) b level=" << cycle.b->GetLevel() << "\n";
    }
    
    // Check final levels
    std::cout << "\n  Final after 10 cycles:\n";
    std::cout << "  a level=" << cycle.a->GetLevel() << "\n";
    std::cout << "  b level=" << cycle.b->GetLevel() << "\n";
    
    double ratio = decrypt_val(cc, kp, cycle.a) / decrypt_val(cc, kp, cycle.b);
    std::cout << "  ratio=" << std::fixed << std::setprecision(6) << ratio << "\n\n";

    return 0;
}
