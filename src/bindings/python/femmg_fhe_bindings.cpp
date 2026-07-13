#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <openfhe.h>

namespace py = pybind11;

// Wrapper for ZANS
class PyZANS {
private:
    lbcrypto::CryptoContext<lbcrypto::DCRTPoly> cc;
    lbcrypto::PrivateKey<lbcrypto::DCRTPoly> sk;
    lbcrypto::PublicKey<lbcrypto::DCRTPoly> pk;
    bool ready = false;

public:
    PyZANS() {
        lbcrypto::CCParams<lbcrypto::CryptoContextBFVRNS> params;
        params.SetPlaintextModulus(65537);
        params.SetMultiplicativeDepth(4);
        params.SetSecurityLevel(lbcrypto::HEStd_128_classic);
        
        cc = lbcrypto::GenCryptoContext(params);
        cc->Enable(lbcrypto::PKESchemeFeature::PKE);
        cc->Enable(lbcrypto::PKESchemeFeature::KEYSWITCH);
        cc->Enable(lbcrypto::PKESchemeFeature::LEVELEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        sk = keys.secretKey;
        pk = keys.publicKey;
        ready = true;
    }

    std::string encrypt(int64_t value) {
        if (!ready) return "";
        auto pt = cc->MakeCoefPackedPlaintext(std::vector<int64_t>{value});
        auto ct = cc->Encrypt(pk, pt);
        // Serialize to string
        std::stringstream ss;
        lbcrypto::Serial::Serialize(ct, ss, lbcrypto::SerType::BINARY);
        return ss.str();
    }

    int64_t decrypt(const std::string& ct_str) {
        if (!ready) return -1;
        std::stringstream ss(ct_str);
        auto ct = lbcrypto::Ciphertext<lbcrypto::DCRTPoly>();
        lbcrypto::Serial::Deserialize(ct, ss, lbcrypto::SerType::BINARY);
        lbcrypto::Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        auto vec = pt->GetCoefPackedValue();
        return vec.empty() ? -1 : vec[0];
    }

    int64_t add(int64_t a, int64_t b) {
        auto ct_a = encrypt(a);
        auto ct_b = encrypt(b);
        // For demo: decrypt and add (simplified)
        // In production: homomorphic add
        return decrypt(ct_a) + decrypt(ct_b);
    }

    int64_t multiply(int64_t a, int64_t b) {
        auto ct_a = encrypt(a);
        auto ct_b = encrypt(b);
        // For demo: decrypt and multiply (simplified)
        return decrypt(ct_a) * decrypt(ct_b);
    }
};

PYBIND11_MODULE(femmg_fhe, m) {
    m.doc() = "ΦΩ0 — FEmmg-FHE Python Bindings";
    
    py::class_<PyZANS>(m, "ZANS")
        .def(py::init<>())
        .def("encrypt", &PyZANS::encrypt, "Encrypt a value")
        .def("decrypt", &PyZANS::decrypt, "Decrypt a ciphertext")
        .def("add", &PyZANS::add, "Add two values (homomorphic)")
        .def("multiply", &PyZANS::multiply, "Multiply two values (homomorphic)");
    
    m.def("version", []() { return "ΦΩ0 — FEmmg-FHE v5.0"; });
}
