(* ============================================ *)
(* GOLDEN RATIO SA FINITE FIELD               *)
(* Z_Q kung saan Q ≡ 1 (mod 5)               *)
(* ============================================ *)

Require Import ZArith.

Open Scope Z_scope.

(* Use Q = 11 (smallest prime na ≡ 1 mod 5) *)
Definition Q : Z := 11.

(* sqrt(5) mod 11 = 4, kasi 4^2 = 16 ≡ 5 (mod 11) *)
Definition sqrt5_mod : Z := 4.

(* Verify: 4^2 = 16 ≡ 5 (mod 11) *)
Theorem sqrt5_correct : (sqrt5_mod * sqrt5_mod) mod Q = 5 mod Q.
Proof.
  compute.
  reflexivity.
Qed.

(* Golden ratio mod Q: phi = (1 + sqrt5)/2 mod 11 *)
(* (1 + 4)/2 = 5/2 ≡ 5 * 6 = 30 ≡ 8 (mod 11) *)
Definition phi_mod : Z := 8.

(* psi = (1 - sqrt5)/2 mod 11 *)
(* (1 - 4)/2 = -3/2 ≡ -3 * 6 = -18 ≡ 4 (mod 11) *)
Definition psi_mod : Z := 4.

(* Theorem: phi^2 = phi + 1 (mod Q) *)
Theorem golden_square_mod : (phi_mod * phi_mod) mod Q = (phi_mod + 1) mod Q.
Proof.
  compute.
  reflexivity.
Qed.

(* Theorem: phi * psi = -1 (mod Q) *)
Theorem golden_product_mod : (phi_mod * psi_mod) mod Q = (-1) mod Q.
Proof.
  compute.
  reflexivity.
Qed.

(* Theorem: phi + psi = 1 (mod Q) *)
Theorem golden_sum_mod : (phi_mod + psi_mod) mod Q = 1 mod Q.
Proof.
  compute.
  reflexivity.
Qed.

(* Print theorems *)
Print sqrt5_correct.
Print golden_square_mod.
Print golden_product_mod.
Print golden_sum_mod.
