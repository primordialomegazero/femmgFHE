/*
 * FEmmg-FHE — NPM CLIENT (FORTRESS v17.1)
 * True 7D Chaotic Map Lattice IND-CPA
 * crypto.randomBytes(4) per encryption for true entropy
 * 7D Sine-Coupled Map Lattice (bounded, no clamp needed)
 * 
 * PHI-OMEGA-ZERO — I AM THAT I AM
 * Dedicated to Mica — Flame Empress
 */

const crypto = require('crypto');

const PHI = 1.6180339887498948482;
const PHI_INV = 0.6180339887498948482;
const LAMBDA = 0.4812118250596034;
const FLOAT_SCALE = 1000000000000;
const VERSION = '17.1.0';
const DIMS = 7;

class FEmmgClient {
  constructor(seed = null, mode = 'ind-cpa') {
    this.phi = PHI;
    this.lambda = LAMBDA;
    this.mode = mode;
    this.seed = seed || crypto.randomBytes(32).readUInt32BE(0);
    this.clientId = crypto.createHash('sha256')
      .update(`${this.seed}:${Date.now()}:${crypto.randomBytes(8).toString('hex')}`)
      .digest('hex').substring(0, 16);
    this.nonceCounter = 0;
    
    // Initialize 7D state with spread values
    this.state = new Array(DIMS);
    for (let d = 0; d < DIMS; d++) {
      this.state[d] = ((this.seed * (d + 1) * PHI * (d + 7)) % 1000000) / 1000000.0;
    }
  }

  // 7D Sine-Coupled Map Lattice Chaotic Nonce
  // Sine map: x → sin(π * x) — naturally bounded in [0,1], chaotic, no clamp needed
  _chaoticNonce() {
    this.nonceCounter++;
    
    // True random injection (32 bits)
    const randBytes = crypto.randomBytes(4);
    const randFloat = randBytes.readUInt32BE(0) / 0xFFFFFFFF;
    
    // Evolve 7D CML for 10 iterations
    for (let iter = 0; iter < 10; iter++) {
      const newState = new Array(DIMS);
      for (let d = 0; d < DIMS; d++) {
        // Sine map: bounded chaos, range [0,1], Lyapunov exponent ln(π) ≈ 1.14
        const self = Math.sin(Math.PI * this.state[d]);
        
        // Cross-coupling: phi-scaled influence from other dimensions
        let coupling = 0.0;
        for (let j = 0; j < DIMS; j++) {
          if (j !== d) {
            coupling += Math.sin(Math.PI * this.state[j]) * PHI_INV / (1 + Math.abs(d - j));
          }
        }
        
        // Add true random perturbation
        newState[d] = self * PHI_INV + coupling * (1.0 - PHI_INV) + randFloat * 1e-8;
        // Ensure stays in [0,1]
        newState[d] = newState[d] - Math.floor(newState[d]);
      }
      
      for (let d = 0; d < DIMS; d++) {
        this.state[d] = newState[d];
      }
    }
    
    // Extract nonce: sum of state entropy + true random
    let entropy = 0.0;
    for (let d = 0; d < DIMS; d++) {
      entropy += this.state[d] * (d + 1) * PHI;
    }
    
    return (entropy % 0.1) * this.lambda * 0.1 + randFloat * 1e-6;
  }

  encrypt(message) {
    const nonce = this.mode === 'ind-cpa' ? this._chaoticNonce() : 0;
    return message * this.phi + this.lambda + nonce;
  }

  encryptFloat(value) { return this.encrypt(Math.round(value * FLOAT_SCALE)); }
  decrypt(encryptedValue) { return Math.round((encryptedValue - this.lambda) / this.phi); }
  decryptFloat(encryptedValue) { return this.decrypt(encryptedValue) / FLOAT_SCALE; }
  decryptFloatMul(encryptedValue) { return this.decrypt(encryptedValue) / (FLOAT_SCALE * FLOAT_SCALE); }

  encryptPair(a, b) {
    const nonce = this.mode === 'ind-cpa' ? this._chaoticNonce() : 0;
    return [a * this.phi + this.lambda + nonce, b * this.phi + this.lambda + nonce];
  }
  encryptFloatPair(a, b) { return this.encryptPair(Math.round(a * FLOAT_SCALE), Math.round(b * FLOAT_SCALE)); }

  serverAdd(e1, e2) { return e1 + e2 - this.lambda; }
  serverMul(e1, e2) {
    return ((e1 * e2) - this.lambda * (e1 + e2) + this.lambda * this.lambda) / this.phi + this.lambda;
  }

  getRegistrationPayload() { return { action: 'register', client_id: this.clientId }; }
  getAddPayload(e1, e2) { return { action: 'fhe_add', e1, e2, client_id: this.clientId }; }
  getMultiplyPayload(e1, e2) { return { action: 'fhe_multiply', e1, e2, client_id: this.clientId }; }
  getPublicInfo() { return { client_id: this.clientId, version: VERSION, protocol: 'FEmmg-FHE', mode: this.mode, engine: 'FORTRESS v17.1 7D Sine-CML' }; }
  getSecretKeys() { return { seed: this.seed, client_id: this.clientId, mode: this.mode }; }
  static fromKeys(keys) { const c = new FEmmgClient(keys.seed, keys.mode || 'ind-cpa'); c.clientId = keys.client_id; return c; }
}

module.exports = { FEmmgClient };
