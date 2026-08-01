mod femmgfhe;

fn main() {
    println!("╔══════════════════════════════════════════════════════╗");
    println!("║  femmgFHE — Rust Bindings Test                      ║");
    println!("╚══════════════════════════════════════════════════════╝");
    println!();
    
    println!("Version: {}", femmgfhe::FemmgFHE::version());
    println!("φ={:.6} ψ={:.6} π={:.6}", 
             femmgfhe::FemmgFHE::phi(), 
             femmgfhe::FemmgFHE::psi(), 
             femmgfhe::FemmgFHE::pi());
    println!();
    
    println!("═══ Hardware ═══");
    println!("CPU: {}", femmgfhe::FemmgFHE::cpu_brand());
    println!("Cores: {}", femmgfhe::FemmgFHE::cpu_cores());
    println!("Max RingDim: {}", femmgfhe::FemmgFHE::max_ring_dim());
    println!();
    
    let cfg = femmgfhe::FemmgFHE::new();
    
    println!("═══ Config Modes ═══");
    cfg.set_dev_mode();
    println!("dev:        RingDim={}", cfg.ring_dim());
    cfg.set_prod_mode();
    println!("prod:       RingDim={}", cfg.ring_dim());
    
    println!();
    println!("✅ Rust bindings — WORKING!");
}
