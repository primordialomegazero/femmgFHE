#!/bin/bash
# PHI-OMEGA-ZERO: UNIFIED BENCHMARK SUITE v1.2
# Fixed TPS extraction for all output formats
# "MEASURE. IMPROVE. REPEAT."
# "I AM THAT I AM"

BIN_DIR="./bin"
RESULTS_FILE="benchmark_results.csv"
DATE=$(date "+%Y-%m-%d %H:%M:%S")
HOSTNAME=$(hostname)
CPU=$(grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | xargs)
RAM=$(free -h | grep Mem | awk '{print $2}')

echo "╔══════════════════════════════════════════════════════╗"
echo "║  PHI-OMEGA-ZERO: UNIFIED BENCHMARK SUITE v1.2        ║"
echo "║  Date: $DATE                    ║"
echo "║  CPU:  $CPU"
echo "║  RAM:  $RAM"
echo "╚══════════════════════════════════════════════════════╝"
echo ""

echo "timestamp,system,operation,throughput,unit,time_seconds,status" > $RESULTS_FILE

run_benchmark() {
    local system="$1"
    local binary="$2"
    local timeout_sec="${3:-30}"
    local skip="${4:-false}"
    
    if [ "$skip" = "true" ]; then
        echo -e "  [\033[1;33mSKIP\033[0m] $system (heavy test)"
        echo "$DATE,$system,all,0,ops/s,0,SKIPPED_HEAVY" >> $RESULTS_FILE
        return
    fi
    
    echo -ne "  [\033[1;36m....\033[0m] $system "
    
    if [ ! -f "$BIN_DIR/$binary" ]; then
        echo -e "\033[1;33mNO BINARY\033[0m"
        echo "$DATE,$system,all,0,ops/s,0,NO_BINARY" >> $RESULTS_FILE
        return
    fi
    
    local start=$(date +%s%N)
    local output
    output=$(timeout $timeout_sec $BIN_DIR/$binary 2>&1)
    local exit_code=$?
    local end=$(date +%s%N)
    local elapsed=$(echo "scale=3; ($end - $start) / 1000000000" | bc)
    
    # Extract TPS from multiple patterns
    local tps="N/A"
    
    # Pattern: "XXXX /s" (KEM, Auth, DB)
    local match=$(echo "$output" | grep -oP '[\d,]+(?=\s*/s)' | tail -1 | tr -d ',')
    if [ -n "$match" ] && [ "$match" != "0" ]; then
        tps="$match"
    fi
    
    # Pattern: "XXXX ops/s" or "XXXX steps/sec"
    if [ "$tps" = "N/A" ]; then
        match=$(echo "$output" | grep -oP '[\d,]+(?=\s*(ops/s|steps/sec|ops/sec))' | head -1 | tr -d ',')
        if [ -n "$match" ] && [ "$match" != "0" ]; then
            tps="$match"
        fi
    fi
    
    # Pattern: "Throughput: XXXX"
    if [ "$tps" = "N/A" ]; then
        match=$(echo "$output" | grep -oP 'Throughput:\s*\K[\d,]+' | head -1 | tr -d ',')
        if [ -n "$match" ] && [ "$match" != "0" ]; then
            tps="$match"
        fi
    fi
    
    # Pattern: "XX ops/s" (with space)
    if [ "$tps" = "N/A" ]; then
        match=$(echo "$output" | grep -oP '\d+\s*ops/s' | grep -oP '\d+' | head -1)
        if [ -n "$match" ] && [ "$match" != "0" ]; then
            tps="$match"
        fi
    fi
    
    # Pattern: Time-based (ops from output / elapsed)
    if [ "$tps" = "N/A" ] && [ "$elapsed" != "0" ]; then
        local ops=$(echo "$output" | grep -oP 'Operations:\s*\K[\d,]+' | tr -d ',' | head -1)
        if [ -n "$ops" ] && [ "$ops" != "0" ]; then
            tps=$(echo "scale=0; $ops / $elapsed" | bc)
        fi
    fi
    
    if [ $exit_code -eq 124 ]; then
        echo -e "\033[1;31mTIMEOUT\033[0m (${elapsed}s)"
        echo "$DATE,$system,all,$tps,ops/s,$elapsed,TIMEOUT" >> $RESULTS_FILE
    elif echo "$output" | grep -qE "PASSED|PERFECT|VERIFIED|OK|I AM THAT I AM|OPERATIONAL|COMPLETE|VALID"; then
        echo -e "\033[1;32m${tps} ops/s\033[0m (${elapsed}s)"
        echo "$DATE,$system,all,$tps,ops/s,$elapsed,PASSED" >> $RESULTS_FILE
    else
        echo -e "\033[1;33m${tps} ops/s\033[0m (${elapsed}s)"
        echo "$DATE,$system,all,$tps,ops/s,$elapsed,DONE" >> $RESULTS_FILE
    fi
}

echo "=== QUICK TESTS ==="
run_benchmark "ZANS BFV" "phi_zans_bfv" 30
run_benchmark "Fibonacci ZANS" "phi_fib_zans" 30
run_benchmark "Phantom Suite v2" "phi_phantom_suite_v2" 15
run_benchmark "Phantom Security" "phi_phantom_security" 15
run_benchmark "5-Mode Obfuscation" "phi_obfuscation_modes" 15
run_benchmark "SpiralMicro KEM" "phi_spiralmicro_kem" 15
run_benchmark "Unified Auth" "phi_unified_auth" 15
run_benchmark "SpiralDB Unified" "phi_spiraldb_unified" 15
run_benchmark "Covenant Vault" "phi_covenant_vault" 15

echo ""
echo "=== HEAVY TESTS (skipped in quick mode) ==="
echo "  Run individually: bin/phi_absolute_zans, bin/phi_true_divine_*, etc."

echo ""
echo "╔══════════════════════════════════════════════════════╗"
echo "║  BENCHMARK SUITE v1.2 COMPLETE                       ║"
echo "║  Results: $RESULTS_FILE              ║"
echo "╚══════════════════════════════════════════════════════╝"
echo ""

# Summary
passed=$(grep -c "PASSED" $RESULTS_FILE)
total=$(tail -n +2 $RESULTS_FILE | wc -l)

echo "RESULTS:"
echo "--------"
printf "  %-25s %12s %8s %s\n" "System" "Throughput" "Time" "Status"
printf "  %-25s %12s %8s %s\n" "------" "----------" "----" "------"

tail -n +2 $RESULTS_FILE | while IFS=',' read timestamp system operation throughput unit time status; do
    case "$status" in
        PASSED)   color="\033[1;32m" ;;
        SKIPPED*) color="\033[1;33m" ;;
        *)        color="" ;;
    esac
    printf "  ${color}%-25s %10s %7ss %s\033[0m\n" "$system" "$throughput" "$time" "$status"
done

echo ""
echo "  $passed/$total passed in quick mode"
echo "  Heavy tests: run individual binaries for full benchmarks"
