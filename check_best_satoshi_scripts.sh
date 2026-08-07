#!/bin/bash
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  CHECKING BEST SATOSHI SCRIPTS                            ║"
echo "╚══════════════════════════════════════════════════════════════╝"

echo ""
echo "═══ 1. GOD LEVEL SOLVER ═══"
head -80 god_level_solver.py 2>/dev/null || echo "Not found"

echo ""
echo "═══ 2. SATOSHI FINAL ═══"
head -60 attack_satoshi_final.py 2>/dev/null

echo ""
echo "═══ 3. φ-DPLL ON ECDLP (C++) ═══"
head -60 tests/breakthrough/test_satoshi_phi_dpll_ecdlp.cpp 2>/dev/null

echo ""
echo "═══ 4. PHASE 16 — SAT APPROACH (C++) ═══"
head -60 tests/breakthrough/test_satoshi_phase16_sat.cpp 2>/dev/null

echo ""
echo "═══ 5. ULTIMATE SATOSHI (C++) ═══"
head -60 tests/breakthrough/test_satoshi_ultimate.cpp 2>/dev/null

echo ""
echo "═══ 6. REAL SATOSHI RECOVERY (C++) ═══"
head -60 tests/breakthrough/test_real_satoshi_recovery.cpp 2>/dev/null

echo ""
echo "═══ 7. ECDLP SAT ATTACK (C++) ═══"
head -60 tests/breakthrough/test_ecdlp_sat_attack.cpp 2>/dev/null

