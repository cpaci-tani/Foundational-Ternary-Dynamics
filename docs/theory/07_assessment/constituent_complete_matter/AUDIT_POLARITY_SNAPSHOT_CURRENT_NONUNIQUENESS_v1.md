# Audit — FTD-0719 polarity-snapshot current non-uniqueness

**Status:** `[AUDIT PASS — THEOREM SCOPE CERTIFIED]`

## Finding

FTD-0719 proves a genuine state-completeness defect for snapshot-only matter
in the selected face-current complex.  The conclusion does not depend on a
small numerical near-miss: endpoint densities, difference divergence,
transport moment, reversal residual, and covariance residuals are exactly zero
in the recorded double-precision construction, while the current and curl
norms are order `1e-1`.

The two histories are physically distinct rather than mere renamings.  They
share unordered endpoints but connect those endpoints by different causal
paths.  Their difference is a divergence-free cycle current and produces a
different transverse field update.

## Overclaim boundary

The result does not show that persistent particle labels are fundamental.  It
shows that a snapshot quotient by labels loses current-history information.
A unique implicit common-action transaction could reconstruct that information
without adding a primitive.  Until uniqueness is proved, both the derived
transaction and explicit connection-state branches remain open.

The result also does not make every divergence-free current physically
admissible.  Energy, force balance, locality, inversion, formation, and
stability remain additional gates.

## Reproducibility

- protocol: `DE13969105F196E64C61FC106945B372EBE63DA0230DB30E32526A4BC83E7B77`
- runner: `18A7B59524A5827B915551DE85F35219CFE21A6679393AB405AF67850483F2CA`
- JSON: `0B0565A11F274A8BFF4D512662BDF570304E429A7AC456806372A199FA6187C7`
- covariance CSV: `54B28CC7C366A4C85E022BA365CD3980752F1A0E8D99E163F5849881BFE84AC4`
- proof certificate: `02C78D9A3ED949DCA5D30F9054C8C9378FE52839E2B9A4A91270D4E940A0D5EA`
