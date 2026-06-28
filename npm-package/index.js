const crypto = require('crypto');

const PHI = 1.6180339887498948482;
const LAMBDA = 0.4812;
const NONCE_SCALE = LAMBDA * 0.1;

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

  encrypt(message) {
    this.nonceCounter++;
    return message * this.phi + this.lambda + this._chaoticNonce(this.nonceCounter);
  }

  encryptPair(a, b) {
    this.nonceCounter++;
    const nonce = this._chaoticNonce(this.nonceCounter);
    return [a * this.phi + this.lambda + nonce, b * this.phi + this.lambda + nonce];
  }

  decrypt(encryptedValue) {
    return Math.round((encryptedValue - this.lambda) / this.phi);
  }

  serverAdd(e1, e2) { return e1 + e2 - this.lambda; }
  serverMul(e1, e2) { return (e1 - this.lambda) * (e2 - this.lambda) / this.phi + this.lambda; }
  getRegistrationPayload() { return { action: 'register', client_id: this.clientId }; }
  getAddPayload(e1, e2) { return { action: 'fhe_add', e1, e2, client_id: this.clientId }; }
  getMultiplyPayload(e1, e2) { return { action: 'fhe_multiply', e1, e2, client_id: this.clientId }; }
  getPublicInfo() { return { client_id: this.clientId, version: '6.2.0', protocol: 'FEmmg-FHE' }; }
  getSecretKeys() { return { seed: this.seed, client_id: this.clientId }; }
  static fromKeys(keys) { const c = new FEmmgClient(keys.seed); c.clientId = keys.client_id; return c; }
}

module.exports = { FEmmgClient };
