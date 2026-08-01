#include <iostream>
#include <string>
#include <chrono>
#include "../../src/database/harmonized_obfuscation.h"

void print_usage() {
    std::cout << "Spiral Fractal iO -- CLI Tool\n\n";
    std::cout << "Usage:\n";
    std::cout << "  femmgfhe encrypt <plaintext> <passphrase>\n";
    std::cout << "  femmgfhe decrypt <ciphertext> <passphrase>\n";
    std::cout << "  femmgfhe benchmark\n";
    std::cout << "  femmgfhe export-key <filepath>\n";
    std::cout << "  femmgfhe import-key <filepath>\n";
    std::cout << "  femmgfhe status\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { print_usage(); return 1; }
    
    std::string cmd = argv[1];
    
    if (cmd == "encrypt" && argc >= 4) {
        HarmonizedObfuscation h;
        h.init("cli.db", argv[3]);
        std::cout << h.encrypt(argv[2]) << "\n";
    }
    else if (cmd == "decrypt" && argc >= 4) {
        HarmonizedObfuscation h;
        h.init("cli.db", argv[3]);
        std::cout << h.decrypt(argv[2]) << "\n";
    }
    else if (cmd == "benchmark") {
        HarmonizedObfuscation h;
        h.init("bench.db", "bench-pass");
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < 1000; i++) {
            h.pg_put("test", "key"+std::to_string(i), "value"+std::to_string(i));
        }
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "1000 encrypted writes: " << ms << " ms (" << (1000/ms*1000) << " ops/sec)\n";
        system("rm -f bench.db*");
    }
    else if (cmd == "export-key" && argc >= 3) {
        HarmonizedObfuscation h;
        h.init("cli.db");
        h.save_key_to_file(argv[2]);
        std::cout << "Key exported to: " << argv[2] << "\n";
    }
    else if (cmd == "import-key" && argc >= 3) {
        HarmonizedObfuscation h;
        h.init("cli.db");
        h.load_key_from_file(argv[2]);
        std::cout << "Key imported from: " << argv[2] << "\n";
    }
    else if (cmd == "status") {
        HarmonizedObfuscation h;
        h.init("cli.db", "status-pass");
        std::cout << h.status() << "\n";
    }
    else { print_usage(); }
    
    system("rm -f cli.db*");
    return 0;
}
