const crypto = require('crypto');

const PHI = 1.6180339887498948482;
const LAMBDA = 0.4812;
const NONCE_SCALE = LAMBDA * 0.0001;  // Ultra-minimal nonce
const FLOAT_SCALE = 1000000000000;

class FEmmgClient {
  constructor(seed = null) {
    this.phi = PHI;
    this.lambda = LAMBDA;
    this.seed = seed || crypto.randomInt(10000, 99999);
    this.clientId = crypto.createHash('sha256')
      .update(`${this.seed}:${Date.now()}:${crypto.randomBytes(8).toString('hex')}`)
      .digest('hex').substring(0, 16);
    this.nonceCounter = 0;
  }

  _chaoticNonce(iteration) {
    let x = (this.seed % 10000) / 10000.0 + 0.1;
    for (let i = 0; i < iteration; i++) x = this.phi * x * (1.0 - x);
    return x * NONCE_SCALE;
  }

  encrypt(message) { this.nonceCounter++; return message * this.phi + this.lambda + this._chaoticNonce(this.nonceCounter); }
  encryptFloat(value) { return this.encrypt(Math.round(value * FLOAT_SCALE)); }
  decrypt(encryptedValue) { return Math.round((encryptedValue - this.lambda) / this.phi); }
  decryptFloat(encryptedValue) { return this.decrypt(encryptedValue) / FLOAT_SCALE; }
  decryptFloatMul(encryptedValue) { return this.decrypt(encryptedValue) / (FLOAT_SCALE * FLOAT_SCALE); }

  encryptPair(a, b) {
    this.nonceCounter++; const nonce = this._chaoticNonce(this.nonceCounter);
    return [a * this.phi + this.lambda + nonce, b * this.phi + this.lambda + nonce];
  }
  encryptFloatPair(a, b) { return this.encryptPair(Math.round(a * FLOAT_SCALE), Math.round(b * FLOAT_SCALE)); }

  serverAdd(e1, e2) { return e1 + e2 - this.lambda; }
  
  // Fully blind multiplication — identical to server formula
  serverMul(e1, e2) {
    const term1 = e1 * e2;
    const term2 = this.lambda * (e1 + e2);
    const term3 = this.lambda * this.lambda;
    return (term1 - term2 + term3) / this.phi + this.lambda;
  }

  getRegistrationPayload() { return { action: 'register', client_id: this.clientId }; }
  getAddPayload(e1, e2) { return { action: 'fhe_add', e1, e2, client_id: this.clientId }; }
  getMultiplyPayload(e1, e2) { return { action: 'fhe_multiply', e1, e2, client_id: this.clientId }; }
  getPublicInfo() { return { client_id: this.clientId, version: '9.0.0', protocol: 'FEmmg-FHE' }; }
  getSecretKeys() { return { seed: this.seed, client_id: this.clientId }; }
  static fromKeys(keys) { const c = new FEmmgClient(keys.seed); c.clientId = keys.client_id; return c; }
}

module.exports = { FEmmgClient };
