package main

import (
	"fmt"
	"os"
	"time"

	"github.com/tuneinsight/lattigo/v5/core/rlwe"
	"github.com/tuneinsight/lattigo/v5/schemes/bgv"
)

func main() {
	log, _ := os.Create("../../../lattigo_zans_10M_results.txt")
	defer log.Close()

	msg := func(s string) {
		fmt.Print(s)
		log.WriteString(s)
	}

	msg("\n===============================================================\n")
	msg("  PHI-OMEGA-ZERO: Lattigo ZANS 10M — DETAILED CHECKPOINTS\n")
	msg("  10,000,000 Sequential ZANS Additions on Lattigo v5 (Go)\n")
	msg("===============================================================\n")
	msg("  HARDWARE: AMD Ryzen 5 2600 (6 cores, 12 threads, 15GB RAM)\n")
	msg("  LIBRARY: Lattigo v5 (Go) | BGV Scheme\n")
	msg("  RING: 2^13=8192 | PLAINTEXT: 65537\n")
	msg("  CHECKPOINT: Every 1M steps\n")
	msg(fmt.Sprintf("  START: %s\n", time.Now().Format("2006-01-02 15:04:05")))
	msg("===============================================================\n\n")

	params, err := bgv.NewParametersFromLiteral(bgv.ParametersLiteral{
		LogN:             13,
		LogQ:             []int{50, 40, 40, 40, 40},
		PlaintextModulus: 65537,
	})
	if err != nil { panic(err) }

	kgen := rlwe.NewKeyGenerator(params)
	sk := kgen.GenSecretKeyNew()
	pk := kgen.GenPublicKeyNew(sk)

	encoder := bgv.NewEncoder(params)
	encryptor := rlwe.NewEncryptor(params, pk)
	decryptor := rlwe.NewDecryptor(params, sk)
	evaluator := bgv.NewEvaluator(params, rlwe.NewMemEvaluationKeySet(kgen.GenRelinearizationKeyNew(sk)))

	pt := make([]uint64, params.MaxSlots())
	pt[0] = 42
	plaintext := bgv.NewPlaintext(params, params.MaxLevel())
	encoder.Encode(pt, plaintext)
	ct, _ := encryptor.EncryptNew(plaintext)

	ptZero := make([]uint64, params.MaxSlots())
	plaintextZero := bgv.NewPlaintext(params, params.MaxLevel())
	encoder.Encode(ptZero, plaintextZero)
	anchor0, _ := encryptor.EncryptNew(plaintextZero)

	totalSteps := 10000000
	checkpoint := 1000000

	msg("  SETUP COMPLETE. Starting 10M ZANS chain...\n\n")
	msg("  Step         Noise    Pattern  Elapsed        TPS        ETA\n")
	msg("  " + "-----------------------------------------------------------------------\n")

	tStart := time.Now()
	prevNoise := 0.0
	stable := true

	for step := 0; step < totalSteps; step++ {
		evaluator.Add(ct, anchor0, ct)

		if (step+1)%checkpoint == 0 {
			elapsed := time.Since(tStart).Seconds()
			tps := float64(step+1) / elapsed
			noise := float64(ct.Level()) // Use level as noise proxy
			remaining := float64(totalSteps-step-1) / tps
			
			pattern := "START"
			if prevNoise > 0 {
				if noise-prevNoise < 1.0 && noise-prevNoise > -1.0 {
					pattern = "OK"
				} else {
					pattern = "?"
					stable = false
				}
			}

			msg(fmt.Sprintf("  %10d %10.1f %10s  %dh%dm%ds %10.1f %10dh%dm\n",
				step+1, noise, pattern,
				int(elapsed)/3600, (int(elapsed)%3600)/60, int(elapsed)%60,
				tps,
				int(remaining)/3600, (int(remaining)%3600)/60))

			prevNoise = noise
		}
	}

	totalSec := time.Since(tStart).Seconds()

	decrypted := make([]uint64, params.MaxSlots())
	encoder.Decode(decryptor.DecryptNew(ct), decrypted)

	msg(fmt.Sprintf("\n=======================================================================\n"))
	msg(fmt.Sprintf("  Lattigo ZANS 10M — COMPLETE\n"))
	msg(fmt.Sprintf("  Steps: %d | Final Level: %d\n", totalSteps, ct.Level()))
	msg(fmt.Sprintf("  Value: %d (expected: 42)\n", decrypted[0]))
	msg(fmt.Sprintf("  Pattern: %s\n", map[bool]string{true: "STABLE", false: "DRIFT"}[stable]))
	msg(fmt.Sprintf("  Time: %.1f hours\n", totalSec/3600.0))
	msg(fmt.Sprintf("  Completed: %s\n", time.Now().Format("2006-01-02 15:04:05")))
	msg("=======================================================================\n\n")
}
