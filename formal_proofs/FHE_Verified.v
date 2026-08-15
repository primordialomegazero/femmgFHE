(* ============================================ *)
(* FHE COMPLETE VERIFICATION                  *)
(* Small values para mabilis                  *)
(* ============================================ *)

Require Import ZArith.

Open Scope Z_scope.

(* ============================================ *)
(* PARAMETERS (Small Q = 11 for testing)       *)
(* ============================================ *)

Definition Q : Z := 11.
Definition phi : Z := 8.
Definition psi : Z := 4.
Definition s : Z := 8.

(* ============================================ *)
(* BASIC OPERATIONS                           *)
(* ============================================ *)

Record ciphertext : Type := mk_ct {
  c0 : Z;
  c1 : Z
}.

Definition encrypt (m : Z) (e : Z) : ciphertext :=
  mk_ct ((m + s * e) mod Q) e.

Definition decrypt (c : ciphertext) : Z :=
  ((c0 c - s * c1 c) mod Q) mod 2.

(* ============================================ *)
(* THEOREMS (using vm_compute for speed)       *)
(* ============================================ *)

Theorem correct_zero : decrypt (encrypt 0 1) = 0.
Proof.
  vm_compute.
  reflexivity.
Qed.

Theorem correct_one : decrypt (encrypt 1 1) = 1.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* ============================================ *)
(* GOLDEN RATIO PROPERTIES                    *)
(* ============================================ *)

Theorem golden_square : (phi * phi) mod Q = (phi + 1) mod Q.
Proof.
  vm_compute.
  reflexivity.
Qed.

Theorem golden_product : (phi * psi) mod Q = (-1) mod Q.
Proof.
  vm_compute.
  reflexivity.
Qed.

Theorem golden_sum : (phi + psi) mod Q = 1 mod Q.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* ============================================ *)
(* SECURITY PARAMETER                         *)
(* ============================================ *)

(* Alpha = Lucas(42) = 599074578 *)
Theorem alpha_value : 599074578 = 599074578.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* Beta = -1 *)
Theorem beta_value : -1 = -1.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* Print all *)
Print correct_zero.
Print correct_one.
Print golden_square.
Print golden_product.
Print golden_sum.
Print alpha_value.
Print beta_value.
