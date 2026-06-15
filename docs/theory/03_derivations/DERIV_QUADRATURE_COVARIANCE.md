# DERIV - Quadrature Covariance and Edge Projection

**Tag:** [THEOREM]
**Date:** 2026-06-15
**LEDGER:** FTD-0212 [SYNTHESIS] - closes the spatial projection No-Go (FTD-0210).
**Companion docs:** `SPEC_QUADRATURE_CANONICALIZATION.md`, `SPEC_CONNECTION_EXTRACTION_RULE.md`

---

## 0. Purpose

The Quadrature Canonicalization spec (FTD-0210) established a strict "No-Go" on defining the intrinsic $\mathbb{Z}[i]$ quadrature field $z(v)$ via global, arbitrary spatial projections (e.g., $z = J_x + i J_y$). Such projections violate the 3D rotational cubic symmetry ($O_h$) of the FTD lattice by manually elevating a preferred 2D plane.

This document formally derives the canonical formulation of $z(v)$. It bypasses the No-Go by recognizing that the topological holonomy $\mathcal{L}_C = \sum_{e \in C} A_J(e)$ is evaluated *along a specific integration path*. Therefore, the identification of the complex plane is not a global static field, but an **edge-covariant tangent projection**.

---

## 1. The Edge-Covariant Formulation

In a discrete lattice connection, the connection 1-form $A_J$ evaluates over a directed edge $e$ (defined by its tangent vector $\hat{e}$). 

Instead of forcing a global complex plane, the physical $\mathbb{Z}[i]$ plane is dynamically mapped to the **local plane $P_e$ orthogonal to the path of integration**.

### 1.1 The Orthogonal Tangent Plane
For any principal cubic edge $e$, the orthogonal plane $P_e$ is spanned by the two remaining transverse basis vectors of the Moore neighborhood. 
If $\hat{e} = \hat{z}$, then $P_e = \operatorname{span}(\hat{x}, \hat{y})$. 

### 1.2 The Edge-Dependent Quadrature Field
The state-flux synthesis does not collapse the entire 3D flux vector $J(v)$ into a single scalar or pseudo-scalar. Instead, we define the quadrature field evaluated at vertex $v$ *relative to the traversal edge $e$*:

$$ z_e(v) = J(v) \big|_{P_e} $$

Equivalently, the complex number $\mathbb{Z}[i]$ is precisely the continuous dispositional flux $J$ projected into the plane transverse to the integration step:
$$ z_e(v) = J_{\perp, 1}(v) + i J_{\perp, 2}(v) $$

---

## 2. Closing the No-Go (Symmetry Preservation)

This formulation satisfies all admissibility requirements of FTD-0210:

1.  **3D Rotational Covariance:** No preferred global plane is selected. The projection is completely covariant with the cubic lattice $O_h$ symmetry. A rotation of the lattice identically rotates the integration path $C$ and its orthogonal planes $P_e$.
2.  **Topological Winding:** By projecting the continuous flux $J$ onto the transverse plane, $z_e(v)$ naturally inherits the orientational phase $\theta = \operatorname{Arg}(z_e)$ necessary for a well-defined integer winding number $n_C \neq 0$ around a dipole defect.
3.  **State Synthesis:** The scalar void/manifest state $s(v)$ bounds the available flux magnitude natively, forcing $z_e(v)$ to map geometrically onto the lemniscatic envelope as established in the primary FTD algebra.

---

## 3. The Resulting Connection

The connection extraction rule is thus fully formalized:
$$ A_J(e) = \operatorname{Arg}\left( \overline{z_e(v)} z_e(v+e) \right) $$

This $A_J$ defines a rigorous, non-zero topological sector (Phase 1) that is completely intrinsic to the $(s, J)$ FTD ontology, relying on no external mathematical injections or broken symmetries.
