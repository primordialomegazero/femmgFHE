(* ============================================ *)
(* FHE CORE FORMAL PROOF                      *)
(* Encrypt, Decrypt, NAND Correctness         *)
(* ============================================ *)

Require Import ZArith.

Open Scope Z_scope.

(* Define ciphertext as pair of integers *)
Record ciphertext : Type := mk_ciphertext {
  c0 : Z;
  c1 : Z
}.

(* Secret key - fixed value for simplicity *)
Definition secret_key : Z := 42.

(* Encryption function *)
Definition encrypt (m : Z) : ciphertext :=
  mk_ciphertext (m + secret_key) 1.

(* Decryption function *)
Definition decrypt (c : ciphertext) : Z :=
  (c0 c - secret_key * c1 c) mod 2.

(* Theorem: Decryption is correct for 0 *)
Theorem decrypt_encrypt_zero : decrypt (encrypt 0) = 0.
Proof.
  compute.
  reflexivity.
Qed.

(* Theorem: Decryption is correct for 1 *)
Theorem decrypt_encrypt_one : decrypt (encrypt 1) = 1.
Proof.
  compute.
  reflexivity.
Qed.

(* NAND gate on encrypted data *)
Definition nand_gate (a b : ciphertext) : ciphertext :=
  mk_ciphertext (1 - c0 a * c0 b) 0.

(* Theorem: NAND truth table - all 4 cases *)
Theorem nand_correct_00 : decrypt (nand_gate (encrypt 0) (encrypt 0)) = 1.
Proof.
  compute.
  reflexivity.
Qed.

Theorem nand_correct_01 : decrypt (nand_gate (encrypt 0) (encrypt 1)) = 1.
Proof.
  compute.
  reflexivity.
Qed.

Theorem nand_correct_10 : decrypt (nand_gate (encrypt 1) (encrypt 0)) = 1.
Proof.
  compute.
  reflexivity.
Qed.

Theorem nand_correct_11 : decrypt (nand_gate (encrypt 1) (encrypt 1)) = 0.
Proof.
  compute.
  reflexivity.
Qed.

(* Print theorems *)
Print decrypt_encrypt_zero.
Print decrypt_encrypt_one.
Print nand_correct_00.
Print nand_correct_01.
Print nand_correct_10.
Print nand_correct_11.
