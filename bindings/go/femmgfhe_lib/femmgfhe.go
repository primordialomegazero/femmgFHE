package femmgfhe_lib

/*
#cgo CFLAGS: -I${SRCDIR}/../..
#cgo LDFLAGS: -L${SRCDIR}/../../bin -lfemmgfhe_c -lstdc++ -lm

#include "../c/femmgfhe.h"
*/
import "C"
import "runtime"

type SystemConfig struct {
	handle C.femmgfhe_handle_t
}

func New() *SystemConfig {
	cfg := &SystemConfig{handle: C.femmgfhe_create()}
	runtime.SetFinalizer(cfg, func(c *SystemConfig) { C.femmgfhe_destroy(c.handle) })
	return cfg
}

func Version() string     { return C.GoString(C.femmgfhe_version()) }
func Phi() float64        { return float64(C.femmgfhe_phi()) }
func Psi() float64        { return float64(C.femmgfhe_psi()) }
func Pi() float64         { return float64(C.femmgfhe_pi()) }
func MaxRingDim() int     { return int(C.femmgfhe_hw_max_ring_dim()) }
func CPUCores() int       { return int(C.femmgfhe_hw_cpu_cores()) }
func CPUBrand() string    { return C.GoString(C.femmgfhe_hw_cpu_brand()) }

func (c *SystemConfig) SetDevMode()        { C.femmgfhe_set_dev_mode(c.handle) }
func (c *SystemConfig) SetTestMode()       { C.femmgfhe_set_test_mode(c.handle) }
func (c *SystemConfig) SetProdMode()       { C.femmgfhe_set_prod_mode(c.handle) }
func (c *SystemConfig) SetEnterpriseMode() { C.femmgfhe_set_enterprise_mode(c.handle) }
func (c *SystemConfig) RingDim() int       { return int(C.femmgfhe_get_ring_dim(c.handle)) }
func (c *SystemConfig) Variants() int      { return int(C.femmgfhe_get_variants(c.handle)) }
func (c *SystemConfig) FractalLayers() int { return int(C.femmgfhe_get_fractal_layers(c.handle)) }
