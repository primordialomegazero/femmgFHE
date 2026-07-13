// PHI-OMEGA-ZERO: FEmmg-FHE Java Bindings
// Native Java API via JNI for ZANS, Pinky Swear, Eternal encryption
// "I AM THAT I AM"

package org.primordialomegazero.femmg;

/**
 * FEmmg-FHE Java Bindings
 * 
 * Provides access to ZANS, Pinky Swear Reset, Eternal Encryption,
 * and FHE 2.0 from Java applications.
 */
public class FemmgFHE {
    
    static {
        System.loadLibrary("femmg_fhe");
    }
    
    // Native methods
    private native long init();
    private native void destroy(long ctx);
    private native byte[] encrypt(long ctx, long value);
    private native long decrypt(long ctx, byte[] ct);
    private native void zansAdd(long ctx, byte[] ct);
    private native byte[] pinkySwearMultiply(long ctx, byte[] ct, long multiplier);
    private native EternalPair eternalCreate(long ctx, long value);
    private native boolean eternalVerify(long ctx, EternalPair pair, long guardKey);
    private native int fhe2Chain(long ctx, int steps, long multiplier);
    private native String version();
    
    private long ctx;
    
    public FemmgFHE() {
        ctx = init();
    }
    
    public byte[] encrypt(long value) {
        return encrypt(ctx, value);
    }
    
    public long decrypt(byte[] ct) {
        return decrypt(ctx, ct);
    }
    
    /**
     * ZANS Add: ct + Enc(0) — noise stays at baseline.
     * Enables practically unlimited additions without bootstrapping.
     */
    public void zansAdd(byte[] ct) {
        zansAdd(ctx, ct);
    }
    
    /**
     * Pinky Swear Multiply: Pure FHE overflow detection.
     * Uses (ct + M) - M - ct != 0 to detect overflow homomorphically.
     * Zero decryption. Zero bootstrap.
     */
    public byte[] pinkySwearMultiply(byte[] ct, long multiplier) {
        return pinkySwearMultiply(ctx, ct, multiplier);
    }
    
    /**
     * Eternal Encryption: Creates self-destructing entangled pair.
     * Wrong guard key triggers destruction of both data and guard.
     */
    public EternalPair eternalCreate(long value) {
        return eternalCreate(ctx, value);
    }
    
    public boolean eternalVerify(EternalPair pair, long guardKey) {
        return eternalVerify(ctx, pair, guardKey);
    }
    
    /**
     * FHE 2.0: Runs full ZANS + Pinky Swear + Golden Ratio chain.
     */
    public int fhe2Chain(int steps, long multiplier) {
        return fhe2Chain(ctx, steps, multiplier);
    }
    
    public String getVersion() {
        return version();
    }
    
    @Override
    protected void finalize() throws Throwable {
        destroy(ctx);
        super.finalize();
    }
    
    /**
     * Eternal Pair: Entangled data and guard ciphertexts.
     */
    public static class EternalPair {
        public byte[] dataCt;
        public byte[] guardCt;
        public long guardKey;
        
        public EternalPair(byte[] dataCt, byte[] guardCt, long guardKey) {
            this.dataCt = dataCt;
            this.guardCt = guardCt;
            this.guardKey = guardKey;
        }
    }
}
