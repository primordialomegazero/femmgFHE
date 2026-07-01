/*
 * FEmmg-FHE v22.0.0 — Fibonacci-Lyapunov Fully Homomorphic Encryption
 * 
 * Main entry point for the FEmmg-FHE library.
 * Include this single header to access all FHE functionality.
 * 
 * Author: Dan Joseph M. Fernandez / Primordial Omega Zero
 * License: MIT
 */

#pragma once

// Mathematical constants
#include "../src/math/phi_constants.h"

// Core FHE engine
#include "../src/core/banach_engine.h"
#include "../src/core/femmg_operations.h"

// Chaos engines (CTU v4)
#include "../src/chaos/golden_chaos.h"
#include "../src/chaos/lyapunov_core.h"

// Security layers
#include "../src/security/blackhole.h"
#include "../src/security/blackhole_history.h"
#include "../src/security/antimatter.h"
#include "../src/security/guardian.h"
#include "../src/security/zkp_fractal.h"
#include "../src/security/zkp_pqc.h"

// Post-Quantum KEM
#include "../src/kem/phi_parallel_kem.h"

// Data persistence
#include "../src/storage/spiral_db_lite.h"

// Mathematical utilities
#include "../src/math/riemann_zeta.h"
#include "../src/math/riemann_zeros.h"
#include "../src/math/riemann_deep.h"
