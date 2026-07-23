// FEmmG-FHE Core Library
// φ-extension ring: R[X]/(X²-X-1) for FHE noise management
// Author: Dan Joseph M. Fernandez / Primordial Omega Zero
// License: MIT

#pragma once
#include <openfhe.h>
#include <vector>
#include <cmath>

namespace femmg {

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct PhiElement {
    lbcrypto::Ciphertext<lbcrypto::DCRTPoly> a;  // φ-coefficient
    lbcrypto::Ciphertext<lbcrypto::DCRTPoly> b;  // ψ-coefficient
};

// Core operations
PhiElement mul_X(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x);
PhiElement div_X(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x);
PhiElement clean(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x, int ratio = 3);
PhiElement multiply(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, const PhiElement& x, const PhiElement& y);
PhiElement bootstrap(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc, 
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys,
                     const PhiElement& x, uint32_t slots);

// Utility
double decrypt_coeff(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys,
                     const lbcrypto::Ciphertext<lbcrypto::DCRTPoly>& ct, uint32_t slots);
PhiElement encrypt_element(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                           lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys,
                           double a, double b, uint32_t slots);
double get_phi_value(const PhiElement& x, lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys, uint32_t slots);
double get_psi_noise(const PhiElement& x, lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
                     lbcrypto::KeyPair<lbcrypto::DCRTPoly>& keys, uint32_t slots);

} // namespace femmg
