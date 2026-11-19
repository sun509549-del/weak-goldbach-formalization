import WeakGoldbach.Foundations

/-!
# A kernel-checked finite certificate

The table supplies one prime `p` for each odd `n` from 7 through 101.
Lean checks that `3`, `p` and `n - 3 - p` are prime and sum to `n`.
The table is untrusted input: the theorem `smallCertificate_checks` checks
all of its relevant entries using ordinary kernel reduction (`decide`).
This file does not import or trust the C++ program or its output.
-/

namespace WeakGoldbach

/-- Candidate witness table; no mathematical claim is made by this definition. -/
def smallWitness : ℕ → ℕ
  | 7 => 2
  | 9 => 3
  | 11 => 3
  | 13 => 3
  | 15 => 5
  | 17 => 3
  | 19 => 3
  | 21 => 5
  | 23 => 3
  | 25 => 3
  | 27 => 5
  | 29 => 3
  | 31 => 5
  | 33 => 7
  | 35 => 3
  | 37 => 3
  | 39 => 5
  | 41 => 7
  | 43 => 3
  | 45 => 5
  | 47 => 3
  | 49 => 3
  | 51 => 5
  | 53 => 3
  | 55 => 5
  | 57 => 7
  | 59 => 3
  | 61 => 5
  | 63 => 7
  | 65 => 3
  | 67 => 3
  | 69 => 5
  | 71 => 7
  | 73 => 3
  | 75 => 5
  | 77 => 3
  | 79 => 3
  | 81 => 5
  | 83 => 7
  | 85 => 3
  | 87 => 5
  | 89 => 3
  | 91 => 5
  | 93 => 7
  | 95 => 3
  | 97 => 5
  | 99 => 7
  | 101 => 19
  | _ => 0

set_option maxRecDepth 10000 in
/-- Every relevant table entry passes primality and exact-sum checks. -/
theorem smallCertificate_checks :
    ∀ n : Fin 102, 7 ≤ n.val → n.val % 2 = 1 →
      Nat.Prime (smallWitness n.val) ∧
      Nat.Prime (n.val - 3 - smallWitness n.val) ∧
      3 + smallWitness n.val + (n.val - 3 - smallWitness n.val) = n.val := by
  decide

/-- All 48 odd integers in the inclusive interval `[7, 101]` have prime triples. -/
theorem weakGoldbach_upTo101 (n : ℕ)
    (hmin : 7 ≤ n) (hmax : n ≤ 101) (hodd : n % 2 = 1) :
    HasPrimeTriple n := by
  have checks := smallCertificate_checks ⟨n, Nat.lt_succ_of_le hmax⟩ hmin hodd
  exact ⟨3, smallWitness n, n - 3 - smallWitness n,
    by decide, checks.1, checks.2.1, checks.2.2⟩

/--
The checked finite interval leaves precisely the tail hypothesis below.
This conditional theorem does not provide or assume a global tail proof.
-/
theorem weakGoldbach_of_tail102
    (tail : ∀ n : ℕ, 102 ≤ n → n % 2 = 1 → HasPrimeTriple n) :
    ∀ n : ℕ, 7 ≤ n → n % 2 = 1 → HasPrimeTriple n := by
  apply finite_tail_splice 102
  · intro n hmin hsmall hodd
    exact weakGoldbach_upTo101 n hmin (Nat.le_of_lt_succ hsmall) hodd
  · intro n _ hlarge hodd
    exact tail n hlarge hodd

end WeakGoldbach
