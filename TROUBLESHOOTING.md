# Troubleshooting Guide

## Docker Build Issues

### "No space left on device"
OpenFHE compilation requires ~8GB temporary space.
```
# Clean Docker cache
docker system prune -af

# Or build on a machine with more space
```

### "make: *** No targets specified"
CMake may have failed silently.
```
# Check if OpenFHE cloned properly
ls openfhe-development/CMakeLists.txt

# Re-clone if missing
git clone https://github.com/openfhe-org/openfhe-development.git
```

### "libOPENFHEpke.so: cannot open shared object"
Library path not set.
```
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
ldconfig
```

### "liboqs not found"
```
# Install manually
git clone https://github.com/open-quantum-safe/liboqs.git
cd liboqs && mkdir build && cd build
cmake .. && make -j$(nproc) && sudo make install
sudo ldconfig
```

---

## Runtime Issues

### "Segmentation fault (core dumped)"
Usually RingDim too high for available RAM.
- Reduce RingDim in SystemConfig (try 2048 or 4096)
- Check: `free -h` for available RAM

### "KS = 1.0000" (all pairs failing)
FHE context may not be initializing.
- Check OpenFHE build: `ldconfig -p | grep OPENFHE`
- Verify library path: `echo $LD_LIBRARY_PATH`

### "HardwareObfuscator: init failed"
Missing /proc/cpuinfo (common in minimal Docker images).
- Mount /proc: `docker run -v /proc:/proc ...`
- Or disable hardware obfuscation in config

### "Cannot open database file"
Permissions issue on mounted volume.
```
# Fix permissions
chmod 777 /data
# Or run as current user
docker run -u $(id -u):$(id -g) -v $(pwd)/data:/data ...
```

---

## Performance Issues

### "Very slow (> 1 min per sample)"
RingDim too high for available CPU.
- DEV mode: RingDim=2048 (~30 seconds)
- TEST mode: RingDim=4096 (~5 minutes)
- PROD mode: RingDim=8192 (~30 minutes)
- ENTERPRISE: RingDim=32768 (hours)

### "Out of memory"
Reduce RingDim or add swap.
```
# Add swap
fallocate -l 16G /swapfile
chmod 600 /swapfile
mkswap /swapfile
swapon /swapfile
```

---

## Architecture-Specific

### ARM64 (Apple Silicon, Raspberry Pi)
OpenFHE works on ARM64 but is slower.
- Use `-DWITH_OPENMP=OFF` for ARM compatibility
- Expect 3-5x slower performance

### WSL (Windows Subsystem for Linux)
Works. Use WSL2 for better performance.
- Ensure WSL2 has enough RAM (.wslconfig)

### Bare Metal Linux
Best performance. No overhead.
- Install dependencies directly
- Build without Docker for maximum speed

---

## Getting Help

- GitHub Issues: https://github.com/primordialomegazero/femmgFHE/issues
- Contact: devilswithin13@gmail.com
- Hardware requirements: see README.md Hardware section
