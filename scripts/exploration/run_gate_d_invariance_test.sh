#!/bin/bash
# Gate D INVARIANCE test for theorem-grade diagonals (JJ, J4, stateSq, reactionDensity).
#
# Theorem 1 + 2 predict M_{JJ,JJ} = b^4 = 16 and M_{J4,J4} = b^8 = 256
# EXACTLY for any smooth-field ensemble configuration. Theorem 3 predicts
# M_{stateSq,stateSq} = b^3 (1 + 2 rho_intra-block) where rho depends on
# the ensemble but the b^3 part is structural.
#
# Test: perturb T_langevin and check whether the theorem predictions hold
# at the perturbed value. PASS = theorem-grade diagonals are invariant
# to T perturbation within stderr.
#
# Test design: 4 T values around the canonical T=0.100, smaller ensemble
# (N_samples=500) for fast runs. Each takes ~5 min wall on RTX 5090
# at L=32.
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd

mkdir -p engine/results/s_eff_nonlinear_2026-04-29/GateD_T_perturbation

for T in 0.090 0.100 0.110 0.120; do
    OUT=engine/results/s_eff_nonlinear_2026-04-29/GateD_T_perturbation/T${T}
    echo "=== Gate D T=${T} L=32 N=500 ==="
    time ./engine/build_wsl/campaign_s_eff_nonlinear \
        --scenario=pair-rich --L=32 --N-seeds=10 --N-samples=500 --N-burn=200 \
        --T-langevin=${T} --b4 \
        --output-dir=${OUT} \
        > ${OUT}.log 2>&1 || true
    echo "  done"
done

echo ""
echo "=== Gate D T-perturbation summary ==="
for T in 0.090 0.100 0.110 0.120; do
    OUT=engine/results/s_eff_nonlinear_2026-04-29/GateD_T_perturbation/T${T}
    if [ -f ${OUT}/M_ab.csv ]; then
        # Extract diagonal entries and stderr from CSV
        M_JJ=$(awk -F, '$1=="JJ" && $2=="JJ" {print $3}' ${OUT}/M_ab.csv)
        M_J4=$(awk -F, '$1=="J4" && $2=="J4" {print $3}' ${OUT}/M_ab.csv)
        M_stateSq=$(awk -F, '$1=="stateSq" && $2=="stateSq" {print $3}' ${OUT}/M_ab.csv)
        M_rxnDens=$(awk -F, '$1=="reactionDensity" && $2=="reactionDensity" {print $3}' ${OUT}/M_ab.csv)
        printf "  T=%-7s JJ=%10.4f  J4=%12.4f  stateSq=%8.4f  reactionDensity=%8.4f\n" \
            "${T}" "${M_JJ:-NaN}" "${M_J4:-NaN}" "${M_stateSq:-NaN}" "${M_rxnDens:-NaN}"
    else
        echo "  T=${T} : no output"
    fi
done

echo ""
echo "Expected by Theorems 1, 2, 3:"
echo "  JJ should be 16.000 invariant; J4 should be 256.000 invariant"
echo "  stateSq, reactionDensity may drift (rho_intra-block depends on T)"
