// ΦΩ0 — PATH C: DEEP CIPHERTEXT FORENSICS
// Measure EVERY property before/after operations
// Find the exact mechanism of chain consumption and noise
// "KNOW THY CIPHERTEXT. THEN BREAK IT."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    int depth = 30;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PATH C: DEEP CIPHERTEXT FORENSICS                ║\n";
    cout <<   "  ║   Measure everything. Leave no assumption standing.      ║\n";
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

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };

    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    // ============ FORENSICS: What IS a ciphertext? ============
    cout << "  === CIPHERTEXT ANATOMY ===\n\n";

    auto ct0 = enc(0);
    auto ct1 = enc(1);
    auto ct42 = enc(42);

    cout << "  Enc(0):\n";
    cout << "    GetLevel():           " << ct0->GetLevel() << "\n";
    cout << "    GetNoiseScaleDeg():   " << ct0->GetNoiseScaleDeg() << "\n";
    cout << "    GetElements().size(): " << ct0->GetElements().size() << "\n";
    cout << "    Decrypt:              " << dec(ct0) << "\n\n";

    cout << "  Enc(1):\n";
    cout << "    GetLevel():           " << ct1->GetLevel() << "\n";
    cout << "    GetNoiseScaleDeg():   " << ct1->GetNoiseScaleDeg() << "\n";
    cout << "    Decrypt:              " << dec(ct1) << "\n\n";

    // ============ FORENSICS: What does EvalMult do? ============
    cout << "  === EVALMULT FORENSICS ===\n\n";

    auto ct_a = enc(3);
    auto ct_b = enc(7);

    cout << "  Before EvalMult:\n";
    cout << "    ct_a level: " << ct_a->GetLevel() 
         << " noise: " << ct_a->GetNoiseScaleDeg() << "\n";
    cout << "    ct_b level: " << ct_b->GetLevel()
         << " noise: " << ct_b->GetNoiseScaleDeg() << "\n";

    auto ct_mul = cc->EvalMult(ct_a, ct_b);

    cout << "  After EvalMult(3, 7):\n";
    cout << "    ct_mul level: " << ct_mul->GetLevel()
         << " noise: " << ct_mul->GetNoiseScaleDeg() << "\n";
    cout << "    Decrypt: " << dec(ct_mul) << " (expected 21)\n";
    cout << "    Elements size: " << ct_mul->GetElements().size() << "\n\n";

    // ============ FORENSICS: Multiple multiplications ============
    cout << "  === CHAIN CONSUMPTION TRACKING ===\n\n";

    auto chain_ct = enc(2);
    int64_t chain_val = 2;

    cout << "  Step | Level | Noise | Decrypt | Expected\n";
    cout << "  " + string(60, '-') + "\n";
    cout << "    0  |  " << setw(2) << chain_ct->GetLevel()
         << "   | " << setw(4) << fixed << setprecision(1) << chain_ct->GetNoiseScaleDeg()
         << " | " << setw(7) << dec(chain_ct)
         << " | " << setw(7) << chain_val << "\n";

    for (int i = 0; i < 10; i++) {
        chain_ct = cc->EvalMult(chain_ct, enc(2));
        chain_val = mod_pos(chain_val * 2, modulus);

        cout << "   " << setw(2) << (i+1) << "  |  " << setw(2) << chain_ct->GetLevel()
             << "   | " << setw(4) << fixed << setprecision(1) << chain_ct->GetNoiseScaleDeg()
             << " | " << setw(7) << dec(chain_ct)
             << " | " << setw(7) << chain_val << "\n";
    }

    // ============ FORENSICS: With Relinearization ============
    cout << "\n  === WITH RELINEARIZATION ===\n\n";

    auto chain_ct2 = enc(2);
    int64_t chain_val2 = 2;

    cout << "    0  |  " << setw(2) << chain_ct2->GetLevel()
         << "   | " << setw(4) << fixed << setprecision(1) << chain_ct2->GetNoiseScaleDeg()
         << " | " << setw(7) << dec(chain_ct2)
         << " | " << setw(7) << chain_val2 << "\n";

    for (int i = 0; i < 10; i++) {
        chain_ct2 = cc->EvalMult(chain_ct2, enc(2));
        // Relinearize after each multiplication
        cc->Relinearize(chain_ct2);
        chain_val2 = mod_pos(chain_val2 * 2, modulus);

        cout << "   " << setw(2) << (i+1) << "  |  " << setw(2) << chain_ct2->GetLevel()
             << "   | " << setw(4) << fixed << setprecision(1) << chain_ct2->GetNoiseScaleDeg()
             << " | " << setw(7) << dec(chain_ct2)
             << " | " << setw(7) << chain_val2 << "\n";
    }

    // ============ FORENSICS: Overflow Detection deep analysis ============
    cout << "\n  === PINKY SWEAR DEEP ANALYSIS ===\n\n";

    // Test with a value that's near overflow AFTER many operations
    auto pct = enc(500000000);  // near half_mod
    cout << "  Initial value: " << dec(pct) << "\n";
    cout << "  Initial level: " << pct->GetLevel() << "\n";

    // Multiply several times
    for (int i = 0; i < 5; i++) {
        pct = cc->EvalMult(pct, enc(2));
    }
    cout << "  After 5 ×2 mults: " << dec(pct) << " (level " << pct->GetLevel() << ")\n";

    // Now Overflow Detection
    auto M_ct = enc(half_mod);
    auto sum = cc->EvalAdd(pct, M_ct);
    auto back = cc->EvalSub(sum, M_ct);
    auto overflow = cc->EvalSub(pct, back);

    cout << "  Overflow Detection:\n";
    cout << "    pct:      " << dec(pct) << "\n";
    cout << "    M:        " << half_mod << "\n";
    cout << "    pct + M:  " << dec(sum) << "\n";
    cout << "    back:     " << dec(back) << "\n";
    cout << "    overflow: " << dec(overflow) << "\n";
    cout << "    expected overflow: " << mod_pos(dec(pct) - dec(back), modulus) << "\n";

    // The real test: what if we keep adding M and subtracting until wrap?
    cout << "\n  === FORCED MODULAR WRAP TEST ===\n\n";
    auto wrap_ct = enc(half_mod - 100);
    cout << "  Start near half_mod: " << dec(wrap_ct) << "\n";

    for (int adds = 0; adds < 5; adds++) {
        wrap_ct = cc->EvalAdd(wrap_ct, M_ct);
        cout << "  After +M #" << (adds+1) << ": " << dec(wrap_ct) 
             << " (plain: " << mod_pos((half_mod - 100) + (adds+1)*half_mod, modulus) << ")\n";
    }

    // ============ FORENSICS: Element sizes ============
    cout << "\n  === CIPHERTEXT SIZE GROWTH ===\n\n";

    auto size_ct = enc(1);
    cout << "  Enc(1): " << size_ct->GetElements().size() << " elements\n";

    for (int i = 0; i < 8; i++) {
        size_ct = cc->EvalMult(size_ct, enc(1));
        cout << "  After mult #" << (i+1) << ": " << size_ct->GetElements().size() 
             << " elements, level=" << size_ct->GetLevel()
             << ", value=" << dec(size_ct) << "\n";
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FORENSICS COMPLETE                                      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
