(* ============================================ *)
(* FIBONACCI SEQUENCE - FORMAL PROOF          *)
(* ============================================ *)

Require Import ZArith.

Open Scope Z_scope.

(* Fixpoint for Fibonacci na may proper recursion *)
Fixpoint fib (n : nat) : Z :=
  match n with
  | O => 0
  | S O => 1
  | S (S n' as n'') => fib n'' + fib n'
  end.

(* Compute Fibonacci numbers *)
Compute fib 0.
Compute fib 1.
Compute fib 2.
Compute fib 3.
Compute fib 4.
Compute fib 5.
Compute fib 10.
Compute fib 42.

(* Lucas numbers via Fibonacci *)
Definition lucas (n : nat) : Z :=
  match n with
  | O => 2
  | S O => 1
  | S (S n') => fib (S n') + fib n'
  end.

(* Compute Lucas numbers *)
Compute lucas 0.
Compute lucas 1.
Compute lucas 2.
Compute lucas 3.
Compute lucas 4.
Compute lucas 5.
Compute lucas 42.

(* Theorem: Lucas(42) = 599074578 (alpha value sa FHE) *)
Theorem lucas_42 : lucas 42 = 599074578.
Proof.
  compute.
  reflexivity.
Qed.

(* Theorem: Fibonacci(42) = 267914296 *)
Theorem fib_42 : fib 42 = 267914296.
Proof.
  compute.
  reflexivity.
Qed.

(* Theorem: Fibonacci recurrence holds for specific values *)
Theorem fib_recurrence_42 : fib 44 = fib 43 + fib 42.
Proof.
  compute.
  reflexivity.
Qed.

(* Theorem: Relationship between Fibonacci and Lucas *)
Theorem lucas_fib_relation : lucas 42 = fib 41 + fib 43.
Proof.
  compute.
  reflexivity.
Qed.

(* Print theorems *)
Print lucas_42.
Print fib_42.
Print fib_recurrence_42.
Print lucas_fib_relation.
