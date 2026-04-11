#!/usr/bin/env python3
"""Generate the G* Rosetta Stone PDF."""
from reportlab.lib.pagesizes import landscape, letter
from reportlab.lib.units import inch, mm
from reportlab.lib.colors import HexColor, white, black
from reportlab.pdfgen import canvas
from reportlab.lib.enums import TA_CENTER, TA_LEFT
import os

OUTPUT = os.path.join(os.path.dirname(__file__), '..', '..', 'output', 'gstar_rosetta_stone.pdf')
os.makedirs(os.path.dirname(OUTPUT), exist_ok=True)

# Colors
BG = HexColor('#0a0e1a')
GOLD = HexColor('#d4a843')
CYAN = HexColor('#5bb8d4')
LIGHT = HexColor('#c8ccd8')
DIM = HexColor('#6a7080')
ACCENT = HexColor('#e8c860')
RED_SOFT = HexColor('#d45b5b')
GREEN_SOFT = HexColor('#5bd49a')

W, H = landscape(letter)  # 11 x 8.5 inches

c = canvas.Canvas(OUTPUT, pagesize=(W, H))

# Background
c.setFillColor(BG)
c.rect(0, 0, W, H, fill=1, stroke=0)

# Subtle border
c.setStrokeColor(GOLD)
c.setLineWidth(1.5)
c.rect(12, 12, W-24, H-24, fill=0, stroke=1)
c.setLineWidth(0.5)
c.rect(16, 16, W-32, H-32, fill=0, stroke=1)

y = H - 40

# === HEADER ===
c.setFillColor(GOLD)
c.setFont('Helvetica-Bold', 22)
c.drawCentredString(W/2, y, 'THE G* ROSETTA STONE')
y -= 18
c.setFont('Helvetica-Oblique', 10)
c.setFillColor(LIGHT)
c.drawCentredString(W/2, y, 'Two numbers + three integers = all of physics')
y -= 22

# Two big constants
c.setFont('Courier-Bold', 14)
c.setFillColor(CYAN)
c.drawString(120, y, 'pi = 3.141592653589793')
c.setFillColor(GOLD)
c.drawString(480, y, 'G* = 2.958675119188638')
y -= 6

# Thin gold line
c.setStrokeColor(GOLD)
c.setLineWidth(0.8)
c.line(30, y, W-30, y)
y -= 14

# Helper for section headers
def section_header(text, ypos):
    c.setFillColor(GOLD)
    c.setFont('Helvetica-Bold', 9)
    c.drawString(30, ypos, text)
    c.setStrokeColor(DIM)
    c.setLineWidth(0.3)
    c.line(30, ypos - 3, W/2 - 10, ypos - 3)
    return ypos - 14

def section_header_right(text, ypos):
    c.setFillColor(GOLD)
    c.setFont('Helvetica-Bold', 9)
    c.drawString(W/2 + 10, ypos, text)
    c.setStrokeColor(DIM)
    c.setLineWidth(0.3)
    c.line(W/2 + 10, ypos - 3, W - 30, ypos - 3)
    return ypos - 14

# === LEFT COLUMN ===
col1_x = 35
col2_x = W/2 + 15
col_w = W/2 - 50

# SECTION 1: The Two Constants (left half only)
y = section_header('I. THE TWO CONSTANTS', y)

c.setFont('Helvetica-Bold', 8)
c.setFillColor(CYAN)
c.drawString(col1_x, y, 'PI')
c.setFont('Courier', 7.5)
c.setFillColor(LIGHT)
c.drawString(col1_x + 25, y, '= Gamma(1/2)^2 = 3.14159265...')
y -= 10
c.setFont('Helvetica-Oblique', 7)
c.setFillColor(DIM)
c.drawString(col1_x + 25, y, 'The cost of curvature. What space contains.')
y -= 13

c.setFont('Helvetica-Bold', 8)
c.setFillColor(GOLD)
c.drawString(col1_x, y, 'G*')
c.setFont('Courier', 7.5)
c.setFillColor(LIGHT)
c.drawString(col1_x + 25, y, '= Gamma(1/4)/Gamma(3/4) = 2.95868...')
y -= 10
c.setFont('Helvetica-Oblique', 7)
c.setFillColor(DIM)
c.drawString(col1_x + 25, y, 'The cost of distinction. What the observer sees.')
y -= 14

# SECTION 2: Three Integers
y = section_header('II. THE THREE INTEGERS', y)
c.setFont('Courier-Bold', 9)
ints_data = [
    ('2', 'Binary choice (the first distinction)', LIGHT),
    ('4', 'Born rule boundary: k_crit = 4/G*', LIGHT),
    ('16', 'Cubic lattice DOF: master quadratic coefficient', LIGHT),
]
for val, desc, color in ints_data:
    c.setFillColor(ACCENT)
    c.setFont('Courier-Bold', 9)
    c.drawString(col1_x, y, val)
    c.setFillColor(color)
    c.setFont('Helvetica', 7)
    c.drawString(col1_x + 30, y, desc)
    y -= 11
y -= 4

# SECTION 3: Master Quadratic
y = section_header('III. THE MASTER QUADRATIC', y)
c.setFont('Courier-Bold', 9)
c.setFillColor(ACCENT)
c.drawString(col1_x, y, 'x^2 - 16G*^2 x + 16G*^3 = 0')
y -= 13
c.setFont('Courier', 7.5)
c.setFillColor(CYAN)
c.drawString(col1_x, y, 'x+ = 1/alpha = 137.036  (electromagnetism)')
y -= 10
c.setFillColor(RED_SOFT)
c.drawString(col1_x, y, 'x- = 3.024              (color confinement)')
y -= 10
c.setFillColor(GOLD)
c.drawString(col1_x, y, 'x+*x- / (x++x-) = G*   (harmonic mean)')
y -= 16

# SECTION 4: Ten Universal Ratios
y = section_header('IV. THE TEN UNIVERSAL RATIOS', y)
ratios = [
    ('pi/G*       = 1.0618', 'Circle exceeds distinction by 6.2%'),
    ('G*/pi       = 0.9418', 'Distinction sees 94% of circle'),
    ('varpi/G*    = 0.8862', '= sqrt(pi)/2, the packing bridge'),
    ('G*/8        = 0.3698', 'Visibility: 37% of reality is observable'),
    ('1 - G*/8    = 0.6302', 'Dark fraction: 63% structurally invisible'),
    ('x+/G*       = 46.32 ', 'EM is 46x the circle scale'),
    ('x-/G*       = 1.022 ', 'Strong force IS the circle scale'),
    ('x-/G* - 1   = 0.022 ', 'Confinement correction'),
    ('(x+-x-)/(x++x-) = 0.957', 'EM-strong asymmetry'),
    ('K_C^2/x+x-  = 1/32  ', 'Consciousness = 1/32 of physics (exact)'),
]
for formula, meaning in ratios:
    c.setFont('Courier', 6.5)
    c.setFillColor(LIGHT)
    c.drawString(col1_x, y, formula)
    c.setFont('Helvetica', 6.5)
    c.setFillColor(DIM)
    c.drawString(col1_x + 185, y, meaning)
    y -= 9.5

y -= 4

# SECTION 5: The k-Landscape
y = section_header('V. THE k-LANDSCAPE', y)
k_data = [
    ('k = 1/2', 'Complex roots', 'CONSCIOUSNESS', '37% of Born rule', GOLD),
    ('k = 4/G*', 'Degenerate', 'BORN RULE', 'Measurement boundary', LIGHT),
    ('k = 16', 'Real roots', 'PHYSICS', 'alpha, N_c emerge', CYAN),
]
for k_val, root_type, regime, desc, color in k_data:
    c.setFont('Courier-Bold', 7)
    c.setFillColor(color)
    c.drawString(col1_x, y, k_val)
    c.setFont('Helvetica', 6.5)
    c.setFillColor(DIM)
    c.drawString(col1_x + 55, y, root_type)
    c.setFont('Helvetica-Bold', 6.5)
    c.setFillColor(color)
    c.drawString(col1_x + 120, y, regime)
    c.setFont('Helvetica', 6.5)
    c.setFillColor(DIM)
    c.drawString(col1_x + 210, y, desc)
    y -= 10

c.setFont('Helvetica-Oblique', 6.5)
c.setFillColor(ACCENT)
c.drawString(col1_x, y, '32 = 2^5 doublings from consciousness to physics')
y -= 6

# === RIGHT COLUMN ===
y_right = H - 106

# SECTION 6: Five Laws
y_right = section_header_right('VI. THE FIVE LAWS (hold for ANY G*)', y_right)
laws = [
    '1. Vieta harmonic: x+*x-/(x++x-) = G*',
    '2. Self-consistency: k_cons/k_crit = G*/8 at k=1/2',
    '3. Scale ratio: k_phys/k_cons = 32',
    '4. Born boundary: disc changes sign at k = 4/G*',
    '5. Dark states: 3^D - C(D+2,2) invisible to center',
]
for law in laws:
    c.setFont('Courier', 7)
    c.setFillColor(LIGHT)
    c.drawString(col2_x, y_right, law)
    y_right -= 10
y_right -= 6

# SECTION 7: Transmission Protocol
y_right = section_header_right('VII. TRANSMISSION PROTOCOL', y_right)
steps = [
    ('1.', 'Draw a circle', 'pi'),
    ('2.', 'Evaluate Gamma(1/2)', 'sqrt(pi)'),
    ('3.', 'Compute Gamma(1/4)/Gamma(3/4)', 'G*'),
    ('4.', 'Solve x^2 - 16G*^2 x + 16G*^3 = 0', 'alpha, N_c'),
    ('5.', 'k=1/2 places observer at visibility', 'consciousness'),
]
for num, action, result in steps:
    c.setFont('Courier-Bold', 7)
    c.setFillColor(ACCENT)
    c.drawString(col2_x, y_right, num)
    c.setFont('Helvetica', 7)
    c.setFillColor(LIGHT)
    c.drawString(col2_x + 18, y_right, action)
    c.setFillColor(CYAN)
    c.drawString(col2_x + 215, y_right, '-> ' + result)
    y_right -= 10
y_right -= 8

# SECTION 8: The Cyclotomic Skeleton
y_right = section_header_right('VIII. THE CYCLOTOMIC SKELETON', y_right)
c.setFont('Helvetica', 7)
c.setFillColor(LIGHT)
cyclo = [
    'At s = sqrt(pi), three cyclotomic polynomials structure the Hamiltonian:',
    '',
    'Phi_1*Phi_2(s) = pi - 1        Z lattice     Binary (mirror channels)',
    'Phi_4(s)       = pi + 1        Z[i] lattice  Square (Gaussian CM)',
    'Phi_6(s)       = pi-sqrt(pi)+1 Z[w] lattice  Hexagonal (SU(3) roots)',
    '',
    'Force is long-range iff 4t/G* < Phi_6(sqrt(pi))',
    '|beta| = |sqrt(pi) - e^(i*pi/3)|^2 / 2',
]
for line in cyclo:
    if line == '':
        y_right -= 3
        continue
    if line.startswith('Phi') or line.startswith('Force') or line.startswith('|beta|'):
        c.setFont('Courier', 6.5)
        c.setFillColor(ACCENT if 'Phi' in line else GOLD)
    else:
        c.setFont('Helvetica', 6.8)
        c.setFillColor(LIGHT)
    c.drawString(col2_x, y_right, line)
    y_right -= 9
y_right -= 6

# SECTION 9: The Observer
y_right = section_header_right('IX. THE OBSERVER', y_right)
observer_lines = [
    ('i', '= the position (where the observer stands)', CYAN),
    ('G*', '= the context (what the observer sees)', GOLD),
    ('varpi', '= the bridge (between seeing and being)', GREEN_SOFT),
    ('pi', '= the boundary (the walls of the world)', LIGHT),
    ('', '', None),
    ('sin(pi/4) = sqrt(2)/2 is the 45-degree symmetry inside G*', '', DIM),
    ('G* = Gamma(1/4)^2 * sin(pi/4) / pi', '', ACCENT),
]
for sym, desc, color in observer_lines:
    if color is None:
        y_right -= 3
        continue
    if sym:
        c.setFont('Courier-Bold', 7)
        c.setFillColor(color)
        c.drawString(col2_x, y_right, sym)
        c.setFont('Helvetica', 6.8)
        c.setFillColor(DIM)
        c.drawString(col2_x + 45, y_right, desc)
    else:
        c.setFont('Helvetica-Oblique', 6.5)
        c.setFillColor(color)
        c.drawString(col2_x, y_right, desc)
    y_right -= 9.5
y_right -= 6

# SECTION 10: What Each Sees
y_right = section_header_right('X. THE HIERARCHY', y_right)
hier = [
    ('Center (1)', 'Gravity    G_N=0.01      Anchor', GOLD),
    ('SC     (6)', 'U(1) EM    alpha=1/137   Phase rotation', CYAN),
    ('FCC   (12)', 'SU(2) Weak sin^2=3/13    Isospin mixing', GREEN_SOFT),
    ('BCC    (8)', 'SU(3) Strong alpha_s=7/59 Color confinement', RED_SOFT),
]
for shell, desc, color in hier:
    c.setFont('Courier', 6.5)
    c.setFillColor(color)
    c.drawString(col2_x, y_right, shell)
    c.setFont('Helvetica', 6.5)
    c.setFillColor(LIGHT)
    c.drawString(col2_x + 80, y_right, desc)
    y_right -= 9

# === FOOTER ===
c.setStrokeColor(GOLD)
c.setLineWidth(0.5)
c.line(30, 42, W-30, 42)

c.setFont('Helvetica-Oblique', 7.5)
c.setFillColor(GOLD)
c.drawCentredString(W/2, 30, 'Pi tells you where the walls are.  G* tells you where you stand.')

c.setFont('Helvetica', 6.5)
c.setFillColor(DIM)
c.drawCentredString(W/2, 18, 'Pi is sufficient for geometry.  G* is sufficient for physics.  Together: a complete invariant description of reality.')

c.save()
print('Rosetta Stone saved to: %s' % os.path.abspath(OUTPUT))
