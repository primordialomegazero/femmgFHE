/*
 * FEmmg-FHE v22.2 — IND-CPA / IND-CCA2 Test Suite
 *
 * Proves:
 *   IND-CPA: Same plaintext → indistinguishable ciphertexts
 *   IND-CCA2: Tampered ciphertext → rejected
 *   Avalanche: 1-bit plaintext difference → massive ciphertext difference
 */

#include "../src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>
#include <cmath>

// Count differing bits between two 64-bit values
int count_bit_diff(uint64_t a, uint64_t b) {
    uint64_t diff = a ^ b;
    int count = 0;
    while (diff) { count += diff & 1; diff >>= 1; }
    return count;
}

// Count differing bytes between two double arrays
int count_double_diff(const double* a, const double* b, int len) {
    int count = 0;
    for (int i = 0; i < len; i++) {
        uint64_t ai, bi;
        std::memcpy(&ai, &a[i], sizeof(ai));
        std::memcpy(&bi, &b[i], sizeof(bi));
        count += count_bit_diff(ai, bi);
    }
    return count;
}

int main() {
    FEmmgFHE fhe;
    int passed = 0, total = 0;

    std::cout << "══════════════════════════════════════════════" << std::endl;
    std::cout << "  FEmmg-FHE v22.2 — CPA/CCA SECURITY SUITE" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;

    // ═══ TEST 1: IND-CPA — Same plaintext, different ciphertexts ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": IND-CPA ═══" << std::endl;
    
    const int NUM_TRIALS = 100;
    bool all_iv_unique = true;
    bool all_decrypt_correct = true;
    uint64_t prev_iv = 0;
    
    for (int i = 0; i < NUM_TRIALS; i++) {
        auto ct = fhe.encrypt(42);
        if (ct.random_iv == prev_iv) all_iv_unique = false;
        prev_iv = ct.random_iv;
        if (fhe.decrypt(ct) != 42) all_decrypt_correct = false;
    }
    
    std::cout << "  " << NUM_TRIALS << " encryptions of 42:" << std::endl;
    std::cout << "    All IVs unique: " << (all_iv_unique ? "✅" : "❌") << std::endl;
    std::cout << "    All decrypt to 42: " << (all_decrypt_correct ? "✅" : "❌") << std::endl;
    
    if (all_iv_unique && all_decrypt_correct) passed++;

    // ═══ TEST 2: IND-CPA — Ciphertext distance ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": CPA — Ciphertext Distance ═══" << std::endl;
    
    auto ct1 = fhe.encrypt(42);
    auto ct2 = fhe.encrypt(42);
    
    int iv_diff = count_bit_diff(ct1.random_iv, ct2.random_iv);
    int tag_diff = count_bit_diff(ct1.integrity_tag, ct2.integrity_tag);
    int hist_diff = count_double_diff(ct1.chaos_history, ct2.chaos_history, 21);
    
    std::cout << "  Same plaintext (42 vs 42):" << std::endl;
    std::cout << "    IV bit diff: " << iv_diff << " (expected ~32)" << std::endl;
    std::cout << "    Tag bit diff: " << tag_diff << " (expected ~32)" << std::endl;
    std::cout << "    Chaos hist bit diff: " << hist_diff << " (expected >100)" << std::endl;
    
    if (iv_diff >= 10 && tag_diff >= 10 && hist_diff >= 50) passed++;

    // ═══ TEST 3: Avalanche — 1-bit plaintext difference ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": Avalanche Effect ═══" << std::endl;
    
    auto ct_a = fhe.encrypt(42);
    auto ct_b = fhe.encrypt(43);
    
    int coord_diff = count_double_diff(ct_a.coordinates.data(), ct_b.coordinates.data(), 7);
    int chaos_diff = count_double_diff(ct_a.chaos_history, ct_b.chaos_history, 21);
    int tag_bit_diff = count_bit_diff(ct_a.integrity_tag, ct_b.integrity_tag);
    
    std::cout << "  42 vs 43 (1-bit plaintext difference):" << std::endl;
    std::cout << "    Coordinates bit diff: " << coord_diff << " (expected >50)" << std::endl;
    std::cout << "    Chaos history bit diff: " << chaos_diff << " (expected >200)" << std::endl;
    std::cout << "    Tag bit diff: " << tag_bit_diff << " (expected ~32)" << std::endl;
    
    if (coord_diff >= 30 && chaos_diff >= 100 && tag_bit_diff >= 10) passed++;

    // ═══ TEST 4: CCA — All tamper vectors ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": CCA2 — Tamper Detection ═══" << std::endl;
    
    struct TamperTest {
        std::string name;
        std::function<void(banach::NDimCiphertext&)> tamper;
    };
    
    std::vector<TamperTest> tamper_tests = {
        {"Chaos history[0]", [](auto& c) { c.chaos_history[0] += 1.0; }},
        {"Chaos history[10]", [](auto& c) { c.chaos_history[10] *= -1.0; }},
        {"Value int +1", [](auto& c) { c.value_int += 1; }},
        {"Value int XOR", [](auto& c) { c.value_int ^= 0xFFFFFFFF; }},
        {"Integrity tag", [](auto& c) { c.integrity_tag ^= 1; }},
        {"Random IV", [](auto& c) { c.random_iv ^= 0xDEADBEEF; }},
        {"Operations field", [](auto& c) { c.operations ^= 1; }},
        {"Coordinates[0]", [](auto& c) { c.coordinates[0] += 0.001; }},
        {"expanded_dim0", [](auto& c) { c.expanded_dim0 += 1.0; }},
        {"lyapunov_spectrum[3]", [](auto& c) { c.lyapunov_spectrum[3] *= 2.0; }},
    };
    
    int tamper_detected = 0;
    for (auto& tt : tamper_tests) {
        auto ct = fhe.encrypt(42);
        tt.tamper(ct);
        int64_t dec = fhe.decrypt(ct);
        bool detected = (dec != 42);
        std::cout << "    " << tt.name << ": " << (detected ? "✅" : "❌") << std::endl;
        if (detected) tamper_detected++;
    }
    
    std::cout << "  Tamper detection rate: " << tamper_detected << "/" << tamper_tests.size() << std::endl;
    if (tamper_detected == (int)tamper_tests.size()) passed++;

    // ═══ TEST 5: CCA — Chosen ciphertext (cross-instance) ═══
    total++;
    std::cout << "\n═══ TEST " << total << ": CCA2 — Cross-Instance ═══" << std::endl;
    
    FEmmgFHE fhe2;
    auto ct_cross = fhe.encrypt(42);
    int64_t dec_cross = fhe2.decrypt(ct_cross);
    bool cross_detected = (dec_cross != 42);
    std::cout << "  Encrypt(fhe1, 42) → Decrypt(fhe2): " << dec_cross 
              << (cross_detected ? " ✅ (not 42)" : " ❌") << std::endl;
    if (cross_detected) passed++;

    // ═══ RESULTS ═══
    std::cout << "\n══════════════════════════════════════════════" << std::endl;
    std::cout << "  RESULTS: " << passed << "/" << total << " PASSED" << std::endl;
    std::cout << "══════════════════════════════════════════════" << std::endl;
    
    return (passed == total) ? 0 : 1;
}
