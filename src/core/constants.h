#pragma once
#ifndef SPIRAL_CORE_CONSTANTS_H
#define SPIRAL_CORE_CONSTANTS_H

#include <cmath>
#include <cstdint>

// Core mathematical constants
#ifndef SPIRAL_PHI_DEFINED
#define SPIRAL_PHI_DEFINED
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr double PI = 3.14159265358979323846;
constexpr double EULER = 2.71828182845904523536;
constexpr double CASSINI_THRESHOLD = 0.1;
#endif

// FHE defaults
constexpr uint32_t DEFAULT_RING_DIM = 8192;
constexpr uint32_t DEFAULT_DEPTH = 60;
constexpr uint32_t DEFAULT_BATCH_SIZE = 512;

// GF-N defaults
constexpr int DEFAULT_GF_LAYERS = 5;
constexpr int DEFAULT_BASE_N = 50;
constexpr int DEFAULT_N_STEP = 7;

// Security levels
constexpr int SECURITY_STANDARD = 1;
constexpr int SECURITY_ELEVATED = 3;
constexpr int SECURITY_MILITARY = 10;
constexpr int SECURITY_UNIVERSE = 100;

#endif // SPIRAL_CORE_CONSTANTS_H
