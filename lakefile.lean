import Lake
open Lake DSL

package «weak-goldbach-formalization»

require mathlib from git
  "https://github.com/leanprover-community/mathlib4.git" @ "feec58a7ee9185f92abddcf7631643b53181a7d3"

@[default_target]
lean_lib WeakGoldbach
