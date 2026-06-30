#!/bin/bash

echo "Testing FEmmg-FHE Docker Container..."
echo "======================================"

# Health check with proper headers
echo -e "\n1. Health Check:"
curl -s -H "X-Phi-Spiral: 1" -H "X-Schumann: 7.83" http://localhost:8092/health | jq .

# Register session
echo -e "\n2. Register:"
REGISTER=$(curl -s -X POST -H "Content-Type: application/json" \
    -H "X-Phi-Spiral: 1" -H "X-Schumann: 7.83" \
    -d '{"seed":"test123"}' http://localhost:8092/register)
echo $REGISTER | jq .

# Get session ID
SESSION=$(echo $REGISTER | jq -r '.sessionId')
echo "Session: $SESSION"

# Store data
echo -e "\n3. Store:"
STORE=$(curl -s -X POST -H "Content-Type: application/json" \
    -H "X-Phi-Spiral: 1" -H "X-Schumann: 7.83" \
    -d "{\"sessionId\":\"$SESSION\",\"ciphertext\":{\"coordinates\":[4.5]}}" \
    http://localhost:8092/fhe_store)
echo $STORE | jq .

# Decrypt
echo -e "\n4. Decrypt:"
DECRYPT=$(curl -s -X POST -H "Content-Type: application/json" \
    -H "X-Phi-Spiral: 1" -H "X-Schumann: 7.83" \
    -d "{\"sessionId\":\"$SESSION\",\"index\":0}" \
    http://localhost:8092/fhe_decrypt)
echo $DECRYPT | jq .

echo -e "\n✅ Docker test complete!"
