#!/bin/bash

echo "BUILDING IMMORTAL FHE..."
echo "NO HARDCODING - ALL PARAMETERS DYNAMIC"

OPENFHE_INCLUDE="/usr/local/include/openfhe"
OPENFHE_LIB="/usr/local/lib"

echo "OpenFHE include: $OPENFHE_INCLUDE"
echo "OpenFHE lib: $OPENFHE_LIB"

g++ -std=c++17 -O2 -Wno-unknown-pragmas -Wno-narrowing -o immortal_fhe_test immortal_fhe_test.cpp \
    -I$OPENFHE_INCLUDE -I$OPENFHE_INCLUDE/core \
    -I$OPENFHE_INCLUDE/pke -I$OPENFHE_INCLUDE/binfhe \
    -L$OPENFHE_LIB -lOPENFHEcore -lOPENFHEpke -lOPENFHEbinfhe \
    -lssl -lcrypto -lm -lpthread -Wl,-rpath,$OPENFHE_LIB

if [ $? -eq 0 ]; then
    echo ""
    echo "BUILD SUCCESSFUL"
    echo ""
    echo "RUNNING IMMORTAL FHE..."
    echo ""
    ./immortal_fhe_test
else
    echo "BUILD FAILED"
    exit 1
fi
