package main

import (
	"fmt"
)

func main() {
	fmt.Println("╔══════════════════════════════════════════════════════╗")
	fmt.Println("║  femmgFHE — Go Bindings Test                        ║")
	fmt.Println("╚══════════════════════════════════════════════════════╝")
	fmt.Println()
	
	fmt.Printf("Version: %s\n", Version())
	fmt.Printf("φ=%.6f ψ=%.6f π=%.6f\n\n", Phi(), Psi(), Pi())
	
	fmt.Println("═══ Hardware ═══")
	fmt.Printf("CPU: %s\n", CPUBrand())
	fmt.Printf("Cores: %d\n", CPUCores())
	fmt.Printf("Max RingDim: %d\n\n", MaxRingDim())
	
	cfg := New()
	
	fmt.Println("═══ Config Modes ═══")
	cfg.SetDevMode()
	fmt.Printf("dev:        RingDim=%d\n", cfg.RingDim())
	cfg.SetProdMode()
	fmt.Printf("prod:       RingDim=%d\n", cfg.RingDim())
	
	fmt.Println("\n✅ Go bindings — WORKING!")
}
