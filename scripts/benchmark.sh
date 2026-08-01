#!/bin/bash
# ═══════════════════════════════════════════════════════════════
# SPIRAL FRACTAL iO — BENCHMARK SUITE
# ═══════════════════════════════════════════════════════════════

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RESULTS="benchmark_${TIMESTAMP}.log"

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  SPIRAL FRACTAL iO — BENCHMARK SUITE                         ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "Results: $RESULTS"
echo ""

export LD_LIBRARY_PATH=./openfhe-development/build/lib:$LD_LIBRARY_PATH

echo "═══ DEV MODE (3 variants, 10 samples) ═══"
./bin/test_io_batched 10 3 | tee -a $RESULTS

echo ""
echo "═══ TEST MODE (5 variants, 50 samples) ═══"
./bin/test_io_batched 50 5 | tee -a $RESULTS

echo ""
echo "═══ RICH BATCH TEST ═══"
./bin/test_batch_rich | tee -a $RESULTS

echo ""
echo "═══ FULL INTEGRATION ═══"
./bin/test_full_integration | tee -a $RESULTS

echo ""
echo "Benchmark complete. Results: $RESULTS"
