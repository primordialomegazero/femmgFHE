// ═══════════════════════════════════════════════════════════════
// femmgFHE — C API Bindings
// ═══════════════════════════════════════════════════════════════

#ifndef FEMMGFHE_C_H
#define FEMMGFHE_C_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle
typedef void* femmgfhe_handle_t;

// Lifecycle
femmgfhe_handle_t femmgfhe_create(void);
void femmgfhe_destroy(femmgfhe_handle_t h);

// Version
const char* femmgfhe_version(void);

// Constants
double femmgfhe_phi(void);
double femmgfhe_psi(void);
double femmgfhe_pi(void);

// Hardware
int femmgfhe_hw_max_ring_dim(void);
int femmgfhe_hw_cpu_cores(void);
const char* femmgfhe_hw_cpu_brand(void);

// Config
void femmgfhe_set_dev_mode(femmgfhe_handle_t h);
void femmgfhe_set_test_mode(femmgfhe_handle_t h);
void femmgfhe_set_prod_mode(femmgfhe_handle_t h);
void femmgfhe_set_enterprise_mode(femmgfhe_handle_t h);

int femmgfhe_get_ring_dim(femmgfhe_handle_t h);
int femmgfhe_get_variants(femmgfhe_handle_t h);
int femmgfhe_get_fractal_layers(femmgfhe_handle_t h);

#ifdef __cplusplus
}
#endif

#endif // FEMMGFHE_C_H
