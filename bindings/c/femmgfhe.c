#include "femmgfhe.h"
#include "../../src/config/system_config.h"
#include "../../src/hardware/hardware_sentinel.h"
#include <cstdlib>
#include <cstring>

femmgfhe_handle_t femmgfhe_create(void) {
    SystemConfig* cfg = new SystemConfig();
    return (femmgfhe_handle_t)cfg;
}

void femmgfhe_destroy(femmgfhe_handle_t h) {
    delete (SystemConfig*)h;
}

const char* femmgfhe_version(void) {
    return "femmgFHE v25.1";
}

double femmgfhe_phi(void) { return PHI; }
double femmgfhe_psi(void) { return PSI; }
double femmgfhe_pi(void) { return PI; }

int femmgfhe_hw_max_ring_dim(void) {
    auto hw = HardwareCapabilities::detect();
    return hw.max_ring_dim;
}

int femmgfhe_hw_cpu_cores(void) {
    auto hw = HardwareCapabilities::detect();
    return hw.cpu_cores;
}

const char* femmgfhe_hw_cpu_brand(void) {
    static std::string brand;
    auto hw = HardwareCapabilities::detect();
    brand = hw.cpu_brand;
    return brand.c_str();
}

void femmgfhe_set_dev_mode(femmgfhe_handle_t h) {
    ((SystemConfig*)h)->set_dev_mode();
}

void femmgfhe_set_test_mode(femmgfhe_handle_t h) {
    ((SystemConfig*)h)->set_test_mode();
}

void femmgfhe_set_prod_mode(femmgfhe_handle_t h) {
    ((SystemConfig*)h)->set_prod_mode();
}

void femmgfhe_set_enterprise_mode(femmgfhe_handle_t h) {
    ((SystemConfig*)h)->set_enterprise_mode();
}

int femmgfhe_get_ring_dim(femmgfhe_handle_t h) {
    return ((SystemConfig*)h)->N_ring_dim;
}

int femmgfhe_get_variants(femmgfhe_handle_t h) {
    return ((SystemConfig*)h)->N_variants;
}

int femmgfhe_get_fractal_layers(femmgfhe_handle_t h) {
    return ((SystemConfig*)h)->N_fractal_layers;
}
