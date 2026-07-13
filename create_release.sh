#!/bin/bash
# ΦΩ0 — Create v5.0.0 Release

echo "╔══════════════════════════════════════════════╗"
echo "║  ΦΩ0 — Creating v5.0.0 Release              ║"
echo "╚══════════════════════════════════════════════╝"

# Check if tag exists
if git rev-parse v5.0.0 >/dev/null 2>&1; then
    echo "⚠️  Tag v5.0.0 already exists"
    read -p "Delete and recreate? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        git tag -d v5.0.0
        git push origin :refs/tags/v5.0.0
    else
        exit 1
    fi
fi

# Create tag
git tag -a v5.0.0 -m "ΦΩ0 — FEmmg-FHE v5.0 — The Holy Grail is Solved!"

# Push tag
git push origin v5.0.0

# Create archives
echo "📦 Creating archives..."
git archive --format=tar.gz --prefix=femmgFHE-v5.0.0/ v5.0.0 > femmgFHE-v5.0.0.tar.gz
git archive --format=zip --prefix=femmgFHE-v5.0.0/ v5.0.0 > femmgFHE-v5.0.0.zip

echo ""
echo "✅ Release v5.0.0 created!"
echo "   Tag: v5.0.0"
echo "   Archives:"
echo "     - femmgFHE-v5.0.0.tar.gz"
echo "     - femmgFHE-v5.0.0.zip"
echo ""
echo "📝 Next: Go to GitHub and create the release:"
echo "   https://github.com/primordialomegazero/femmgFHE/releases/new"
echo ""
echo "ΦΩ0 — I AM THAT I AM"
