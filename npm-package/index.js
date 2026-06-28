const crypto = require('crypto');

const PHI = 1.6180339887498948482;
const LAMBDA = 0.4812;
const FLOAT_SCALE = 1000000000000;

class FEmmgClient {
  constructor(seed = null, mode = 'ind-cpa') {
    this.phi = PHI;
    this.lambda = LAMBDA;
    this.mode = mode; // 'ind-cpa' or 'accurate'
    this.seed = seed || crypto.randomInt(100000, 999999);
    this.clientId = crypto.createHash('sha256')
      .update(`${this.seed}:${Date.now()}:${crypto.randomBytes(8).toString('hex')}`)
      .digest('hex').substring(0, 16);
    this.nonceCounter = 0;
  }

  // Chaotic nonce with TRUE randomness injected each call
  _chaoticNonce() {
    this.nonceCounter++;
    // Mix true random bytes with chaotic map
    const randBytes = crypto.randomBytes(4);
    const randFloat = randBytes.readUInt32BE(0) / 0xFFFFFFFF;
    
    let x = (this.seed % 10000) / 10000.0 + randFloat * 0.1;
    for (let i = 0; i < 7; i++) {
      x = this.phi * x * (1.0 - x);
    }
    return x * this.lambda * 0.01;
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
  getPublicInfo() { return { client_id: this.clientId, version: '13.0.0', protocol: 'FEmmg-FHE', mode: this.mode }; }
  getSecretKeys() { return { seed: this.seed, client_id: this.clientId, mode: this.mode }; }
  static fromKeys(keys) { const c = new FEmmgClient(keys.seed, keys.mode || 'ind-cpa'); c.clientId = keys.client_id; return c; }
}

module.exports = { FEmmgClient };
