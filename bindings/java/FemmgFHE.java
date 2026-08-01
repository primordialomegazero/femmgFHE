// ═══════════════════════════════════════════════════════════════
// femmgFHE — Java Bindings (JNI)
// ═══════════════════════════════════════════════════════════════

public class FemmgFHE {
    static {
        System.loadLibrary("femmgfhe_java");
    }
    
    // Native methods
    private native long create();
    private native void destroy(long handle);
    private native String version();
    private native double phi();
    private native double psi();
    private native double pi();
    private native int maxRingDim();
    private native int cpuCores();
    private native String cpuBrand();
    private native void setDevMode(long handle);
    private native void setTestMode(long handle);
    private native void setProdMode(long handle);
    private native void setEnterpriseMode(long handle);
    private native int getRingDim(long handle);
    private native int getVariants(long handle);
    private native int getFractalLayers(long handle);
    
    private long handle;
    
    public FemmgFHE() {
        handle = create();
    }
    
    public void setDevMode()        { setDevMode(handle); }
    public void setTestMode()       { setTestMode(handle); }
    public void setProdMode()       { setProdMode(handle); }
    public void setEnterpriseMode() { setEnterpriseMode(handle); }
    
    public int ringDim()       { return getRingDim(handle); }
    public int variants()      { return getVariants(handle); }
    public int fractalLayers() { return getFractalLayers(handle); }
    
    @Override
    protected void finalize() {
        destroy(handle);
    }
    
    // Test
    public static void main(String[] args) {
        System.out.println("╔══════════════════════════════════════════════════════╗");
        System.out.println("║  femmgFHE — Java Bindings Test                      ║");
        System.out.println("╚══════════════════════════════════════════════════════╝");
        System.out.println();
        
        FemmgFHE fh = new FemmgFHE();
        
        System.out.println("Version: " + fh.version());
        System.out.printf("φ=%.6f ψ=%.6f π=%.6f%n%n", fh.phi(), fh.psi(), fh.pi());
        
        System.out.println("═══ Hardware ═══");
        System.out.println("CPU: " + fh.cpuBrand());
        System.out.println("Cores: " + fh.cpuCores());
        System.out.println("Max RingDim: " + fh.maxRingDim());
        System.out.println();
        
        System.out.println("═══ Config Modes ═══");
        fh.setDevMode();
        System.out.println("dev:        RingDim=" + fh.ringDim());
        fh.setProdMode();
        System.out.println("prod:       RingDim=" + fh.ringDim());
        
        System.out.println();
        System.out.println("✅ Java bindings — WORKING!");
    }
}
