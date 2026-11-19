import Mathlib.Data.Nat.Prime
import Mathlib.Analysis.SpecialFunctions.Trigonometric.Basic

/-!
# Elementary foundations for a weak Goldbach project

These results do not establish either of the analytic estimates required by
the circle method. In particular, the bound below is only the triangle
inequality bound; it contains no cancellation estimate.
-/

open scoped BigOperators

namespace WeakGoldbach

noncomputable section

/-- The primes strictly below `N`, matching the original draft's convention. -/
def primesBelow (N : ℕ) : Finset ℕ :=
  (Finset.range N).filter Nat.Prime

/-- The complex phase `exp(i x)`, with the real-to-complex coercion explicit. -/
def phase (x : ℝ) : ℂ :=
  Complex.exp ((x : ℂ) * Complex.I)

/-- An unweighted exponential sum over primes strictly below `N`. -/
def expSum (N : ℕ) (α : ℝ) : ℂ :=
  ∑ p in primesBelow N, phase (2 * Real.pi * (p : ℝ) * α)

/-- Every summand has unit norm. -/
theorem norm_phase (x : ℝ) : ‖phase x‖ = 1 := by
  change Complex.abs (Complex.exp ((x : ℂ) * Complex.I)) = 1
  rw [Complex.abs_exp]
  simp

/-- Foundation 1: the triangle inequality bounds the sum by its number of terms. -/
theorem norm_expSum_le_card (N : ℕ) (α : ℝ) :
    ‖expSum N α‖ ≤ ((primesBelow N).card : ℝ) := by
  calc
    ‖expSum N α‖ ≤ ∑ p in primesBelow N,
        ‖phase (2 * Real.pi * (p : ℝ) * α)‖ := norm_sum_le _ _
    _ = ((primesBelow N).card : ℝ) := by simp [norm_phase]

/-- A coarser bound that does not use a prime-counting estimate. -/
theorem norm_expSum_le (N : ℕ) (α : ℝ) : ‖expSum N α‖ ≤ (N : ℝ) := by
  have hcard : (primesBelow N).card ≤ N := by
    calc
      (primesBelow N).card ≤ (Finset.range N).card :=
        Finset.card_le_card (Finset.filter_subset _ _)
      _ = N := Finset.card_range N
  exact (norm_expSum_le_card N α).trans (Nat.cast_le.mpr hcard)

/-- The integration domain is the closed unit interval. -/
def unitInterval : Set ℝ := Set.Icc 0 1

/--
Rational neighborhoods on the unit interval, including integer translates
so that neighborhoods near `0` and `1` wrap around the circle. The radius is
the explicit parameter `δ`; no relation to `N` is silently assumed.
-/
def majorArc (Q : ℕ) (δ : ℝ) : Set ℝ :=
  {α | α ∈ unitInterval ∧ ∃ q : ℕ, 1 ≤ q ∧ q ≤ Q ∧
    ∃ a : ℕ, a < q ∧ Nat.Coprime a q ∧
    ∃ k : ℤ, |α - ((a : ℝ) / (q : ℝ) + (k : ℝ))| ≤ δ}

/-- The complementary part of the same integration domain. -/
def minorArc (Q : ℕ) (δ : ℝ) : Set ℝ :=
  unitInterval \ majorArc Q δ

theorem majorArc_subset_unitInterval (Q : ℕ) (δ : ℝ) :
    majorArc Q δ ⊆ unitInterval := fun _ h => h.1

/-- Foundation 2: the two arc regions cover exactly the chosen domain. -/
theorem majorArc_union_minorArc (Q : ℕ) (δ : ℝ) :
    majorArc Q δ ∪ minorArc Q δ = unitInterval := by
  classical
  ext α
  constructor
  · intro h
    rcases h with h | h
    · exact h.1
    · exact h.1
  · intro h
    by_cases hm : α ∈ majorArc Q δ
    · exact Or.inl hm
    · exact Or.inr ⟨h, hm⟩

/-- The partition has no overlapping points. -/
theorem majorArc_inter_minorArc (Q : ℕ) (δ : ℝ) :
    majorArc Q δ ∩ minorArc Q δ = ∅ := by
  ext α
  constructor
  · intro h
    exact (h.2.2 h.1).elim
  · intro h
    exact h.elim

/-- A concrete ternary-prime representation; repetition of primes is allowed. -/
def HasPrimeTriple (n : ℕ) : Prop :=
  ∃ a b c : ℕ, Nat.Prime a ∧ Nat.Prime b ∧ Nat.Prime c ∧ a + b + c = n

/-- The boundary case `7 = 2 + 2 + 3`, checked by Lean's kernel. -/
theorem seven_hasPrimeTriple : HasPrimeTriple 7 := by
  exact ⟨2, 2, 3, by decide, by decide, by decide, rfl⟩

/-- Foundation 3: a finite proof and a tail proof combine at an explicit cutoff.
Neither input is asserted to exist: both are hypotheses of this theorem. -/
theorem finite_tail_splice (cutoff : ℕ)
    (finite_part : ∀ n : ℕ, 7 ≤ n → n < cutoff → n % 2 = 1 → HasPrimeTriple n)
    (tail_part : ∀ n : ℕ, 7 ≤ n → cutoff ≤ n → n % 2 = 1 → HasPrimeTriple n) :
    ∀ n : ℕ, 7 ≤ n → n % 2 = 1 → HasPrimeTriple n := by
  intro n hn hodd
  rcases lt_or_ge n cutoff with hsmall | hlarge
  · exact finite_part n hn hsmall hodd
  · exact tail_part n hn hlarge hodd

end

end WeakGoldbach
