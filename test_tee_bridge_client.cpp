#include "src/bridge/tee_dual_gate_bridge.h"
#include <chrono>

int main() {
    SpiralIO::TEEBridgeClient client;
    auto sc = create_fhe_context(8192, 40);
    SpiralIO::TFHEContext tfhe;
    tfhe.init();
    
    std::cout << "===============================================================\n";
    std::cout << "  TEE BRIDGE CLIENT TEST\n";
    std::cout << "===============================================================\n\n";
    
    // CKKS → TFHE
    auto ckks_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto ckks_ct = sc.cc->Encrypt(sc.kp.publicKey, ckks_pt);
    
    std::cout << "CKKS → TFHE...\n";
    auto tfhe_ct = client.ckks_to_tfhe(ckks_ct);
    
    LWEPlaintext tfhe_pt;
    tfhe.cc.Decrypt(tfhe.sk, tfhe_ct, &tfhe_pt);
    bool tfhe_bit = (tfhe_pt == 1);
    std::cout << "TFHE bit: " << tfhe_bit << " (expect 1)\n\n";
    
    // TFHE → CKKS
    auto tfhe_input = tfhe.encrypt_bool(true);
    std::cout << "TFHE → CKKS...\n";
    auto ckks_out = client.tfhe_to_ckks(tfhe_input);
    
    Plaintext ckks_out_pt;
    sc.cc->Decrypt(sc.kp.secretKey, ckks_out, &ckks_out_pt);
    double ckks_val = ckks_out_pt->GetCKKSPackedValue()[0].real();
    std::cout << "CKKS value: " << ckks_val << " (expect 1.0)\n\n";
    
    std::cout << "Status: " << (tfhe_bit && ckks_val > 0.5 ? "PASS" : "FAIL") << "\n";
    return (tfhe_bit && ckks_val > 0.5) ? 0 : 1;
}
