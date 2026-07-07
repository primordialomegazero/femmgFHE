/*
 * ZANS TRUE COMPARISON
 * Dito natin malalaman kung ang ZANS ba talaga ay nagco-contract
 * ng noise kumpara sa ordinary addition na nag-e-expand.
 *
 * Ang pagkakaiba sa naunang tests:
 * - Dati: ZANS(ct + Enc(0)) vs Ordinary(ct + fresh Enc(1))
 *   Pareho silang fresh ang dinadagdag, kaya pareho ang noise growth.
 *
 * - Ngayon: ZANS(ct + Enc(0)) vs Ordinary(ct + ct) o (ct + lumang ct)
 *   Dito makikita kung nag-e-expand ang ordinary at nagco-contract ang ZANS.
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace seal;
using namespace std;

void print_header(const string& title) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(60, '=') << "\n\n";
}

int main() {
    print_header("ZANS vs ORDINARY - TRUE NOISE COMPARISON");

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(8192);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(8192));
    parms.set_plain_modulus(PlainModulus::Batching(8192, 20));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    BatchEncoder batch_encoder(context);

    // Kunin ang noise budget ng sariwang ciphertext
    Plaintext pt_zero("0");
    Ciphertext fresh_ct(context);
    encryptor.encrypt(pt_zero, fresh_ct);
    int fresh_noise = decryptor.invariant_noise_budget(fresh_ct);
    cout << "  Fresh ciphertext noise budget: " << fresh_noise << " bits\n\n";

    // ==========================================
    // TEST 1: ZANS - Paulit-ulit na pagdagdag ng Enc(0)
    // ==========================================
    cout << "--- TEST 1: ZANS (Adding Enc(0) to itself repeatedly) ---\n";
    cout << "    Ito ang ZANS: ct = ct + Enc(0)\n\n";

    Plaintext pt_value("42");
    Ciphertext ct_zans(context);
    encryptor.encrypt(pt_value, ct_zans);
    int zans_start = decryptor.invariant_noise_budget(ct_zans);

    Plaintext pt_zero_plain("0");
    Ciphertext ct_zero(context);

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        encryptor.encrypt(pt_zero_plain, ct_zero);
        evaluator.add_inplace(ct_zans, ct_zero);
    }

    auto end = chrono::high_resolution_clock::now();
    double zans_time = chrono::duration<double>(end - start).count();

    int zans_end = decryptor.invariant_noise_budget(ct_zans);

    // Decrypt to check
    Plaintext zans_result;
    decryptor.decrypt(ct_zans, zans_result);
    cout << "  Start noise: " << zans_start << " bits\n";
    cout << "  End noise:   " << zans_end << " bits\n";
    cout << "  Drift:       " << (zans_start - zans_end) << " bits\n";
    cout << "  Value:       " << zans_result.to_string() << " (expected 42)\n";
    cout << "  Time:        " << zans_time << " sec\n\n";

    // ==========================================
    // TEST 2: ORDINARY - Paulit-ulit na pagdagdag ng ciphertext sa sarili
    // Ito ang dapat magpakita ng EXPANSION
    // ==========================================
    cout << "--- TEST 2: ORDINARY (Adding ciphertext to ITSELF) ---\n";
    cout << "    ct = ct + ct  (noise should grow FAST!)\n\n";

    Ciphertext ct_ord_self(context);
    encryptor.encrypt(pt_value, ct_ord_self);
    int ord_start = decryptor.invariant_noise_budget(ct_ord_self);

    start = chrono::high_resolution_clock::now();

    // Adding ct to itself — ito ang magpapalaki ng noise nang mabilis
    for (int i = 0; i < 100; i++) {  // 100 lang, kasi mabilis mag-expand
        Ciphertext ct_copy = ct_ord_self;  // Kopyahin muna
        evaluator.add_inplace(ct_ord_self, ct_copy);
    }

    end = chrono::high_resolution_clock::now();
    double ord_self_time = chrono::duration<double>(end - start).count();

    int ord_self_end = decryptor.invariant_noise_budget(ct_ord_self);

    Plaintext ord_self_result;
    decryptor.decrypt(ct_ord_self, ord_self_result);
    cout << "  Start noise: " << ord_start << " bits\n";
    cout << "  End noise:   " << ord_self_end << " bits\n";
    cout << "  Drift:       " << (ord_start - ord_self_end) << " bits\n";
    cout << "  Value:       " << ord_self_result.to_string() << "\n";
    cout << "  Time:        " << ord_self_time << " sec\n\n";

    // ==========================================
    // TEST 3: ORDINARY - Pagdagdag ng LUMA na ciphertext
    // ==========================================
    cout << "--- TEST 3: ORDINARY (Adding OLD ciphertext repeatedly) ---\n";
    cout << "    ct = ct + old_ct (old_ct accumulates noise)\n\n";

    Ciphertext ct_ord_old(context);
    encryptor.encrypt(pt_value, ct_ord_old);
    int ord_old_start = decryptor.invariant_noise_budget(ct_ord_old);

    Ciphertext ct_accumulator(context);
    encryptor.encrypt(pt_zero_plain, ct_accumulator);
    int old_ct_noise_start = decryptor.invariant_noise_budget(ct_accumulator);
    cout << "  Old ciphertext start noise: " << old_ct_noise_start << " bits\n";

    start = chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        // Ang ct_accumulator ay patuloy na lumalaki ang noise
        Ciphertext old_copy = ct_accumulator;
        evaluator.add_inplace(ct_ord_old, old_copy);

        // I-update ang accumulator para lumaki ang noise nito
        Ciphertext fresh_zero(context);
        encryptor.encrypt(pt_zero_plain, fresh_zero);
        evaluator.add_inplace(ct_accumulator, fresh_zero);
    }

    end = chrono::high_resolution_clock::now();
    double ord_old_time = chrono::duration<double>(end - start).count();

    int ord_old_end = decryptor.invariant_noise_budget(ct_ord_old);
    int old_ct_noise_end = decryptor.invariant_noise_budget(ct_accumulator);

    Plaintext ord_old_result;
    decryptor.decrypt(ct_ord_old, ord_old_result);
    cout << "  Start noise (target):     " << ord_old_start << " bits\n";
    cout << "  End noise (target):       " << ord_old_end << " bits\n";
    cout << "  Drift (target):           " << (ord_old_start - ord_old_end) << " bits\n";
    cout << "  Old ct start noise:       " << old_ct_noise_start << " bits\n";
    cout << "  Old ct end noise:         " << old_ct_noise_end << " bits\n";
    cout << "  Value:                    " << ord_old_result.to_string() << "\n";
    cout << "  Time:                     " << ord_old_time << " sec\n\n";

    // ==========================================
    // TEST 4: ZANS vs ORDINARY - HEAD TO HEAD
    // Parehong starting ciphertext, magkaiba ang dinadagdag
    // ==========================================
    cout << "--- TEST 4: HEAD TO HEAD ---\n";
    cout << "    Parehong start, 1000 operations each\n\n";

    // ZANS side
    Ciphertext ct_head_zans(context);
    encryptor.encrypt(pt_value, ct_head_zans);
    int head_zans_start = decryptor.invariant_noise_budget(ct_head_zans);

    for (int i = 0; i < 1000; i++) {
        Ciphertext zero_ct(context);
        encryptor.encrypt(pt_zero_plain, zero_ct);
        evaluator.add_inplace(ct_head_zans, zero_ct);
    }

    int head_zans_end = decryptor.invariant_noise_budget(ct_head_zans);
    Plaintext head_zans_result;
    decryptor.decrypt(ct_head_zans, head_zans_result);

    // Ordinary side (adding old ciphertexts)
    Ciphertext ct_head_ord(context);
    encryptor.encrypt(pt_value, ct_head_ord);
    int head_ord_start = decryptor.invariant_noise_budget(ct_head_ord);

    Ciphertext old_ct(context);
    encryptor.encrypt(pt_zero_plain, old_ct);

    for (int i = 0; i < 1000; i++) {
        Ciphertext old_copy = old_ct;
        evaluator.add_inplace(ct_head_ord, old_copy);
        // Palakihin ang old_ct
        Ciphertext fresh(context);
        encryptor.encrypt(pt_zero_plain, fresh);
        evaluator.add_inplace(old_ct, fresh);
    }

    int head_ord_end = decryptor.invariant_noise_budget(ct_head_ord);
    Plaintext head_ord_result;
    decryptor.decrypt(ct_head_ord, head_ord_result);

    cout << "  ZANS:\n";
    cout << "    Start: " << head_zans_start << " bits\n";
    cout << "    End:   " << head_zans_end << " bits\n";
    cout << "    Drift: " << (head_zans_start - head_zans_end) << " bits\n";
    cout << "    Value: " << head_zans_result.to_string() << " ✅\n\n";

    cout << "  ORDINARY:\n";
    cout << "    Start: " << head_ord_start << " bits\n";
    cout << "    End:   " << head_ord_end << " bits\n";
    cout << "    Drift: " << (head_ord_start - head_ord_end) << " bits\n";
    cout << "    Value: " << head_ord_result.to_string() << " ✅\n\n";

    // ==========================================
    // CONCLUSION
    // ==========================================
    print_header("CONCLUSION");

    cout << "  Noise drift comparison:\n";
    cout << "    ZANS:      " << (zans_start - zans_end) << " bits (adding Enc(0))\n";
    cout << "    Ordinary (self-add): " << (ord_start - ord_self_end) << " bits (ct + ct)\n";
    cout << "    Ordinary (old ct):  " << (ord_old_start - ord_old_end) << " bits (ct + old)\n\n";

    cout << "  Head to head:\n";
    cout << "    ZANS drift:      " << (head_zans_start - head_zans_end) << " bits\n";
    cout << "    Ordinary drift:  " << (head_ord_start - head_ord_end) << " bits\n\n";

    if ((head_zans_start - head_zans_end) < (head_ord_start - head_ord_end)) {
        cout << "  ✅ ZANS shows LESS noise growth than ordinary!\n";
        cout << "  This supports the contraction hypothesis.\n";
    } else if ((head_zans_start - head_zans_end) == (head_ord_start - head_ord_end)) {
        cout << "  ⚠️ ZANS and ordinary show EQUAL noise growth.\n";
        cout << "  More investigation needed.\n";
    } else {
        cout << "  ❌ ZANS shows MORE noise growth than ordinary.\n";
        cout << "  Contradicts the hypothesis.\n";
    }

    cout << "\n  Tandaan:\n";
    cout << "  - Ang ZANS ay nagdadagdag ng Enc(0). Dapat mas mabagal ang noise growth.\n";
    cout << "  - Ang Ordinary (self-add) ay dapat mabilis mag-expand ng noise.\n";
    cout << "  - Kung pareho sila, ibig sabihin ang Enc(0) ay may dala pa ring noise.\n";
    cout << "  - Ang tunay na pagsubok ay kung ang ZANS ay KAYANG mag-decrypt\n";
    cout << "    pagkatapos ng mas maraming operasyon kaysa sa ordinary.\n\n";

    return 0;
}
