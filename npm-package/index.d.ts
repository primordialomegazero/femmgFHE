/**
 * FEmmg-FHE v22.0.0 — Fibonacci-Lyapunov Fully Homomorphic Encryption
 * 
 * Unlimited Depth FHE with Golden Chaos (CTU v4) + Banach Contraction.
 * 
 * @author Dan Joseph M. Fernandez / Primordial Omega Zero
 * @license MIT
 * @version 22.0.0
 */

declare module '@primordialomegazero/femmg-fhe' {
  export interface FHEConfig {
    key?: bigint;
    mode?: 'banach' | 'golden-chaos' | 'blackhole';
    nonce?: bigint;
  }

  export interface Ciphertext {
    value_int: bigint;
    noise: number;
    operations: number;
  }

  export class FEmmgFHE {
    constructor(config?: FHEConfig);
    encrypt(plaintext: bigint | number, key?: bigint): Ciphertext;
    decrypt(ciphertext: Ciphertext): bigint;
    add(a: Ciphertext, b: Ciphertext): Ciphertext;
    multiply(a: Ciphertext, b: Ciphertext): Ciphertext;
    noise(): number;
    operations(): bigint;
  }

  export const VERSION: string;
  export const CTU_VERSION: string;
  export const BENCHMARK_TPS: number;
}
