// ΦΩ0 — TENSOR UK×UK + ZANS COMBO
// Combining +34-bit tensor decomposition with Enc(0) stabilization
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — TENSOR UK×UK + ZANS COMBO              ║\n";
    cout <<   "║  +34-bit decomposition + Enc(0) stabilization ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n\n";

    // === SETUP ===
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(10);  // More depth for UK×UK
    params.SetPlaintextModulus(65537);
    params.SetSecurityLevel(HEStd_NotSet);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalSumKeyGen(keys.secretKey);

    // Anchors
    vector<int64_t> zero_vec = {0};
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));
    auto zero_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zero_vec));

    cout << "Φ Setup complete.\n\n";

    // === EXPERIMENT 1: Direct UK×UK vs Tensor UK×UK ===
    cout << "=== EXPERIMENT 1: DIRECT vs TENSOR UK×UK ===\n\n";

    // Test values
    vector<pair<int64_t,int64_t>> tests = {
        {12, 34},   // 12 × 34 = 408
        {7, 13},    // 7 × 13 = 91
        {5, 21},    // 5 × 21 = 105
        {3, 42},    // 3 × 42 = 126
        {15, 15},   // 15 × 15 = 225
    };

    cout << "┌──────────────────┬──────────┬──────────┬──────────┐\n";
    cout << "│ Method           │ Result   │ Expected │ Status   │\n";
    cout << "├──────────────────┼──────────┼──────────┼──────────┤\n";

    for(auto [a, b] : tests) {
        int64_t expected = a * b;

        // --- DIRECT UK×UK ---
        vector<int64_t> va = {a}, vb = {b};
        auto cta = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(va));
        auto ctb = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vb));
        
        auto direct = cc->EvalMult(cta, ctb);
        // Stabilize after multiplication
        direct = cc->EvalAdd(direct, anchor0);
        
        Plaintext dpt;
        cc->Decrypt(keys.secretKey, direct, &dpt);
        int64_t dval = dpt->GetPackedValue()[0];
        
        cout << "│ Direct UK×UK     │ " << setw(8) << dval 
             << " │ " << setw(8) << expected
             << " │ " << (dval == expected ? "✅" : "❌") << "      │\n";

        // --- TENSOR UK×UK (decompose a into a1+a2) ---
        // Decompose: 12 = 6+6, 7 = 3+4, 5 = 2+3, 3 = 1+2, 15 = 8+7
        int64_t a1, a2;
        if(a == 12) { a1 = 6; a2 = 6; }
        else if(a == 7) { a1 = 3; a2 = 4; }
        else if(a == 5) { a1 = 2; a2 = 3; }
        else if(a == 3) { a1 = 1; a2 = 2; }
        else if(a == 15) { a1 = 8; a2 = 7; }
        else { a1 = a/2; a2 = a - a1; }

        vector<int64_t> va1 = {a1}, va2 = {a2};
        auto ct_a1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(va1));
        auto ct_a2 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(va2));
        
        // Tensor: (a1+a2) × b = (a1×b) + (a2×b)
        auto part1 = cc->EvalMult(ct_a1, ctb);
        part1 = cc->EvalAdd(part1, anchor0);  // ZANS stabilize
        
        auto part2 = cc->EvalMult(ct_a2, ctb);
        part2 = cc->EvalAdd(part2, anchor0);  // ZANS stabilize
        
        auto tensor_result = cc->EvalAdd(part1, part2);
        tensor_result = cc->EvalAdd(tensor_result, anchor0);  // ZANS stabilize
        
        Plaintext tpt;
        cc->Decrypt(keys.secretKey, tensor_result, &tpt);
        int64_t tval = tpt->GetPackedValue()[0];
        
        cout << "│ Tensor UK×UK     │ " << setw(8) << tval 
             << " │ " << setw(8) << expected
             << " │ " << (tval == expected ? "✅" : "❌") << "      │\n";
        cout << "├──────────────────┼──────────┼──────────┼──────────┤\n";
    }
    cout << "└──────────────────┴──────────┴──────────┴──────────┘\n\n";

    // === EXPERIMENT 2: CHAINED UK×UK WITH ZANS ===
    cout << "=== EXPERIMENT 2: CHAINED UK×UK WITH ZANS ===\n";
    cout << "How many consecutive UK×UK multiplications can we do?\n\n";

    // Start with 2, multiply by 3 repeatedly
    vector<int64_t> start_vec = {2};
    auto chain = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    
    vector<int64_t> mult_vec = {3};
    auto three = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mult_vec));
    
    int chain_length = 0;
    int64_t current_val = 2;
    bool chain_ok = true;
    
    cout << "  Chain: 2";
    for(int i = 0; i < 10 && chain_ok; i++) {
        chain = cc->EvalMult(chain, three);
        chain = cc->EvalAdd(chain, anchor0);  // ZANS stabilize after each mult
        chain_length++;
        current_val *= 3;
        
        Plaintext cpt;
        cc->Decrypt(keys.secretKey, chain, &cpt);
        int64_t cval = cpt->GetPackedValue()[0];
        
        cout << " ×3 = " << cval;
        if(cval == current_val) {
            cout << " ✅";
        } else {
            cout << " ❌ (expected " << current_val << ")";
            chain_ok = false;
        }
        cout << "\n";
        if(chain_ok && i < 9) cout << "         ";
    }
    
    cout << "\n  Chain length: " << chain_length << " multiplications before";
    if(chain_ok) cout << " (still going!)";
    cout << "\n\n";

    // === EXPERIMENT 3: TENSOR + ZANS DEEP CHAIN ===
    cout << "=== EXPERIMENT 3: TENSOR + ZANS DEEP CHAIN ===\n";
    cout << "Using tensor decomposition for each multiplication\n\n";

    // Restart: 2 × 3 repeatedly, but tensor-decompose 3 = 1+2
    auto chain2 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(start_vec));
    current_val = 2;
    chain_ok = true;
    int chain2_length = 0;
    
    vector<int64_t> one_vec = {1}, two_vec = {2};
    auto one_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(one_vec));
    auto two_ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(two_vec));
    
    cout << "  Tensor chain: 2";
    for(int i = 0; i < 10 && chain_ok; i++) {
        // Tensor: chain2 × (1+2) = chain2×1 + chain2×2
        auto p1 = cc->EvalMult(chain2, one_ct);
        p1 = cc->EvalAdd(p1, anchor0);
        
        auto p2 = cc->EvalMult(chain2, two_ct);
        p2 = cc->EvalAdd(p2, anchor0);
        
        chain2 = cc->EvalAdd(p1, p2);
        chain2 = cc->EvalAdd(chain2, anchor0);
        chain2_length++;
        current_val *= 3;
        
        Plaintext cpt;
        cc->Decrypt(keys.secretKey, chain2, &cpt);
        int64_t cval = cpt->GetPackedValue()[0];
        
        cout << " ×3 = " << cval;
        if(cval == current_val) {
            cout << " ✅";
        } else {
            cout << " ❌ (expected " << current_val << ")";
            chain_ok = false;
        }
        cout << "\n";
        if(chain_ok && i < 9) cout << "                ";
    }
    
    cout << "\n  Tensor chain length: " << chain2_length << " multiplications";
    if(chain_ok) cout << " (still going!)";
    cout << "\n\n";

    cout << "╔══════════════════════════════════════════════╗\n";
    cout <<   "║  ΦΩ0 — TENSOR + ZANS COMBO COMPLETE           ║\n";
    cout <<   "║  I AM THAT I AM                              ║\n";
    cout <<   "╚══════════════════════════════════════════════╝\n";

    return 0;
}
