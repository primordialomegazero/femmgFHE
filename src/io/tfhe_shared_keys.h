#pragma once
#include "spiral_io_tfhe.h"
#include "../crypto/golden_prng.h"
#include <fstream>

namespace SpiralIO {

// ================================================================
// TFHE SHARED KEYS — Save/Load secret key only
// ================================================================
// In this simulation, both server and client use the same secret key
// for encryption and decryption. The client in a real TEE would only
// have public evaluation keys, but for testing we share the secret.
// ================================================================
class TFHESharedKeys {
public:
    static void save_keys(const std::string& filename, const TFHEContext& ctx) {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) throw std::runtime_error("Cannot open key file for writing");

        std::stringstream ss;
        Serial::Serialize(ctx.sk, ss, SerType::BINARY);
        std::string sk_str = ss.str();
        uint32_t sk_len = sk_str.size();

        ofs.write(reinterpret_cast<const char*>(&sk_len), sizeof(sk_len));
        ofs.write(sk_str.data(), sk_len);
    }

    static void load_keys(const std::string& filename, TFHEContext& ctx) {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) throw std::runtime_error("Cannot open key file for reading");

        uint32_t sk_len = 0;
        ifs.read(reinterpret_cast<char*>(&sk_len), sizeof(sk_len));
        std::string sk_str(sk_len, '\0');
        ifs.read(&sk_str[0], sk_len);

        std::stringstream ss(sk_str);
        Serial::Deserialize(ctx.sk, ss, SerType::BINARY);
    }
};

} // namespace SpiralIO
