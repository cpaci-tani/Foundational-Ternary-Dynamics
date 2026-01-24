#!/usr/bin/env python3
"""
Global Grid Analysis

Examines the distribution of ancient megalithic sites for
potential patterns suggesting coordinated placement.

Explores the hypothesis that ancient sites form a
global network or "flux management grid."

This is speculative exploration, not verified archaeology.
"""

import math
from dataclasses import dataclass
from typing import List, Tuple, Optional


# =============================================================================
# Constants
# =============================================================================

EARTH_RADIUS = 6371  # km
EARTH_CIRCUMFERENCE = 2 * math.pi * EARTH_RADIUS  # ~40,075 km

# FTD Integers
N_C = 3
N_BASE = 4
B_3 = 7
N_EFF = 13

# Mathematical constants
PHI = (1 + math.sqrt(5)) / 2  # Golden ratio


# =============================================================================
# Site Data
# =============================================================================

@dataclass
class AncientSite:
    """An ancient megalithic or anomalous site."""
    name: str
    latitude: float  # degrees
    longitude: float  # degrees
    notes: str = ""
    age_bp: Optional[int] = None  # years before present


# Major sites (approximate coordinates)
SITES = [
    # Egypt
    AncientSite("Great Pyramid of Giza", 29.9792, 31.1342, "Precision construction", 4500),
    AncientSite("Sphinx", 29.9753, 31.1376, "Water erosion suggests older", 10000),

    # Middle East
    AncientSite("Baalbek", 34.0069, 36.2039, "1000+ ton stones", 9000),
    AncientSite("Göbekli Tepe", 37.2231, 38.9225, "12,000 years old", 12000),

    # Europe
    AncientSite("Stonehenge", 51.1789, -1.8262, "Acoustic properties", 5000),
    AncientSite("Newgrange", 53.6947, -6.4756, "Precise solar alignment", 5200),
    AncientSite("Carnac", 47.5839, -3.0775, "Thousands of standing stones", 6000),

    # Americas
    AncientSite("Teotihuacan", 19.6925, -98.8438, "City of the Gods", 2000),
    AncientSite("Tiwanaku", -16.5544, -68.6731, "High altitude precision", 4000),
    AncientSite("Puma Punku", -16.5617, -68.6803, "Machined H-blocks", 4000),
    AncientSite("Sacsayhuamán", -13.5094, -71.9817, "Polygonal masonry", 1000),
    AncientSite("Nazca Lines", -14.7350, -75.1300, "Visible from air only", 2000),

    # Asia
    AncientSite("Angkor Wat", 13.4125, 103.8670, "Astronomical encoding", 900),
    AncientSite("Yonaguni", 24.4350, 123.0119, "Underwater 'monument'", 10000),
    AncientSite("Xi'an Pyramids", 34.3833, 108.7000, "Hundreds of pyramids", 2000),

    # Pacific
    AncientSite("Easter Island", -27.1127, -109.3497, "Moai statues", 1000),
    AncientSite("Nan Madol", 6.8444, 158.3350, "Venice of the Pacific", 1000),

    # Other
    AncientSite("Great Zimbabwe", -20.2712, 30.9339, "Stone enclosures", 900),
    AncientSite("Coral Castle", 25.5003, -80.4445, "Modern anomaly", 80),
]


# =============================================================================
# Analysis Functions
# =============================================================================

def haversine_distance(lat1: float, lon1: float, lat2: float, lon2: float) -> float:
    """
    Calculate great-circle distance between two points (in km).
    """
    lat1, lon1, lat2, lon2 = map(math.radians, [lat1, lon1, lat2, lon2])

    dlat = lat2 - lat1
    dlon = lon2 - lon1

    a = math.sin(dlat/2)**2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon/2)**2
    c = 2 * math.asin(math.sqrt(a))

    return EARTH_RADIUS * c


def find_alignments(sites: List[AncientSite], tolerance_km: float = 100) -> List[Tuple]:
    """
    Find sets of 3+ sites that lie approximately on a great circle.
    """
    alignments = []

    for i, site1 in enumerate(sites):
        for j, site2 in enumerate(sites[i+1:], i+1):
            # Check if any other site lies on the great circle between these two
            aligned = [site1, site2]

            for k, site3 in enumerate(sites):
                if k == i or k == j:
                    continue

                # Calculate distance from site3 to great circle through site1-site2
                # (Simplified: check if site3 is close to the line in lat/lon space)
                # This is an approximation; true great circle math is more complex

                d12 = haversine_distance(site1.latitude, site1.longitude,
                                         site2.latitude, site2.longitude)
                d13 = haversine_distance(site1.latitude, site1.longitude,
                                         site3.latitude, site3.longitude)
                d23 = haversine_distance(site2.latitude, site2.longitude,
                                         site3.latitude, site3.longitude)

                # If d13 + d23 ≈ d12, site3 is on the line
                deviation = abs(d13 + d23 - d12)

                if deviation < tolerance_km:
                    aligned.append(site3)

            if len(aligned) >= 3:
                alignments.append(tuple(aligned))

    return alignments


def latitude_analysis(sites: List[AncientSite]) -> dict:
    """
    Analyze latitude distribution for patterns.
    """
    latitudes = [s.latitude for s in sites]

    # Check for clustering at specific latitudes
    lat_30 = [s for s in sites if 28 < abs(s.latitude) < 32]
    lat_equator = [s for s in sites if abs(s.latitude) < 5]

    # Check golden ratio relationships
    # 90° / φ ≈ 55.6°
    # 90° / φ² ≈ 34.4°
    # etc.

    phi_latitudes = {
        '90/φ': 90 / PHI,
        '90/φ²': 90 / PHI**2,
        '90/φ³': 90 / PHI**3,
        '30°': 30,  # Giza
        '90/3': 30,  # Giza (N_c)
    }

    return {
        'all_latitudes': latitudes,
        'lat_30_sites': lat_30,
        'equatorial_sites': lat_equator,
        'phi_latitudes': phi_latitudes,
    }


def distance_analysis(sites: List[AncientSite]) -> List[Tuple]:
    """
    Analyze distances between sites for patterns.
    """
    distances = []

    for i, site1 in enumerate(sites):
        for site2 in sites[i+1:]:
            d = haversine_distance(
                site1.latitude, site1.longitude,
                site2.latitude, site2.longitude
            )
            distances.append((site1.name, site2.name, d))

    return sorted(distances, key=lambda x: x[2])


def check_ftd_ratios(distances: List[Tuple]) -> List[Tuple]:
    """
    Check if any distance ratios match FTD integers.
    """
    matches = []

    ftd_values = [N_C, N_BASE, B_3, N_EFF, 2*N_EFF, PHI, PHI**2, math.pi, math.e]

    for i, (name1a, name1b, d1) in enumerate(distances):
        for name2a, name2b, d2 in distances[i+1:]:
            if d1 == 0 or d2 == 0:
                continue

            ratio = d1 / d2 if d1 > d2 else d2 / d1

            for val in ftd_values:
                if abs(ratio - val) / val < 0.05:  # Within 5%
                    matches.append((
                        f"{name1a}-{name1b}",
                        f"{name2a}-{name2b}",
                        ratio,
                        val
                    ))

    return matches


# =============================================================================
# Main Analysis
# =============================================================================

def run_analysis():
    """Run complete global grid analysis."""

    print("=" * 70)
    print("GLOBAL GRID ANALYSIS")
    print("Ancient Site Distribution Patterns")
    print("=" * 70)

    # List all sites
    print("\n--- SITE CATALOG ---")
    print(f"{'Site':<25} {'Latitude':>10} {'Longitude':>12} {'Age (BP)':>10}")
    print("-" * 60)
    for site in sorted(SITES, key=lambda s: -s.latitude):
        age = f"{site.age_bp:,}" if site.age_bp else "Unknown"
        print(f"{site.name:<25} {site.latitude:>10.4f} {site.longitude:>12.4f} {age:>10}")

    # Latitude analysis
    print("\n--- LATITUDE ANALYSIS ---")
    lat_data = latitude_analysis(SITES)

    print(f"\nSites near 30° latitude (±2°):")
    for site in lat_data['lat_30_sites']:
        print(f"  {site.name}: {site.latitude:.2f}°")

    print(f"\nFTD-significant latitudes:")
    for name, lat in lat_data['phi_latitudes'].items():
        print(f"  {name}: {lat:.2f}°")

    print(f"\nGreat Pyramid latitude: 29.9792°")
    print(f"  = 30° - 0.02° (almost exactly 1/3 of 90°)")
    print(f"  = 90° / N_c = 90° / 3 = 30° (matches N_c!)")

    # Distance analysis
    print("\n--- DISTANCE ANALYSIS ---")
    distances = distance_analysis(SITES)

    print("\nShortest distances:")
    for name1, name2, d in distances[:10]:
        print(f"  {name1} ↔ {name2}: {d:.0f} km")

    print("\nSelected significant distances:")
    giza_teotihuacan = haversine_distance(29.9792, 31.1342, 19.6925, -98.8438)
    giza_angkor = haversine_distance(29.9792, 31.1342, 13.4125, 103.8670)
    giza_easter = haversine_distance(29.9792, 31.1342, -27.1127, -109.3497)

    print(f"  Giza ↔ Teotihuacan: {giza_teotihuacan:.0f} km")
    print(f"  Giza ↔ Angkor Wat: {giza_angkor:.0f} km")
    print(f"  Giza ↔ Easter Island: {giza_easter:.0f} km")

    print(f"\nEarth circumference: {EARTH_CIRCUMFERENCE:.0f} km")
    print(f"Giza-Teotihuacan as fraction: {giza_teotihuacan/EARTH_CIRCUMFERENCE:.4f}")
    print(f"Giza-Angkor as fraction: {giza_angkor/EARTH_CIRCUMFERENCE:.4f}")

    # The famous alignment
    print("\n--- THE GIZA-ANGKOR-EASTER ALIGNMENT ---")
    print("""
A line drawn through Giza, Angkor Wat, and Easter Island
approximately follows a great circle that also passes near
Nazca, Machu Picchu, and other sites.

This "ancient equator" or "Giza meridian" is offset from
the current equator by about 30° - again, 90°/N_c.
    """)

    # FTD ratio check
    print("\n--- FTD RATIO ANALYSIS ---")
    ratio_matches = check_ftd_ratios(distances[:50])  # Check closest 50

    if ratio_matches:
        print("Distance ratios matching FTD values (within 5%):")
        for pair1, pair2, ratio, ftd_val in ratio_matches[:10]:
            print(f"  {pair1} / {pair2} = {ratio:.3f} ≈ {ftd_val:.3f}")
    else:
        print("No obvious FTD ratio matches found (may need more sites).")

    # Conclusions
    print("\n" + "=" * 70)
    print("HYPOTHESES")
    print("=" * 70)
    print("""
1. LATITUDE CLUSTERING
   Multiple major sites cluster near 30° latitude (90°/3 = 90°/N_c).
   This could indicate intentional placement related to Earth geometry.

2. THE ANCIENT EQUATOR
   A great circle through Giza, Angkor, Easter Island, and Nazca
   suggests a possible previous "equator" or intentional alignment.

3. DISTANCE RELATIONSHIPS
   Some inter-site distances may reflect FTD ratios (φ, π, N_eff),
   but more rigorous analysis is needed.

4. NETWORK HYPOTHESIS
   If pyramids are flux resonators, their placement may optimize
   global flux management through interference/reinforcement patterns.

CAVEATS:
- This analysis is preliminary and speculative
- Many sites have uncertain ages
- Coordinates are approximate
- Pattern-finding is subject to confirmation bias
- Proper statistical analysis would require random comparison sites
    """)


if __name__ == "__main__":
    run_analysis()
