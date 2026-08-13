#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <csignal>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <atomic>

#include "src/io/spiral_io_tfhe.h"
#include "src/io/tfhe_shared_keys.h"
#include "src/config/gf_n_encryption.h"

using namespace lbcrypto;

namespace SpiralIO {

// ================================================================
// BRIDGE BOOTSTRAP SERVER (Trusted Process / TEE Simulation)
// ================================================================
// Uses shared TFHE keys loaded from file.
// Holds GF-N for optional MAC.
// ================================================================
class GFNBootstrapServer {
private:
    TFHEContext tfhe;
    GFNEncryption gf_n;
    int server_fd;
    std::string socket_path;
    std::atomic<bool> running;
    
public:
    GFNBootstrapServer(const std::string& path = "/tmp/gf_n_bootstrap.sock")
        : socket_path(path), server_fd(-1), running(false) {}

    bool start() {
        // Load shared keys
        TFHESharedKeys::load_keys("/tmp/fhe_io_keys.bin", tfhe);
        gf_n.init_enterprise(42.0, 5);

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
        std::cout << "[BRIDGE BOOTSTRAP SERVER] Ready at " << socket_path << "\n";
        return true;
    }

    void serve() {
        signal(SIGPIPE, SIG_IGN);
        while (running) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;

            uint32_t len = 0;
            if (!recv_all(client_fd, &len, sizeof(len))) { close(client_fd); continue; }
            std::string serialized(len, '\0');
            if (!recv_all(client_fd, &serialized[0], len)) { close(client_fd); continue; }

            LWECiphertext ct;
            std::stringstream ss(serialized);
            Serial::Deserialize(ct, ss, SerType::BINARY);

            LWEPlaintext lwe_pt;
            tfhe.cc.Decrypt(tfhe.sk, ct, &lwe_pt);
            bool bit = (lwe_pt == 1);

            // GF-N MAC (no decrypt)
            double gf_in = bit ? 1.0 : 0.0;
            auto gf_ct = gf_n.encrypt(gf_in);

            auto fresh_ct = tfhe.encrypt_bool(bit);

            std::stringstream out_ss;
            Serial::Serialize(fresh_ct, out_ss, SerType::BINARY);
            std::string out_str = out_ss.str();
            uint32_t out_len = out_str.size();

            if (!send_all(client_fd, &out_len, sizeof(out_len))) { close(client_fd); continue; }
            if (!send_all(client_fd, out_str.data(), out_len)) { close(client_fd); continue; }

            close(client_fd);
        }
    }

    void stop() {
        running = false;
        if (server_fd >= 0) close(server_fd);
        unlink(socket_path.c_str());
    }

private:
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
};

// ================================================================
// CLIENT SIDE (Untrusted Server)
// ================================================================
class GFNBootstrapClient {
private:
    std::string socket_path;

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

public:
    GFNBootstrapClient(const std::string& path = "/tmp/gf_n_bootstrap.sock")
        : socket_path(path) {}

    LWECiphertext refresh(const LWECiphertext& ct) {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("Cannot connect to bootstrap server");
        }

        std::stringstream ss;
        Serial::Serialize(ct, ss, SerType::BINARY);
        std::string serialized = ss.str();
        uint32_t len = serialized.size();

        if (!send_all(fd, &len, sizeof(len))) { close(fd); throw std::runtime_error("send len failed"); }
        if (!send_all(fd, serialized.data(), len)) { close(fd); throw std::runtime_error("send data failed"); }

        uint32_t out_len = 0;
        if (!recv_all(fd, &out_len, sizeof(out_len))) { close(fd); throw std::runtime_error("recv len failed"); }
        std::string out_str(out_len, '\0');
        if (!recv_all(fd, &out_str[0], out_len)) { close(fd); throw std::runtime_error("recv data failed"); }

        LWECiphertext result;
        std::stringstream in_ss(out_str);
        Serial::Deserialize(result, in_ss, SerType::BINARY);

        close(fd);
        return result;
    }
};

} // namespace SpiralIO
