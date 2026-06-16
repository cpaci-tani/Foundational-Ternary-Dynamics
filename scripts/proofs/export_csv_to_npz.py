import numpy as np
import pandas as pd
import argparse
import os

def main():
    parser = argparse.ArgumentParser(description="Convert Koopman CSV to NPZ for estimator.")
    parser.add_argument("--input", type=str, default=r"c:\Users\cpaci\Desktop\ftd\engine\tests\traj_koopman_A14.csv")
    parser.add_argument("--output", type=str, default=r"c:\Users\cpaci\Desktop\ftd\engine\tests\trajectory_A14.npz")
    args = parser.parse_args()

    print(f"Reading {args.input}...")
    if not os.path.exists(args.input):
        print("File not found.")
        return

    df = pd.read_csv(args.input)

    # The Koopman feature matrix is the PHYSICS observables only. We drop the
    # tick column AND every scale-context (sc_*) annotation column so the
    # admissibility-gate metadata never enters the operator fit. The sc_* columns
    # are carried into the .npz separately (sibling arrays) so the estimator's
    # readout-admissibility guard can read them without polluting the dynamics.
    # See docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md.
    sc_cols = [c for c in df.columns if c.startswith("sc_")]
    drop_cols = {"tick", *sc_cols}
    feature_cols = [c for c in df.columns if c not in drop_cols]
    features = df[feature_cols].values

    print(f"Extracted features shape: {features.shape}  (physics columns: {feature_cols})")

    extra = {c: df[c].values for c in sc_cols}
    if sc_cols:
        print(f"Carrying {len(sc_cols)} scale-context column(s) into the npz: {sc_cols}")
    else:
        print("No sc_* scale-context columns found (legacy CSV); npz carries features only.")

    np.savez_compressed(args.output, features=features, **extra)
    print(f"Saved to {args.output}")

if __name__ == "__main__":
    main()