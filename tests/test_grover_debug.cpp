#include "../src/fhe/golden_bootstrapping.h"
#include <iostream>
#include <vector>

int main() {
    GoldenFHE::init_ring();
    
    GoldenFHE::PublicKey pk;
    GoldenFHE::SecretKey sk;
    GoldenFHE::keygen(pk, sk, 42);
    
    std::cout << "Debug: Position 7 search...\n\n";
    
    // Database: solution sa position 7
    std::vector<bool> database = {0, 0, 0, 0, 0, 0, 0, 1};
    
    // I-encrypt
    std::vector<GoldenFHE::Cipher> encrypted_db;
    for (size_t i = 0; i < database.size(); i++) {
        encrypted_db.push_back(GoldenFHE::encrypt(pk, database[i], 1000000 + i));
    }
    
    // I-check ang initial encryption
    std::cout << "Initial encrypted values:\n";
    for (size_t i = 0; i < encrypted_db.size(); i++) {
        bool dec = GoldenFHE::decrypt(encrypted_db[i], sk);
        std::cout << "  db[" << i << "] = " << dec << " (expected " << database[i] << ")\n";
    }
    
    // Oracle: i-flip ang sign ng solution (position 7)
    GoldenBootstrapping::UnlimitedFHE fhe(pk, sk);
    encrypted_db[7] = fhe.not_with_bootstrap(encrypted_db[7]);
    
    std::cout << "\nAfter oracle:\n";
    for (size_t i = 0; i < encrypted_db.size(); i++) {
        bool dec = GoldenFHE::decrypt(encrypted_db[i], sk);
        std::cout << "  db[" << i << "] = " << dec << "\n";
    }
    
    // Diffusion: XOR lahat
    std::vector<GoldenFHE::Cipher> result = encrypted_db;
    GoldenBootstrapping::UnlimitedFHE fhe2(pk, sk);
    
    for (size_t i = 0; i < encrypted_db.size(); i++) {
        for (size_t j = i + 1; j < encrypted_db.size(); j++) {
            GoldenFHE::Cipher xor_result = fhe2.xor_with_bootstrap(encrypted_db[i], encrypted_db[j]);
            result[i] = xor_result;
        }
    }
    
    std::cout << "\nAfter diffusion:\n";
    for (size_t i = 0; i < result.size(); i++) {
        bool dec = GoldenFHE::decrypt(result[i], sk);
        std::cout << "  result[" << i << "] = " << dec << "\n";
    }
    
    // I-measure: hanapin ang "1"
    int measured = 0;
    for (size_t i = 0; i < result.size(); i++) {
        bool dec = GoldenFHE::decrypt(result[i], sk);
        if (dec) {
            measured = static_cast<int>(i);
        }
    }
    
    std::cout << "\nMeasured: " << measured << " (expected 7)\n";
    
    return 0;
}
