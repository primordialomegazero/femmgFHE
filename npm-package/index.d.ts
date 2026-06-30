/**
 * FEmmg-FHE v21.0 - TypeScript Definitions
 */

export interface Ciphertext {
    coordinates: number[];
    expanded_dim0: number;
    noise: number;
    phi_state: number;
    operations: number;
    party_id: number;
    nonce: string;
    seed: string;
    signature: string;
}

export interface Config {
    serverUrl?: string;
}

export class FEmmgClient {
    constructor(config?: Config);
    
    /** Register a new session */
    register(): Promise<string>;
    
    /** Encrypt plaintext locally (client-side) */
    encrypt(plaintext: number | string, party?: number): Ciphertext;
    
    /** Generate hybrid nonce from 3 algos */
    generateHybridNonce(): Buffer;
    
    /** Store encrypted data (True ZK) */
    store(ciphertext: Ciphertext): Promise<number>;
    
    /** Decrypt by index */
    decrypt(index: number): Promise<number>;
    
    /** Homomorphic add */
    add(index1: number, index2: number): Promise<number>;
    
    /** Homomorphic multiply */
    multiply(index1: number, index2: number): Promise<number>;
    
    /** Health check */
    health(): Promise<any>;
    
    /** Benchmark TPS */
    tps(): Promise<any>;
}
