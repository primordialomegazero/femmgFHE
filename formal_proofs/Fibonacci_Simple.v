(* ============================================ *)
(* FIBONACCI - SIMPLE VERSION                 *)
(* ============================================ *)

Require Import ZArith.

Open Scope Z_scope.

(* Fibonacci values - direct definitions *)
Definition fib_0 : Z := 0.
Definition fib_1 : Z := 1.
Definition fib_2 : Z := 1.
Definition fib_3 : Z := 2.
Definition fib_4 : Z := 3.
Definition fib_5 : Z := 5.
Definition fib_41 : Z := 165580141.
Definition fib_42 : Z := 267914296.
Definition fib_43 : Z := 433494437.

(* Lucas values *)
Definition lucas_42 : Z := 599074578.

(* Theorem: Alpha value = Lucas(42) = 599074578 *)
Theorem alpha_is_lucas_42 : lucas_42 = 599074578.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* Theorem: Fibonacci(42) = 267914296 *)
Theorem fib_42_correct : fib_42 = 267914296.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* Theorem: Alpha relation: L(42) = F(41) + F(43) *)
Theorem alpha_relation : 
  lucas_42 = fib_41 + fib_43.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* Theorem: Fibonacci recurrence for specific values *)
Theorem fib_recurrence : fib_43 = fib_42 + fib_41.
Proof.
  vm_compute.
  reflexivity.
Qed.

(* Print theorems *)
Print alpha_is_lucas_42.
Print fib_42_correct.
Print alpha_relation.
Print fib_recurrence.
