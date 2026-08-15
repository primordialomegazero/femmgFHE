Theorem golden_square : forall phi : R,
  phi = (1 + sqrt 5)/2 -> phi^2 = phi + 1.
Proof.
  intros. rewrite H. field. apply sqrt_sqrt. lra.
Qed.
