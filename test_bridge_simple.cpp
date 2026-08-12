// SIMPLE BRIDGE TEST: No sockets, direct conversion
#include "src/bridge/dual_gate_bridge_fixed.h"
#include "src/fhe/spiral_fhe_io_final.h"
#include "src/io/spiral_io_tfhe.h"

int main() {
    auto sc = create_fhe_context(8192, 40);
    SpiralIO::TFHEContext tfhe;
    tfhe.init();
    
    // Direct conversion (no TEE, for testing only)
    auto ckks_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto ckks_ct = sc.cc->Encrypt(sc.kp.publicKey, ckks_pt);
    
    Plaintext pt;
    sc.cc->Decrypt(sc.kp.secretKey, ckks_ct, &pt);
    double val = pt->GetCKKSPackedValue()[0].real();
    
    SpiralIO::DualGateFixed dg(val, 1.0 - val);
    bool bit = (dg.to_bool() > 0.5);
    
    auto tfhe_ct = tfhe.encrypt_bool(bit);
    LWEPlaintext lwe_pt;
    tfhe.cc.Decrypt(tfhe.sk, tfhe_ct, &lwe_pt);
    bool recovered = (lwe_pt == 1);
    
    std::cout << "CKKS val: " << val << "\n";
    std::cout << "DualGate to_bool: " << dg.to_bool() << "\n";
    std::cout << "TFHE bit: " << recovered << " (expect 1)\n";
    std::cout << "Status: " << (recovered ? "PASS" : "FAIL") << "\n";
    
    return recovered ? 0 : 1;
}
