"""
The Triad at 250 Digits — Visual
Every digit rendered. Merge directions shown. The identity verified at every position.
"""

import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

from mpmath import mp, mpf, gamma, sqrt, nstr
mp.dps = 300

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib import rcParams
import numpy as np

rcParams['font.family'] = 'monospace'
rcParams['figure.dpi'] = 150

# ── Compute ──────────────────────────────────────────────────────
g1 = gamma(mpf(1)/4)
g2 = gamma(mpf(1)/2)
g34 = gamma(mpf(3)/4)

PI = g2**2
GSTAR = g1 / g34
VARPI = g1**2 / (2 * sqrt(2) * g2)
SQRT_PI = g2

# Get 252-char strings
pi_str = nstr(PI, 252, strip_zeros=False)
gs_str = nstr(GSTAR, 252, strip_zeros=False)
vp_str = nstr(VARPI, 252, strip_zeros=False)
sp_str = nstr(SQRT_PI, 252, strip_zeros=False)

# Verify merges
varpi_check = nstr(GSTAR * SQRT_PI / 2, 252, strip_zeros=False)
pi_check = nstr(4 * VARPI**2 / GSTAR**2, 252, strip_zeros=False)
gstar_check = nstr(2 * VARPI / SQRT_PI, 252, strip_zeros=False)

# ── Count matching digits ────────────────────────────────────────
def count_match(a, b):
    n = 0
    for i in range(min(len(a), len(b))):
        if a[i] == b[i]:
            if a[i] != '.':
                n += 1
        else:
            break
    return n

vp_match = count_match(vp_str, varpi_check)
pi_match = count_match(pi_str, pi_check)
gs_match = count_match(gs_str, gstar_check)

print(f"varpi merge matches: {vp_match} digits")
print(f"pi merge matches: {pi_match} digits")
print(f"G* merge matches: {gs_match} digits")

# ── Colors ───────────────────────────────────────────────────────
COL_PI = '#cc3333'
COL_GS = '#2266cc'
COL_VP = '#cc8800'
COL_SP = '#cc3366'
COL_LOCK = '#222222'
COL_CHANGE = '#cc4444'
COL_BG = '#fafafa'

# ── Figure ───────────────────────────────────────────────────────
CHARS_PER_LINE = 50
DIGIT_W = 0.018    # width per char as fraction of figure
DIGIT_H = 0.012    # height per line
BLOCK_GAP = 0.025  # gap between constant blocks

fig = plt.figure(figsize=(20, 28), facecolor='white')

def draw_digit_block(ax, y_start, label, color, digit_str, check_str=None, merge_label=None):
    """Draw a constant's digits with optional merge verification coloring."""
    # Label
    ax.text(0.01, y_start, label, fontsize=14, fontweight='bold',
            color=color, transform=ax.transAxes, verticalalignment='top')

    # Merge formula
    if merge_label:
        ax.text(0.35, y_start, merge_label, fontsize=9, color='#888',
                transform=ax.transAxes, verticalalignment='top',
                fontstyle='italic', fontfamily='serif')

    y = y_start - 0.018

    # Split into lines
    digits_only = digit_str.replace('.', '')
    decimal_pos = digit_str.index('.')
    full_str = digit_str

    line_num = 0
    char_idx = 0

    for i, ch in enumerate(full_str):
        col_in_line = char_idx % CHARS_PER_LINE
        line_num = char_idx // CHARS_PER_LINE

        x = 0.02 + col_in_line * DIGIT_W

        if ch == '.':
            # Decimal point
            ax.text(x, y - line_num * DIGIT_H, '.', fontsize=11,
                    color='#999', transform=ax.transAxes,
                    verticalalignment='top', fontfamily='monospace')
            char_idx += 1
            continue

        # Determine if this digit matches the check string
        if check_str and i < len(check_str):
            matched = (ch == check_str[i])
        else:
            matched = True

        # Color: locked (matches) = dark, changing = red
        digit_color = COL_LOCK if matched else COL_CHANGE

        # Size: first few digits larger
        digit_pos = i - (1 if i > decimal_pos else 0)  # position ignoring decimal
        if digit_pos < 1:
            fsize = 16
            fweight = 'bold'
        elif digit_pos < 3:
            fsize = 13
            fweight = 'bold'
        elif digit_pos < 8:
            fsize = 10
            fweight = 'normal'
        else:
            fsize = 8
            fweight = 'normal'

        # Background highlight for groups of 5
        pure_digit_idx = sum(1 for c in full_str[:i] if c != '.')
        if (pure_digit_idx // 5) % 2 == 0:
            bg_color = '#f0f0f5'
        else:
            bg_color = '#ffffff'

        ax.text(x, y - line_num * DIGIT_H, ch, fontsize=fsize,
                fontweight=fweight, color=digit_color,
                transform=ax.transAxes, verticalalignment='top',
                fontfamily='monospace',
                bbox=dict(boxstyle='square,pad=0.05', facecolor=bg_color,
                          edgecolor='none') if fsize >= 10 else None)

        char_idx += 1

    # Line numbers on the right
    total_lines = char_idx // CHARS_PER_LINE + 1
    for ln in range(total_lines):
        digit_start = ln * CHARS_PER_LINE
        ax.text(0.95, y - ln * DIGIT_H, f'{digit_start}', fontsize=6,
                color='#bbb', transform=ax.transAxes, verticalalignment='top',
                ha='right')

    return y - total_lines * DIGIT_H - 0.005

# Create single axes
ax = fig.add_axes([0, 0, 1, 1])
ax.axis('off')

# Title
ax.text(0.5, 0.98, 'THE TRIAD AT 250 DIGITS', fontsize=20,
        fontweight='bold', ha='center', va='top', transform=ax.transAxes,
        color='#333', fontfamily='serif')
ax.text(0.5, 0.965, 'Three constants. One identity. Every digit verified.',
        fontsize=11, ha='center', va='top', transform=ax.transAxes,
        color='#888', fontfamily='serif', fontstyle='italic')

# ── Block 1: pi ──────────────────────────────────────────────────
y = draw_digit_block(ax, 0.94, 'pi', COL_PI, pi_str,
                     pi_check, 'verified: pi = 4*varpi^2 / G*^2')

y -= BLOCK_GAP
# Merge arrow
ax.annotate('', xy=(0.5, y + 0.01), xytext=(0.5, y + BLOCK_GAP - 0.005),
            arrowprops=dict(arrowstyle='->', color='#ccc', lw=1.5),
            xycoords='axes fraction', textcoords='axes fraction')
ax.text(0.52, y + BLOCK_GAP/2, 'pi = 4 varpi^2 / G*^2', fontsize=8,
        color='#aaa', transform=ax.transAxes, fontfamily='serif',
        fontstyle='italic', va='center')

# ── Block 2: G* ──────────────────────────────────────────────────
y = draw_digit_block(ax, y, 'G*', COL_GS, gs_str,
                     gstar_check, 'verified: G* = 2*varpi / sqrt(pi)')

y -= BLOCK_GAP
ax.annotate('', xy=(0.5, y + 0.01), xytext=(0.5, y + BLOCK_GAP - 0.005),
            arrowprops=dict(arrowstyle='->', color='#ccc', lw=1.5),
            xycoords='axes fraction', textcoords='axes fraction')
ax.text(0.52, y + BLOCK_GAP/2, 'G* = Gamma(1/4) / Gamma(3/4)', fontsize=8,
        color='#aaa', transform=ax.transAxes, fontfamily='serif',
        fontstyle='italic', va='center')

# ── Block 3: varpi ───────────────────────────────────────────────
y = draw_digit_block(ax, y, 'varpi', COL_VP, vp_str,
                     varpi_check, 'verified: varpi = G* * sqrt(pi) / 2')

y -= BLOCK_GAP
ax.annotate('', xy=(0.5, y + 0.01), xytext=(0.5, y + BLOCK_GAP - 0.005),
            arrowprops=dict(arrowstyle='->', color='#ccc', lw=1.5),
            xycoords='axes fraction', textcoords='axes fraction')
ax.text(0.52, y + BLOCK_GAP/2, 'varpi = G* * sqrt(pi) / 2  =  Race 1 x Race 2 / 2',
        fontsize=8, color='#aaa', transform=ax.transAxes, fontfamily='serif',
        fontstyle='italic', va='center')

# ── Block 4: sqrt(pi) ───────────────────────────────────────────
y = draw_digit_block(ax, y, 'sqrt(pi)', COL_SP, sp_str)

# ── Bottom summary ───────────────────────────────────────────────
y -= 0.02
ax.text(0.5, y, f'All three merge directions verified to 250+ digits.',
        fontsize=12, ha='center', va='top', transform=ax.transAxes,
        color='#444', fontweight='bold', fontfamily='serif')
ax.text(0.5, y - 0.015,
        'pi = 4 varpi^2 / G*^2      varpi = G* sqrt(pi) / 2      G* = 2 varpi / sqrt(pi)',
        fontsize=10, ha='center', va='top', transform=ax.transAxes,
        color='#888', fontfamily='monospace')
ax.text(0.5, y - 0.035,
        'Race 1 (evens/odds) = sqrt(pi)   |   Race 2 (inert/split) = G*   |   Both races = varpi',
        fontsize=9, ha='center', va='top', transform=ax.transAxes,
        color='#aaa', fontfamily='serif', fontstyle='italic')

# ── Save ─────────────────────────────────────────────────────────
out_png = 'C:/Users/cpaci/Desktop/ftd/docs/papers/triad_250digits.png'
out_pdf = 'C:/Users/cpaci/Desktop/ftd/docs/papers/triad_250digits.pdf'
plt.savefig(out_png, dpi=200, bbox_inches='tight', facecolor='white')
plt.savefig(out_pdf, bbox_inches='tight', facecolor='white')
print(f"Saved: {out_png}")
print(f"Saved: {out_pdf}")
plt.show()
