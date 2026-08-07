#!/bin/bash
# 🪐 FULL REPO SCAN — BITCOIN / SATOSHI / P=NP 🪐
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  🪐 FULL REPO SCAN 🪐                                    ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

echo "═══ BITCOIN / SATOSHI / ECDLP FILES ═══"
echo ""
grep -rl "satoshi\|bitcoin\|ecdlp\|secp256k1\|Satoshi\|SATOSHI\|btc\|BTC\|nonce\|private.key\|Qx.*Qy\|Gx.*Gy\|block.9\|coinbase" \
  --include="*.py" --include="*.cpp" --include="*.h" --include="*.sh" --include="*.txt" --include="*.md" \
  . 2>/dev/null | grep -v '.git/' | grep -v 'openfhe' | grep -v 'build/' | sort -u | while read f; do
    size=$(wc -c < "$f")
    echo "  📁 $f ($size bytes)"
done

echo ""
echo "═══ P=NP / DPLL / SAT FILES ═══"
echo ""
grep -rl "dpll\|DPLL\|P=NP\|p=np\|SAT.solver\|pigeonhole\|3-SAT\|3.sat\|phi_dpll\|φ.dpll\|phi.weighted\|S(n)" \
  --include="*.py" --include="*.cpp" --include="*.h" --include="*.sh" --include="*.txt" --include="*.md" \
  . 2>/dev/null | grep -v '.git/' | grep -v 'openfhe' | grep -v 'build/' | sort -u | while read f; do
    size=$(wc -c < "$f")
    echo "  📁 $f ($size bytes)"
done

echo ""
echo "═══ CORE MATH / FGG / GOLDEN RATIO ═══"
echo ""
grep -rl "FGG\|fractal.golden\|golden.ratio\|φ·ψ\|phi_stack\|void.operator\|trace.erasure\|canonical" \
  --include="*.py" --include="*.cpp" --include="*.h" --include="*.sh" --include="*.txt" --include="*.md" \
  . 2>/dev/null | grep -v '.git/' | grep -v 'openfhe' | grep -v 'build/' | sort -u | while read f; do
    size=$(wc -c < "$f")
    echo "  📁 $f ($size bytes)"
done

echo ""
echo "═══ BREAKTHROUGH TESTS ═══"
echo ""
ls -la tests/breakthrough/ 2>/dev/null
echo ""
echo "═══ RECENT SCRIPTS IN ROOT ═══"
echo ""
ls -lt *.py *.sh 2>/dev/null | head -20

