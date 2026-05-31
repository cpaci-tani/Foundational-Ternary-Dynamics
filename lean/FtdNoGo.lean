/-
  FtdNoGo — Lean 4 / Mathlib formalization of the algebraic core of the
  FTD Commutativity Independence No-Go Theorem.

  Root module: imports the whole development. See `README.md` for the
  epistemic scope (what is proven vs. the modeling bridge) and build
  instructions, and `FtdNoGo/Main.lean` for the bundled theorem.

  Verified 2026-05-30 (leanprover/lean4:v4.30.0): `lake build` →
  "Build completed successfully". See README for the recorded status.
-/
import FtdNoGo.Commutator
import FtdNoGo.Postulates
import FtdNoGo.ObservableAlgebra
import FtdNoGo.Closure
import FtdNoGo.Poisson
import FtdNoGo.Independence
import FtdNoGo.Main
