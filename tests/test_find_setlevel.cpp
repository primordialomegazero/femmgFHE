// FIND SetLevel IMPLEMENTATION
#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIND SetLevel IMPLEMENTATION\n";
    std::cout << "========================================\n\n";

    const char* paths[] = {
        "/usr/local/openfhe/include/openfhe/pke/ciphertext-impl.h",
        "/usr/local/openfhe/include/openfhe/pke/ciphertext.h",
        "/usr/local/openfhe/include/openfhe/pke/ciphertext-impl.cpp"
    };

    for (const char* path : paths) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::string line;
            int line_num = 0;
            while (std::getline(file, line)) {
                line_num++;
                if (line.find("SetLevel") != std::string::npos ||
                    line.find("GetLevel") != std::string::npos ||
                    line.find("insufficient") != std::string::npos ||
                    line.find("throw") != std::string::npos) {
                    std::cout << "  " << path << ":" << line_num << ": ";
                    // I-print lang ang first 80 chars
                    std::cout << line.substr(0, 80) << "\n";
                }
            }
            file.close();
        } else {
            std::cout << "  " << path << " — NOT FOUND\n";
        }
    }

    // Hanapin din sa source directory
    std::ifstream cpp_file("/home/singularitynode/femmgFHE/openfhe-development/src/pke/lib/ciphertext-impl.cpp");
    if (cpp_file.is_open()) {
        std::string line;
        int line_num = 0;
        while (std::getline(cpp_file, line)) {
            line_num++;
            if (line.find("SetLevel") != std::string::npos ||
                line.find("insufficient") != std::string::npos) {
                std::cout << "  ciphertext-impl.cpp:" << line_num << ": ";
                std::cout << line.substr(0, 80) << "\n";
            }
        }
    }

    return 0;
}
