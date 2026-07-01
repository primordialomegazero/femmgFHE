/**
 * FEmmg-FHE v22 — CTU v4 Golden Chaos + Blackhole Security
 * Unlimited Depth Fully Homomorphic Encryption
 */

declare module '@primordialomegazero/femmg-fhe' {
  export interface FHEConfig {
    key?: string;
    mode?: 'banach' | 'golden-chaos' | 'blackhole';
    nonce?: bigint;
  }

  export interface Ciphertext {
    coordinates: number[];
    chaos_history: number[];
    value_int: bigint;
    noise: number;
    operations: number;
  }

  export class FEmmgFHE {
    constructor(config?: FHEConfig);
    encrypt(plaintext: bigint | number): Ciphertext;
    decrypt(ciphertext: Ciphertext): bigint;
    add(a: Ciphertext, b: Ciphertext): Ciphertext;
    multiply(a: Ciphertext, b: Ciphertext): Ciphertext;
    noise(): number;
    operations(): bigint;
  }

  export class BlackholeFHE {
    constructor(config?: FHEConfig);
    encrypt(data: Buffer | Uint8Array): Buffer;
    decrypt(data: Buffer): Buffer;
  }

  export const VERSION: string;
  export const CTU_VERSION: string;
}
