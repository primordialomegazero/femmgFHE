declare module 'femmg-fhe-client' {
  interface ClientKeys {
    phi: number;
    lambda: number;
    client_id: string;
    seed: number;
  }

  interface PublicInfo {
    client_id: string;
    version: string;
    protocol: string;
  }

  interface RegistrationPayload {
    action: string;
    client_id: string;
  }

  interface FHEPayload {
    action: string;
    e1: number;
    e2: number;
    client_id: string;
  }

  class FEmmgClient {
    phi: number;
    lambda: number;
    clientId: string;

    constructor(phi?: number | null);
    encrypt(message: number): number;
    decrypt(encryptedValue: number): number;
    getRegistrationPayload(): RegistrationPayload;
    getAddPayload(e1: number, e2: number): FHEPayload;
    getMultiplyPayload(e1: number, e2: number): FHEPayload;
    getPublicInfo(): PublicInfo;
    getSecretKeys(): ClientKeys;
    static fromKeys(keys: ClientKeys): FEmmgClient;
  }

  export { FEmmgClient, ClientKeys, PublicInfo, RegistrationPayload, FHEPayload };
}
