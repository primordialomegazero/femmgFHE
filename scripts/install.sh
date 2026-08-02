#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════════
# SPIRAL FRACTAL iO — INSTALL SCRIPT
# ═══════════════════════════════════════════════════════════════════════════════
set -e

PREFIX="${PREFIX:-/usr/local}"
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  SPIRAL FRACTAL iO — INSTALLING → $PREFIX                      ║"
echo "╚══════════════════════════════════════════════════════════════╝"

# Build everything
echo ""
echo "=== Building ==="
make all

# Directories
echo ""
echo "=== Installing ==="
sudo install -d $PREFIX/bin $PREFIX/include/spiral $PREFIX/lib/spiral

# Binaries
for bin in spiralc spiralrun test_unified_all phi_kem_level5; do
    if [ -f bin/$bin ]; then
        sudo install -m 755 bin/$bin $PREFIX/bin/$bin
        echo "  ✅ $PREFIX/bin/$bin"
    fi
done

# Symlinks
sudo ln -sf $PREFIX/bin/test_unified_all $PREFIX/bin/spiral-phi-test
sudo ln -sf $PREFIX/bin/phi_kem_level5 $PREFIX/bin/spiral-kem

# Headers
sudo cp -r src/* $PREFIX/include/spiral/
sudo cp unified-phi-stack/phi_stack.h $PREFIX/include/spiral/
echo "  ✅ Headers → $PREFIX/include/spiral/"

# OpenFHE libs
sudo cp openfhe-development/build/lib/libOPENFHE*.so* $PREFIX/lib/spiral/ 2>/dev/null || true
echo "  ✅ Libraries → $PREFIX/lib/spiral/"

# Environment
cat > /tmp/spiral-env << EOF
#!/bin/bash
export LD_LIBRARY_PATH=$PREFIX/lib/spiral:\$LD_LIBRARY_PATH
export SPIRAL_HOME=$PREFIX
export PATH=$PREFIX/bin:\$PATH
EOF
sudo install -m 755 /tmp/spiral-env $PREFIX/bin/spiral-env
rm /tmp/spiral-env

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  INSTALL COMPLETE                                            ║"
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║  Tools:   spiralc, spiralrun, spiral-kem, spiral-phi-test    ║"
echo "║  Env:     source $PREFIX/bin/spiral-env                      ║"
echo "║  Headers: $PREFIX/include/spiral/                            ║"
echo "╚══════════════════════════════════════════════════════════════╝"
