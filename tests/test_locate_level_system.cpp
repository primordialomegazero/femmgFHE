// LOCATE OPENFHE LEVEL SYSTEM
// Hanapin kung saan naka-implement ang depth check
// para malaman kung paano i-modify

#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::cout << "========================================\n";
    std::cout << "  LOCATE LEVEL SYSTEM\n";
    std::cout << "========================================\n\n";

    // Hanapin ang SetLevel sa OpenFHE headers
    const char* paths[] = {
        "/usr/local/openfhe/include/openfhe/pke/ciphertext-impl.h",
        "/usr/local/openfhe/include/openfhe/core/math/dcrtpoly.h",
        "/usr/local/openfhe/include/openfhe/pke/cryptocontext.h"
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
                    line.find("multiplicative depth") != std::string::npos ||
                    line.find("insufficient") != std::string::npos) {
                    std::cout << "  " << path << ":" << line_num << ": " << line << "\n";
                }
            }
            file.close();
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  NEXT: I-modify ang SetLevel para sa\n";
    std::cout << "  φ-recycling sa halip na throw error\n";
    std::cout << "========================================\n";

    return 0;
}
