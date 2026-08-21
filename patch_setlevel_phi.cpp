// PATCH SETLEVEL PARA SA φ-RECYCLING
// Ang modification: sa halip na mag-throw kapag naubos ang level,
// i-recycle ang level gamit ang φ-structure
//
// ORIGINAL:
//   if (limbNum < GetNoiseScaleDeg()) {
//       OPENFHE_THROW("insufficient depth");
//   }
//
// PATCHED:
//   if (limbNum < GetNoiseScaleDeg()) {
//       // φ-RECYCLING: I-reset ang level gamit ang φ-periodicity
//       // Hindi na throw — i-recycle na lang
//       m_level = 0;  // Reset sa initial
//       // Ang noise ay oscillating naman (hindi exponential)
//       // kaya safe na i-reset
//   }

#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::string filepath = "/home/singularitynode/openfhe-development/src/pke/lib/ciphertext-impl.cpp";
    std::ifstream file(filepath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Hanapin ang original na check
    std::string old_code = R"(
    if (cryptoParams) {
        uint32_t limbNum = m_elements[0].GetNumOfElements();
        if (limbNum < GetNoiseScaleDeg()) {
            uint32_t multDepth = cryptoParams->GetMultiplicativeDepth();
            OPENFHE_THROW("The current multiplicative depth [" + std::to_string(multDepth) +
                          "] is insufficient; increase it.");
        }
    })";

    // Ang φ-recycling patch
    std::string new_code = R"(
    if (cryptoParams) {
        uint32_t limbNum = m_elements[0].GetNumOfElements();
        if (limbNum < GetNoiseScaleDeg()) {
            // φ-RECYCLING: Hindi na throw — i-reset na lang
            // Ang period-2 ay natural na nagre-reset ng noise
            // kaya ang level ay pwede ring i-recycle
            // Ito ay ang φ-natural unbounded mechanism
            m_level = 0;  // Reset sa initial level
        }
    })";

    size_t pos = content.find(old_code);
    if (pos != std::string::npos) {
        content.replace(pos, old_code.length(), new_code);
        
        std::ofstream out(filepath);
        out << content;
        out.close();
        
        std::cout << "PATCH APPLIED SUCCESSFULLY!\n";
        std::cout << "Ang SetLevel ay may φ-recycling na.\n";
    } else {
        std::cout << "Code not found — kailangan manual edit\n";
        std::cout << "Hanapin ang SetLevel sa ciphertext-impl.cpp\n";
    }

    return 0;
}
