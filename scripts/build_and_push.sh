#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════════
# BUILD & PUSH TO GHCR
# ═══════════════════════════════════════════════════════════════════════════════
set -e

GHCR_IMAGE="ghcr.io/primordialomegazero/femmgfhe"
VERSION="${1:-latest}"

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  BUILDING & PUSHING TO GHCR                                  ║"
echo "║  Image: ${GHCR_IMAGE}:${VERSION}                         ║"
echo "╚══════════════════════════════════════════════════════════════╝"

# Login to GHCR
echo ""
echo "=== Logging into GHCR ==="
echo $GITHUB_TOKEN | docker login ghcr.io -u primordialomegazero --password-stdin || \
    docker login ghcr.io

# Build
echo ""
echo "=== Building Docker image ==="
docker build -t ${GHCR_IMAGE}:${VERSION} .
docker tag ${GHCR_IMAGE}:${VERSION} ${GHCR_IMAGE}:latest

# Push
echo ""
echo "=== Pushing to GHCR ==="
docker push ${GHCR_IMAGE}:${VERSION}
docker push ${GHCR_IMAGE}:latest

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  DONE: ${GHCR_IMAGE}:${VERSION}                          ║"
echo "║  Pull: docker pull ${GHCR_IMAGE}:latest                  ║"
echo "╚══════════════════════════════════════════════════════════════╝"
