// ================================================================
// SPIRAL iO — PackedDecryptLayer (NO duplicate includes)
// ================================================================
// Uses DecryptLayer from spiral_fhe_io_final.h (already included).
// ================================================================

#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include "openfhe.h"
#include "src/fhe/spiral_fhe_io_final.h"  // Has DecryptLayer, SecureContext, etc.
#include "src/config/gf_n_encryption.h"  // GF-N encryption

using namespace lbcrypto;

namespace SpiralIO {

class PackedDecryptLayer {
private:
    CryptoContext<DCRTPoly> cc;
    PrivateKey<DCRTPoly> secretKey;
    PublicKey<DCRTPoly> publicKey;
    GFNEncryption gf_n;
    
    int batch_size;
    int N_gf_layers;
    int bootstrap_count;
    double master_seed;
    
    std::vector<double> internal_decrypt_packed(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(secretKey, ct, &pt);
        auto cv = pt->GetCKKSPackedValue();
        std::vector<double> values(cv.size(), 0.0);
        for (size_t i = 0; i < cv.size(); i++) {
            values[i] = cv[i].real();
        }
        return values;
    }
    
    double gf_n_round_trip(double wire_val, int slot_idx) {
        auto gf_ct = gf_n.encrypt(wire_val);
        double recovered = gf_n.decrypt(gf_ct);
        return (recovered > 0.5) ? 1.0 : (recovered < -0.5) ? -1.0 : 0.0;
    }
    
public:
    void init(SecureContext& sc, double seed = 42.0, int gf_layers = 5) {
        cc = sc.cc;
        secretKey = sc.kp.secretKey;
        publicKey = sc.kp.publicKey;
        batch_size = cc->GetRingDimension() / 16;
        N_gf_layers = gf_layers;
        master_seed = seed;
        bootstrap_count = 0;
        gf_n.init_enterprise(seed, gf_layers);
    }
    
    Ciphertext<DCRTPoly> bootstrap_packed(const Ciphertext<DCRTPoly>& encrypted_wires) {
        bootstrap_count++;
        
        auto wire_values = internal_decrypt_packed(encrypted_wires);
        
        std::vector<double> refreshed_wires(wire_values.size(), 0.0);
        for (size_t i = 0; i < wire_values.size(); i++) {
            refreshed_wires[i] = gf_n_round_trip(wire_values[i], i);
        }
        
        double new_seed = std::fmod(master_seed * PHI + bootstrap_count * 0.618, 1.0);
        master_seed = new_seed;
        gf_n.init_enterprise(new_seed, N_gf_layers);
        
        auto fresh_pt = cc->MakeCKKSPackedPlaintext(refreshed_wires);
        return cc->Encrypt(publicKey, fresh_pt);
    }
    
    int get_bootstrap_count() const { return bootstrap_count; }
};

} // namespace SpiralIO
