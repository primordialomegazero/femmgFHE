// ================================================================
// SPIRAL BRIDGE — TEE DualGate Bridge (with DualGate Integration)
// ================================================================
// Uses DualGateFixed for golden projection during conversion.
// SKs isolated in trusted process.
// ================================================================

#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <atomic>
#include "src/fhe/spiral_fhe_io_final.h"
#include "src/io/spiral_io_tfhe.h"
#include "src/bridge/dual_gate_bridge_fixed.h"

using namespace lbcrypto;

namespace SpiralIO {

class TEEBridgeServer {
private:
    SecureContext ckks_sc;
    TFHEContext tfhe_ctx;
    int server_fd;
    std::string socket_path;
    std::atomic<bool> running;
    
public:
    TEEBridgeServer(const std::string& path = "/tmp/fhe_io_bridge.sock")
        : socket_path(path), server_fd(-1), running(false) {}
    
    bool start() {
        ckks_sc = create_fhe_context(8192, 40);
        tfhe_ctx.init();
        
        unlink(socket_path.c_str());
        server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_fd < 0) return false;
        
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
        
        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(server_fd);
            return false;
        }
        if (listen(server_fd, 5) < 0) {
            close(server_fd);
            return false;
        }
        
        running = true;
        std::cout << "[TEE BRIDGE] Ready at " << socket_path << "\n";
        return true;
    }
    
    void serve() {
        while (running) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;
            
            uint8_t req_type;
            recv(client_fd, &req_type, sizeof(req_type), 0);
            uint32_t len = 0;
            recv(client_fd, &len, sizeof(len), 0);
            std::string serialized(len, '\0');
            recv(client_fd, &serialized[0], len, 0);
            
            if (req_type == 1) {
                // CKKS → TFHE with DualGate
                Ciphertext<DCRTPoly> ckks_ct;
                std::stringstream ss(serialized);
                Serial::Deserialize(ckks_ct, ss, SerType::BINARY);
                
                Plaintext pt;
                ckks_sc.cc->Decrypt(ckks_sc.kp.secretKey, ckks_ct, &pt);
                double a = pt->GetCKKSPackedValue()[0].real();
                
                // DUALGATE GOLDEN PROJECTION
                DualGateFixed dg(a, 1.0 - a);
                bool bit = (dg.to_bool() > 0.5);
                
                auto tfhe_ct = tfhe_ctx.encrypt_bool(bit);
                std::stringstream out_ss;
                Serial::Serialize(tfhe_ct, out_ss, SerType::BINARY);
                std::string out_str = out_ss.str();
                uint32_t out_len = out_str.size();
                send(client_fd, &out_len, sizeof(out_len), 0);
                send(client_fd, out_str.data(), out_len, 0);
            } else {
                // TFHE → CKKS with DualGate
                LWECiphertext tfhe_ct;
                std::stringstream ss(serialized);
                Serial::Deserialize(tfhe_ct, ss, SerType::BINARY);
                
                LWEPlaintext lwe_pt;
                tfhe_ctx.cc.Decrypt(tfhe_ctx.sk, tfhe_ct, &lwe_pt);
                double bit_val = (lwe_pt == 1) ? 1.0 : 0.0;
                
                // DUALGATE GOLDEN PROJECTION
                DualGateFixed dg(bit_val, 1.0 - bit_val);
                double recovered = dg.to_bool();
                
                auto ckks_pt = ckks_sc.cc->MakeCKKSPackedPlaintext(
                    std::vector<double>{recovered});
                auto ckks_ct = ckks_sc.cc->Encrypt(ckks_sc.kp.publicKey, ckks_pt);
                
                std::stringstream out_ss;
                Serial::Serialize(ckks_ct, out_ss, SerType::BINARY);
                std::string out_str = out_ss.str();
                uint32_t out_len = out_str.size();
                send(client_fd, &out_len, sizeof(out_len), 0);
                send(client_fd, out_str.data(), out_len, 0);
            }
            
            close(client_fd);
        }
    }
    
    void stop() {
        running = false;
        if (server_fd >= 0) close(server_fd);
        unlink(socket_path.c_str());
    }
};

class TEEBridgeClient {
private:
    std::string socket_path;
    
public:
    TEEBridgeClient(const std::string& path = "/tmp/fhe_io_bridge.sock")
        : socket_path(path) {}
    
    LWECiphertext ckks_to_tfhe(const Ciphertext<DCRTPoly>& ckks_ct) {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
        connect(fd, (struct sockaddr*)&addr, sizeof(addr));
        
        uint8_t req_type = 1;
        send(fd, &req_type, sizeof(req_type), 0);
        
        std::stringstream ss;
        Serial::Serialize(ckks_ct, ss, SerType::BINARY);
        std::string serialized = ss.str();
        uint32_t len = serialized.size();
        send(fd, &len, sizeof(len), 0);
        send(fd, serialized.data(), len, 0);
        
        uint32_t out_len = 0;
        recv(fd, &out_len, sizeof(out_len), 0);
        std::string out_str(out_len, '\0');
        recv(fd, &out_str[0], out_len, 0);
        
        LWECiphertext result;
        std::stringstream in_ss(out_str);
        Serial::Deserialize(result, in_ss, SerType::BINARY);
        close(fd);
        return result;
    }
    
    Ciphertext<DCRTPoly> tfhe_to_ckks(const LWECiphertext& tfhe_ct) {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
        connect(fd, (struct sockaddr*)&addr, sizeof(addr));
        
        uint8_t req_type = 2;
        send(fd, &req_type, sizeof(req_type), 0);
        
        std::stringstream ss;
        Serial::Serialize(tfhe_ct, ss, SerType::BINARY);
        std::string serialized = ss.str();
        uint32_t len = serialized.size();
        send(fd, &len, sizeof(len), 0);
        send(fd, serialized.data(), len, 0);
        
        uint32_t out_len = 0;
        recv(fd, &out_len, sizeof(out_len), 0);
        std::string out_str(out_len, '\0');
        recv(fd, &out_str[0], out_len, 0);
        
        Ciphertext<DCRTPoly> result;
        std::stringstream in_ss(out_str);
        Serial::Deserialize(result, in_ss, SerType::BINARY);
        close(fd);
        return result;
    }
};

} // namespace SpiralIO
