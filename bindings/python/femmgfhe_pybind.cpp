#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <iostream>
#include "../../src/config/system_config.h"
#include "../../src/metaprogramming/fractal_optimizer.h"
#include "../../src/hardware/hardware_sentinel.h"

namespace py = pybind11;

// ═══════════════════════════════════════════════════════════════
// PYTHON BINDINGS — femmgFHE Fractal iO
// ═══════════════════════════════════════════════════════════════

PYBIND11_MODULE(femmgfhe, m) {
    m.doc() = "femmgFHE — Fractal iO System with Post-Quantum Hardware Obfuscation";
    
    // ═══════════════════════════════════════════════════════════
    // SystemConfig
    // ═══════════════════════════════════════════════════════════
    py::class_<SystemConfig>(m, "SystemConfig")
        .def(py::init<>())
        .def("set_dev_mode", &SystemConfig::set_dev_mode)
        .def("set_test_mode", &SystemConfig::set_test_mode)
        .def("set_prod_mode", &SystemConfig::set_prod_mode)
        .def("set_enterprise_mode", &SystemConfig::set_enterprise_mode)
        .def("derive_all", &SystemConfig::derive_all)
        .def("report", &SystemConfig::report)
        .def("mode_string", &SystemConfig::mode_string)
        .def_readwrite("N_ring_dim", &SystemConfig::N_ring_dim)
        .def_readwrite("N_variants", &SystemConfig::N_variants)
        .def_readwrite("N_fractal_layers", &SystemConfig::N_fractal_layers)
        .def_readwrite("N_fractal_depth", &SystemConfig::N_fractal_depth)
        .def_readwrite("N_stats_samples", &SystemConfig::N_stats_samples)
        .def_readwrite("master_seed", &SystemConfig::master_seed);
    
    // ═══════════════════════════════════════════════════════════
    // Hardware Sentinel
    // ═══════════════════════════════════════════════════════════
    py::class_<HardwareCapabilities>(m, "HardwareCapabilities")
        .def_static("detect", &HardwareCapabilities::detect)
        .def("report", &HardwareCapabilities::report)
        .def_readonly("max_ring_dim", &HardwareCapabilities::max_ring_dim)
        .def_readonly("cpu_cores", &HardwareCapabilities::cpu_cores)
        .def_readonly("total_ram_gb", &HardwareCapabilities::total_ram_gb)
        .def_readonly("has_avx2", &HardwareCapabilities::has_avx2)
        .def_readonly("has_avx512", &HardwareCapabilities::has_avx512)
        .def_readonly("cpu_brand", &HardwareCapabilities::cpu_brand);
    
    // ═══════════════════════════════════════════════════════════
    // Quick Functions
    // ═══════════════════════════════════════════════════════════
    m.def("version", []() { return "femmgFHE v25.1 — Fractal iO"; });
    
    m.def("hardware_info", []() {
        auto hw = HardwareCapabilities::detect();
        return hw.report();
    });
    
    m.def("create_config", [](const std::string& mode) {
        SystemConfig cfg;
        if (mode == "dev") cfg.set_dev_mode();
        else if (mode == "test") cfg.set_test_mode();
        else if (mode == "prod") cfg.set_prod_mode();
        else if (mode == "enterprise") cfg.set_enterprise_mode();
        cfg.derive_all();
        return cfg;
    });
    
    // Constants
    m.attr("PHI") = PHI;
    m.attr("PSI") = PSI;
    m.attr("PI") = PI;
}
