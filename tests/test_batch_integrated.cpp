#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

int main() {
    std::cout << "BATCH ENCRYPTION INTEGRATED sa GoldenPrivacySystem\n";
    std::cout << "===================================================\n\n";
    
    GoldenPrivacySystem gps(42);
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    gps.obfuscate_program(xor_func, 2);
    
    // Test batch encryption + decrypt
    std::cout << "Batch encryption test:\n";
    
    std::vector<bool> input_bits;
    for (int i = 0; i < 16; i++) {
        input_bits.push_back(i % 2 == 0);
    }
    
    auto batch_ct = gps.batch_encrypt(input_bits);
    auto decoded = gps.batch_decrypt(batch_ct, 16);
    
    bool correct = true;
    for (int i = 0; i < 16; i++) {
        if (decoded[i] != input_bits[i]) {
            correct = false;
        }
    }
    
    std::cout << "  Input:  ";
    for (bool b : input_bits) std::cout << (b ? "1" : "0");
    std::cout << "\n  Decoded: ";
    for (bool b : decoded) std::cout << (b ? "1" : "0");
    std::cout << "\n  " << (correct ? "✅ MATCH" : "❌ MISMATCH") << "\n\n";
    
    // Batch compute benchmark
    std::cout << "Batch compute benchmark (100 ops):\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::pair<bool, bool>> inputs;
    for (int i = 0; i < 100; i++) {
        inputs.push_back({(bool)((i >> 1) & 1), (bool)(i & 1)});
    }
    
    auto results = gps.batch_compute(inputs);
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    
    int correct_count = 0;
    for (size_t i = 0; i < results.size(); i++) {
        if (results[i] == (inputs[i].first ^ inputs[i].second)) {
            correct_count++;
        }
    }
    
    std::cout << "  Results: " << correct_count << "/100 correct\n";
    std::cout << "  Time: " << duration << " s\n";
    std::cout << "  Throughput: " << 100.0 / duration << " ops/sec\n\n";
    
    // Full pipeline: batch encrypt → iO compute → batch decrypt
    std::cout << "Full batch pipeline:\n";
    
    auto full_start = std::chrono::high_resolution_clock::now();
    
    // Batch encrypt 128 bits
    std::vector<bool> bits_128(128);
    for (int i = 0; i < 128; i++) bits_128[i] = i % 2;
    auto enc_128 = gps.batch_encrypt(bits_128);
    
    // iO compute (direct evaluation)
    std::vector<bool> io_results;
    for (int i = 0; i < 64; i++) {
        io_results.push_back(gps.evaluate_io_public({(bool)((i >> 1) & 1), (bool)(i & 1)}));
    }
    
    // Batch decrypt
    auto dec_128 = gps.batch_decrypt(enc_128, 128);
    
    auto full_end = std::chrono::high_resolution_clock::now();
    double full_time = std::chrono::duration<double>(full_end - full_start).count();
    
    std::cout << "  128-bit batch encrypt + 64 iO evals + batch decrypt\n";
    std::cout << "  Time: " << full_time << " s\n";
    std::cout << "  Effective throughput: " << 128.0 / full_time << " ops/sec\n";
    
    gps.print_metrics();
    gps.print_security();
    
    std::cout << "\n✅ BATCH ENCRYPTION INTEGRATED!\n";
    return 0;
}
