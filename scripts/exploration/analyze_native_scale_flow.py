import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import argparse

def analyze(csv_path):
    df = pd.read_csv(csv_path)
    print(f"Loaded {len(df)} rows from {csv_path}")

    # Group by b and compute means
    means = df.groupby('b').mean()
    print("\nTime-Averaged Means:")
    print(means[['e_flux', 'v_coupling', 'total_q', 'total_sq', 'reaction_l1']])

    fig, axs = plt.subplots(3, 1, figsize=(10, 12), sharex=True)

    for b in df['b'].unique():
        sub = df[df['b'] == b]
        axs[0].plot(sub['tick'], sub['e_flux'], label=f"b={b}", alpha=0.7)
        axs[1].plot(sub['tick'], sub['v_coupling'], label=f"b={b}", alpha=0.7)
        axs[2].plot(sub['tick'], sub['reaction_l1'], label=f"b={b}", alpha=0.7)

    axs[0].set_ylabel('Flux Energy $E$')
    axs[0].legend()
    axs[0].set_title('Canonical Flux Energy (Scale Flow)')

    axs[1].set_ylabel('Vertex Coupling $V$')
    axs[1].legend()
    axs[1].set_title('Current-Flux Vertex Coupling')

    axs[2].set_ylabel('Total Reaction $S_R$')
    axs[2].set_xlabel('Tick')
    axs[2].legend()
    axs[2].set_title('Continuity Reaction Sources')

    plt.tight_layout()
    plt.savefig('native_scale_flow_plot.png', dpi=300)
    print("Saved plot to native_scale_flow_plot.png")

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--csv', type=str, default='native_scale_flow_telemetry.csv')
    args = parser.parse_args()
    analyze(args.csv)
