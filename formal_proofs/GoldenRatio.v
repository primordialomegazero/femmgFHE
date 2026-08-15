(* ============================================ *)
(* FIBONACCI FHE FORMAL PROOF                 *)
(* Theorem 1: Golden Ratio Square             *)
(* ============================================ *)

Require Import Reals.
Require Import Lra.

Open Scope R_scope.

(* The golden ratio *)
Definition phi : R := (1 + sqrt 5) / 2.
Definition psi : R := (1 - sqrt 5) / 2.

(* Helper lemma: sqrt 5 squared equals 5 *)
Lemma sqrt5_square : sqrt 5 * sqrt 5 = 5.
Proof.
  apply sqrt_sqrt.
  lra.
Qed.

(* Theorem: phi^2 = phi + 1 *)
Theorem golden_ratio_square : phi^2 = phi + 1.
Proof.
  unfold phi.
  (* Expand the square *)
  field_simplify.
  (* Replace sqrt 5 * sqrt 5 with 5 *)
  rewrite sqrt5_square.
  (* Now it's simple arithmetic *)
  ring.
Qed.

(* Theorem: phi * psi = -1 *)
Theorem golden_ratio_product : phi * psi = -1.
Proof.
  unfold phi, psi.
  (* Expand the product *)
  field_simplify.
  (* Replace sqrt 5 * sqrt 5 with 5 *)
  rewrite sqrt5_square.
  (* Now it's simple arithmetic *)
  ring.
Qed.

(* Theorem: phi + psi = 1 *)
Theorem golden_ratio_sum : phi + psi = 1.
Proof.
  unfold phi, psi.
  (* Simple fraction addition *)
  field_simplify.
  (* The sqrt terms cancel *)
  lra.
Qed.

(* Print theorems *)
Print golden_ratio_square.
Print golden_ratio_product.
Print golden_ratio_sum.
