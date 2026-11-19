import WeakGoldbach.Foundations
import Mathlib.Analysis.SpecialFunctions.Log.Basic

/-!
# Unproved specification interfaces

The definitions below are propositions, not theorems. No term inhabiting
either proposition is supplied. The parameters, constants and ranges must
be justified by a future analytic argument. Their compatibility with a
specific published circle-method estimate is not established here.
-/

namespace WeakGoldbach

noncomputable section

/--
An interface for a uniform major-arc approximation. A future proof must
first specify an independently defined main term (for example, a precise
singular-series expression), an error function and admissible parameters.
Allowing arbitrary inputs here does not itself provide an estimate.
-/
def MajorArcApproximation
    (Q : ℕ → ℕ) (δ : ℕ → ℝ)
    (mainTerm : ℕ → ℝ → ℂ) (error : ℕ → ℝ) (N₀ : ℕ) : Prop :=
  2 ≤ N₀ ∧
  ∀ N : ℕ, N₀ ≤ N → 0 ≤ error N ∧
    ∀ α : ℝ, α ∈ majorArc (Q N) (δ N) →
      ‖expSum N α - mainTerm N α‖ ≤ error N

/--
An explicit uniform inequality in place of the draft's invalid scalar
Big-O notation. `C` is shared by every `N` and every minor-arc point.
This is a proposed interface, not a claim that the bound holds for arbitrary
choices of `Q`, `δ`, `C` and `N₀`.
-/
def UniformMinorArcBound
    (Q : ℕ → ℕ) (δ : ℕ → ℝ) (C : ℝ) (N₀ : ℕ) : Prop :=
  0 < C ∧ 2 ≤ N₀ ∧
  ∀ N : ℕ, N₀ ≤ N →
    ∀ α : ℝ, α ∈ minorArc (Q N) (δ N) →
      ‖expSum N α‖ ≤ C * (N : ℝ) / (Real.log (N : ℝ)) ^ 2

end

end WeakGoldbach
