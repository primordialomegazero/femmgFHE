#include <stdio.h>
#include "femmgfhe.h"

int main() {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  femmgFHE — C Bindings Test                         ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
    
    printf("Version: %s\n", femmgfhe_version());
    printf("φ=%.6f ψ=%.6f π=%.6f\n\n", femmgfhe_phi(), femmgfhe_psi(), femmgfhe_pi());
    
    printf("═══ Hardware ═══\n");
    printf("CPU: %s\n", femmgfhe_hw_cpu_brand());
    printf("Cores: %d\n", femmgfhe_hw_cpu_cores());
    printf("Max RingDim: %d\n\n", femmgfhe_hw_max_ring_dim());
    
    femmgfhe_handle_t h = femmgfhe_create();
    
    printf("═══ Config Modes ═══\n");
    femmgfhe_set_dev_mode(h);
    printf("dev:        RingDim=%d\n", femmgfhe_get_ring_dim(h));
    
    femmgfhe_set_prod_mode(h);
    printf("prod:       RingDim=%d\n", femmgfhe_get_ring_dim(h));
    
    femmgfhe_destroy(h);
    printf("\n✅ C bindings — WORKING!\n");
    
    return 0;
}
