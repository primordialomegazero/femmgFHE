// ================================================================
// SPIRAL BRIDGE — TEE DualGate Bridge (FIXED Serialization)
// ================================================================
// Uses Serial::SerializeToString / DeserializeFromString (WORKING).
// 44.8MB per ciphertext — socket can handle, just needs timeout.
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
#include <csignal>
#include "src/fhe/spiral_fhe_io_final.h"
#include "src/io/spiral_io_tfhe.h"
#include "src/bridge/dual_gate_bridge_fixed.h"
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

using namespace lbcrypto;

// CEREAL REGISTRATIONS (WORKING)
CEREAL_REGISTER_TYPE(lbcrypto::CryptoParametersCKKSRNS);
CEREAL_REGISTER_TYPE(lbcrypto::CryptoParametersBase<lbcrypto::DCRTPoly>);
CEREAL_REGISTER_TYPE(lbcrypto::SchemeCKKSRNS);
CEREAL_REGISTER_TYPE(lbcrypto::SchemeBase<lbcrypto::DCRTPoly>);

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
        signal(SIGPIPE, SIG_IGN);  // Don't die on client disconnect
        
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
    
    // Robust receive all bytes
    bool recv_all(int fd, void* buf, size_t n) {
        size_t offset = 0;
        while (offset < n) {
            ssize_t r = recv(fd, (char*)buf + offset, n - offset, 0);
            if (r <= 0) return false;
            offset += r;
        }
        return true;
    }
    
    bool send_all(int fd, const void* buf, size_t n) {
        size_t offset = 0;
        while (offset < n) {
            ssize_t r = send(fd, (const char*)buf + offset, n - offset, 0);
            if (r <= 0) return false;
            offset += r;
        }
        return true;
    }
    
    void serve() {
        while (running) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;
            
            uint8_t req_type;
            if (!recv_all(client_fd, &req_type, sizeof(req_type))) {
                close(client_fd);
                continue;
            }
            
            uint32_t len = 0;
            if (!recv_all(client_fd, &len, sizeof(len))) {
                close(client_fd);
                continue;
            }
            
            std::string serialized(len, '\0');
            if (!recv_all(client_fd, &serialized[0], len)) {
                close(client_fd);
                continue;
            }
            
            if (req_type == 1) {
                // CKKS → TFHE
                Ciphertext<DCRTPoly> ckks_ct;
                Serial::DeserializeFromString(ckks_ct, serialized);
                
                Plaintext pt;
                ckks_sc.cc->Decrypt(ckks_sc.kp.secretKey, ckks_ct, &pt);
                double a = pt->GetCKKSPackedValue()[0].real();
                
                DualGateFixed dg(a, 1.0 - a);
                bool bit = (dg.to_bool() > 0.5);
                
                auto tfhe_ct = tfhe_ctx.encrypt_bool(bit);
                std::string out_str = Serial::SerializeToString(tfhe_ct);
                uint32_t out_len = out_str.size();
                send_all(client_fd, &out_len, sizeof(out_len));
                send_all(client_fd, out_str.data(), out_len);
            } else {
                // TFHE → CKKS
                LWECiphertext tfhe_ct;
                Serial::DeserializeFromString(tfhe_ct, serialized);
                
                LWEPlaintext lwe_pt;
                tfhe_ctx.cc.Decrypt(tfhe_ctx.sk, tfhe_ct, &lwe_pt);
                double bit_val = (lwe_pt == 1) ? 1.0 : 0.0;
                
                DualGateFixed dg(bit_val, 1.0 - bit_val);
                double recovered = dg.to_bool();
                
                auto ckks_pt = ckks_sc.cc->MakeCKKSPackedPlaintext(
                    std::vector<double>{recovered});
                auto ckks_ct = ckks_sc.cc->Encrypt(ckks_sc.kp.publicKey, ckks_pt);
                
                std::string out_str = Serial::SerializeToString(ckks_ct);
                uint32_t out_len = out_str.size();
                send_all(client_fd, &out_len, sizeof(out_len));
                send_all(client_fd, out_str.data(), out_len);
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
    
    bool send_all(int fd, const void* buf, size_t n) {
        size_t offset = 0;
        while (offset < n) {
            ssize_t r = send(fd, (const char*)buf + offset, n - offset, 0);
            if (r <= 0) return false;
            offset += r;
        }
        return true;
    }
    
    bool recv_all(int fd, void* buf, size_t n) {
        size_t offset = 0;
        while (offset < n) {
            ssize_t r = recv(fd, (char*)buf + offset, n - offset, 0);
            if (r <= 0) return false;
            offset += r;
        }
        return true;
    }
    
    LWECiphertext ckks_to_tfhe(const Ciphertext<DCRTPoly>& ckks_ct) {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
        connect(fd, (struct sockaddr*)&addr, sizeof(addr));
        
        uint8_t req_type = 1;
        send_all(fd, &req_type, sizeof(req_type));
        
        std::string serialized = Serial::SerializeToString(ckks_ct);
        uint32_t len = serialized.size();
        send_all(fd, &len, sizeof(len));
        send_all(fd, serialized.data(), len);
        
        uint32_t out_len = 0;
        recv_all(fd, &out_len, sizeof(out_len));
        std::string out_str(out_len, '\0');
        recv_all(fd, &out_str[0], out_len);
        
        LWECiphertext result;
        Serial::DeserializeFromString(result, out_str);
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
        send_all(fd, &req_type, sizeof(req_type));
        
        std::string serialized = Serial::SerializeToString(tfhe_ct);
        uint32_t len = serialized.size();
        send_all(fd, &len, sizeof(len));
        send_all(fd, serialized.data(), len);
        
        uint32_t out_len = 0;
        recv_all(fd, &out_len, sizeof(out_len));
        std::string out_str(out_len, '\0');
        recv_all(fd, &out_str[0], out_len);
        
        Ciphertext<DCRTPoly> result;
        Serial::DeserializeFromString(result, out_str);
        close(fd);
        return result;
    }
};

} // namespace SpiralIO
