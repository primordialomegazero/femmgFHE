/*
 * FIBONACCI MULTIPLICATION NOISE MEASUREMENT
 * Fixed: Proper context handling for ciphertext operations
 */

#include "seal/seal.h"
#include <iostream>
#include <vector>

using namespace seal;

std::vector<uint64_t> generate_fib(uint64_t limit) {
    std::vector<uint64_t> fib = {1, 2};
    while (fib.back() <= limit) {
        fib.push_back(fib[fib.size()-1] + fib[fib.size()-2]);
    }
    fib.pop_back();
    return fib;
}

std::vector<int> zeckendorf(uint64_t n, const std::vector<uint64_t>& fib) {
    std::vector<int> indices;
    uint64_t rem = n;
    for (int i = (int)fib.size() - 1; i >= 0 && rem > 0; i--) {
        if (fib[i] <= rem) {
            indices.push_back(i);
            rem -= fib[i];
        }
    }
    return indices;
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  FIBONACCI MULTIPLICATION - ACTUAL NOISE MEASUREMENT   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 20));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);

    // Pre-compute Enc(0)
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    auto fib = generate_fib(1000000);

    std::cout << "Measuring Fibonacci multiplication noise...\n\n";

    // Test 1: Small multiplier (2)
    {
        Plaintext plain_3("3");
        Ciphertext ct;
        encryptor.encrypt(plain_3, ct);
        int start_noise = decryptor.invariant_noise_budget(ct);

        std::cout << "  ×2: start noise = " << start_noise << " bits\n";

        // Multiply by 2 using Fibonacci method
        auto idx = zeckendorf(2, fib);
        std::vector<Ciphertext> terms;
        terms.push_back(ct);  // F0 = 1
        Ciphertext ct2 = ct;
        evaluator.add_inplace(ct2, ct);  // F1 = 2
        evaluator.add_inplace(ct2, enc_zero);
        terms.push_back(ct2);

        Ciphertext result = terms[idx[0]];
        for (size_t j = 1; j < idx.size(); j++) {
            evaluator.add_inplace(result, terms[idx[j]]);
            evaluator.add_inplace(result, enc_zero);
        }

        int end_noise = decryptor.invariant_noise_budget(result);
        int noise_cost = start_noise - end_noise;

        std::cout << "    end noise = " << end_noise << " bits\n";
        std::cout << "    cost = " << noise_cost << " bits\n\n";
    }

    // Test 2: Medium multiplier (42)
    {
        Plaintext plain_7("7");
        Ciphertext ct;
        encryptor.encrypt(plain_7, ct);
        int start_noise = decryptor.invariant_noise_budget(ct);

        std::cout << "  ×42: start noise = " << start_noise << " bits\n";

        // Multiply by 42 using Fibonacci method
        auto idx = zeckendorf(42, fib);
        int max_i = idx[0];
        std::vector<Ciphertext> terms(max_i + 1);
        terms[0] = ct;
        terms[1] = ct;
        evaluator.add_inplace(terms[1], terms[1]);
        evaluator.add_inplace(terms[1], enc_zero);

        for (int i = 2; i <= max_i; i++) {
            terms[i] = terms[i-1];
            evaluator.add_inplace(terms[i], terms[i-2]);
            evaluator.add_inplace(terms[i], enc_zero);
        }

        Ciphertext result = terms[idx[0]];
        for (size_t j = 1; j < idx.size(); j++) {
            evaluator.add_inplace(result, terms[idx[j]]);
            evaluator.add_inplace(result, enc_zero);
        }

        int end_noise = decryptor.invariant_noise_budget(result);
        int noise_cost = start_noise - end_noise;

        std::cout << "    end noise = " << end_noise << " bits\n";
        std::cout << "    cost = " << noise_cost << " bits\n";
        std::cout << "    Fib terms: ";
        for (int idx_val : idx) std::cout << idx_val << " ";
        std::cout << "\n\n";
    }

    // Test 3: Large multiplier (1000)
    {
        Plaintext plain_5("5");
        Ciphertext ct;
        encryptor.encrypt(plain_5, ct);
        int start_noise = decryptor.invariant_noise_budget(ct);

        std::cout << "  ×1000: start noise = " << start_noise << " bits\n";

        // Multiply by 1000 using Fibonacci method
        auto idx = zeckendorf(1000, fib);
        int max_i = idx[0];
        std::vector<Ciphertext> terms(max_i + 1);
        terms[0] = ct;
        terms[1] = ct;
        evaluator.add_inplace(terms[1], terms[1]);
        evaluator.add_inplace(terms[1], enc_zero);

        for (int i = 2; i <= max_i; i++) {
            terms[i] = terms[i-1];
            evaluator.add_inplace(terms[i], terms[i-2]);
            evaluator.add_inplace(terms[i], enc_zero);
        }

        Ciphertext result = terms[idx[0]];
        for (size_t j = 1; j < idx.size(); j++) {
            evaluator.add_inplace(result, terms[idx[j]]);
            evaluator.add_inplace(result, enc_zero);
        }

        int end_noise = decryptor.invariant_noise_budget(result);
        int noise_cost = start_noise - end_noise;

        std::cout << "    end noise = " << end_noise << " bits\n";
        std::cout << "    cost = " << noise_cost << " bits\n";
        std::cout << "    Fib terms: ";
        for (int idx_val : idx) std::cout << idx_val << " ";
        std::cout << "\n\n";
    }

    // Test 4: Direct multiply_plain for comparison
    {
        Plaintext plain_5("5");
        Ciphertext ct;
        encryptor.encrypt(plain_5, ct);
        int start_noise = decryptor.invariant_noise_budget(ct);

        std::cout << "  Direct multiply_plain ×1000: start noise = " << start_noise << " bits\n";

        Plaintext plain_1000("1000");
        evaluator.multiply_plain_inplace(ct, plain_1000);

        int end_noise = decryptor.invariant_noise_budget(ct);
        int noise_cost = start_noise - end_noise;

        std::cout << "    end noise = " << end_noise << " bits\n";
        std::cout << "    cost = " << noise_cost << " bits\n\n";
    }

    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  RESULTS SUMMARY                            ║\n";
    std::cout << "╠══════════════════════════════════════════════╣\n";
    std::cout << "║  Check the numbers above for actual costs   ║\n";
    std::cout << "║                                             ║\n";
    std::cout << "║  Paper claims: 1.6 bits/multiply           ║\n";
    std::cout << "║  Actual:       See measured above          ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
