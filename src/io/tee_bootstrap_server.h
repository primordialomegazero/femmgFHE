// ================================================================
// SPIRAL iO — TEE Bootstrap Server (FIXED: No duplicate includes)
// ================================================================
// Uses ONLY spiral_fhe_io_final.h (which already has DecryptLayer)
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
#include "openfhe.h"
#include "src/fhe/spiral_fhe_io_final.h"  // Has DecryptLayer, SecureContext, etc.
#include "src/io/packed_decrypt_layer.h"  // Has PackedDecryptLayer

using namespace lbcrypto;

namespace SpiralIO {

class TEEBootstrapServer {
private:
    SecureContext sc;
    PackedDecryptLayer pdl;
    int server_fd;
    std::string socket_path;
    std::atomic<bool> running;
    
public:
    TEEBootstrapServer(const std::string& path = "/tmp/fhe_tee.sock")
        : socket_path(path), server_fd(-1), running(false) {}
    
    bool start() {
        // Initialize FHE + GF-N INSIDE trusted process
        sc = create_fhe_context(8192, 40);
        pdl.init(sc, 42.0, 5);
        
        // Create Unix socket
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
        std::cout << "[TEE] Bootstrap server ready at " << socket_path << "\n";
        return true;
    }
    
    void serve() {
        while (running) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;
            
            // Read serialized ciphertext
            uint32_t len = 0;
            ssize_t n = recv(client_fd, &len, sizeof(len), 0);
            if (n != sizeof(len)) {
                close(client_fd);
                continue;
            }
            
            std::string serialized(len, '\0');
            n = recv(client_fd, &serialized[0], len, 0);
            if (n != (ssize_t)len) {
                close(client_fd);
                continue;
            }
            
            // Deserialize
            Ciphertext<DCRTPoly> ct;
            std::stringstream ss(serialized);
            Serial::Deserialize(ct, ss, SerType::BINARY);
            
            // BOOTSTRAP INSIDE TEE (SK + GF-N isolated)
            auto ct_fresh = pdl.bootstrap_packed(ct);
            
            // Serialize fresh ciphertext
            std::stringstream out_ss;
            Serial::Serialize(ct_fresh, out_ss, SerType::BINARY);
            std::string out_str = out_ss.str();
            uint32_t out_len = out_str.size();
            
            send(client_fd, &out_len, sizeof(out_len), 0);
            send(client_fd, out_str.data(), out_len, 0);
            
            close(client_fd);
        }
    }
    
    void stop() {
        running = false;
        if (server_fd >= 0) close(server_fd);
        unlink(socket_path.c_str());
    }
};

// TEE Client (untrusted server side)
class TEEClient {
private:
    std::string socket_path;
    
public:
    TEEClient(const std::string& path = "/tmp/fhe_tee.sock")
        : socket_path(path) {}
    
    bool bootstrap(const Ciphertext<DCRTPoly>& ct_in,
                   Ciphertext<DCRTPoly>& ct_out) {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) return false;
        
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
        
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(fd);
            return false;
        }
        
        std::stringstream ss;
        Serial::Serialize(ct_in, ss, SerType::BINARY);
        std::string serialized = ss.str();
        uint32_t len = serialized.size();
        send(fd, &len, sizeof(len), 0);
        send(fd, serialized.data(), len, 0);
        
        uint32_t out_len = 0;
        recv(fd, &out_len, sizeof(out_len), 0);
        std::string out_str(out_len, '\0');
        recv(fd, &out_str[0], out_len, 0);
        
        std::stringstream in_ss(out_str);
        Serial::Deserialize(ct_out, in_ss, SerType::BINARY);
        
        close(fd);
        return true;
    }
};

} // namespace SpiralIO
