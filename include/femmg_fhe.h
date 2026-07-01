/*
 * FEmmg-FHE v22.0.0 — Fibonacci-Lyapunov Fully Homomorphic Encryption
 * 
 * Main entry point for the FEmmg-FHE library.
 * Include this single header to access all FHE functionality.
 * 
 * Usage: g++ -I<path-to-femmgFHE>/include -o app app.cpp -lm -lssl -lcrypto
 * 
 * Author: Dan Joseph M. Fernandez / Primordial Omega Zero
 * License: MIT
 */

#pragma once

// Core FHE engine (includes math, chaos, security transitively)
#include "femmg_operations.h"
