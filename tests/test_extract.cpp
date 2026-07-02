#include "../src/core/triple_layer_fhe.h"
#include <iostream>
using namespace std;

int main() {
    FEmmgFHE fhe;
    uint64_t seed = 42;
    
    cout << "=== EXTRACTION ROUNDTRIP TEST ===" << endl;
    
    // Create original ciphertexts
    auto orig_a = fhe.encrypt(100);
    auto orig_b = fhe.encrypt(200);
    
    cout << "Original ct_a.value_int = " << orig_a.value_int << endl;
    cout << "Original ct_b.value_int = " << orig_b.value_int << endl;
    
    // Manually add (should work)
    auto direct_sum = fhe.add(orig_a, orig_b);
    int64_t direct_dec = fhe.decrypt(direct_sum);
    cout << "Direct add result: " << direct_dec << " (expected 300)" << endl;
    
    // Now go through pack/extract cycle
    TripleLayerFHE triple;
    
    // Pack
    auto packed_a = triple.encrypt(100, seed);
    auto packed_b = triple.encrypt(200, seed);
    
    // Extract (without XOR — just the raw bytes)
    // Let's manually extract the NDimCiphertext from the packed bytes
    
    // The packed bytes are: XOR( NDimCiphertext_bytes | padding )
    // We need to de-XOR first
    
    anti_lattice::InfoTheoreticLayer it;
    it.generate_pad(64 * 8, seed);
    
    vector<uint8_t> raw_a(64 * 8), raw_b(64 * 8);
    it.decrypt(packed_a.data(), raw_a.data(), 64 * 8);
    it.decrypt(packed_b.data(), raw_b.data(), 64 * 8);
    
    banach::NDimCiphertext extracted_a, extracted_b;
    memcpy(&extracted_a, raw_a.data(), sizeof(extracted_a));
    memcpy(&extracted_b, raw_b.data(), sizeof(extracted_b));
    
    cout << "\nAfter pack→extract:" << endl;
    cout << "Extracted ct_a.value_int = " << extracted_a.value_int << endl;
    cout << "Extracted ct_b.value_int = " << extracted_b.value_int << endl;
    cout << "Match a: " << (orig_a.value_int == extracted_a.value_int ? "YES" : "NO") << endl;
    cout << "Match b: " << (orig_b.value_int == extracted_b.value_int ? "YES" : "NO") << endl;
    
    // Now try decrypting the extracted ciphertexts
    int64_t dec_a = fhe.decrypt(extracted_a);
    int64_t dec_b = fhe.decrypt(extracted_b);
    cout << "\nDecrypt extracted: a=" << dec_a << " (expect 100), b=" << dec_b << " (expect 200)" << endl;
    
    // Try adding extracted ciphertexts
    auto extracted_sum = fhe.add(extracted_a, extracted_b);
    int64_t extracted_dec = fhe.decrypt(extracted_sum);
    cout << "Extracted add: " << extracted_dec << " (expect 300)" << endl;
    
    return 0;
}
