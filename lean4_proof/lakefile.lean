import Lake
open Lake DSL

package ftd_proof where
  leanOptions := #[
    ⟨`autoImplicit, false⟩
  ]

@[default_target]
lean_lib FTD where
  srcDir := "."
