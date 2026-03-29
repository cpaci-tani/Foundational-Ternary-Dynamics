"""
Prime Race to 1 Trillion — Segmented Sieve

Tracks the race between primes ≡ 1 (mod 4) and ≡ 3 (mod 4)
up to 10^12, looking for the Bays-Hudson crossover at ~609 billion.

Uses a memory-efficient segmented sieve (~50 MB RAM).
Outputs progress every 10 billion.
"""

import math
import time
import sys

def small_sieve(limit):
    """Standard sieve up to limit. Returns list of primes."""
    is_prime = bytearray(b'\x01') * (limit + 1)
    is_prime[0] = is_prime[1] = 0
    for i in range(2, int(math.sqrt(limit)) + 1):
        if is_prime[i]:
            for j in range(i*i, limit + 1, i):
                is_prime[j] = 0
    return [i for i in range(2, limit + 1) if is_prime[i]]

def prime_race(N, report_interval=10_000_000_000):
    """
    Count primes ≡ 1 and ≡ 3 (mod 4) up to N using segmented sieve.
    Track lead changes and gap statistics.
    """
    sqrt_N = int(math.sqrt(N)) + 1
    seg_size = max(sqrt_N, 1 << 19)  # at least 512K per segment

    print(f"Sieving small primes up to {sqrt_N:,}...")
    t0 = time.time()
    small_primes = small_sieve(sqrt_N)
    print(f"  Found {len(small_primes):,} small primes in {time.time()-t0:.1f}s")
    print()

    c1 = 0  # count of primes ≡ 1 (mod 4)
    c3 = 0  # count of primes ≡ 3 (mod 4)
    leader_is_inert = None  # True if 3mod4 leads
    lead_changes = []
    max_gap_inert = 0  # max (c3 - c1)
    max_gap_split = 0  # max (c1 - c3)
    last_report = 0

    # Handle p=2 and p=3 (first primes)
    # p=2: skip (neither 1 nor 3 mod 4 in the relevant sense)
    # p=3: 3 mod 4 → c3
    c3 = 1
    leader_is_inert = True

    # Handle remaining small primes
    for p in small_primes:
        if p <= 3:
            continue
        if p % 4 == 1:
            c1 += 1
        else:
            c3 += 1

        gap = c3 - c1
        now_inert = gap > 0
        if gap != 0 and leader_is_inert is not None and now_inert != leader_is_inert:
            lead_changes.append((p, c1, c3))
            leader_is_inert = now_inert
        elif gap != 0:
            leader_is_inert = now_inert

        if gap > max_gap_inert:
            max_gap_inert = gap
        if -gap > max_gap_split:
            max_gap_split = -gap

    print(f"After small primes (up to {sqrt_N:,}):")
    print(f"  split={c1:,}, inert={c3:,}, gap={c3-c1:+,}, changes={len(lead_changes)}")
    print()

    # Segmented sieve for primes > sqrt_N
    t_start = time.time()
    low = sqrt_N + 1
    if low % 2 == 0:
        low += 1  # start odd

    processed = sqrt_N
    total_segments = 0

    print(f"Starting segmented sieve from {low:,} to {N:,}...")
    print(f"Segment size: {seg_size:,}")
    print()

    while low <= N:
        high = min(low + seg_size * 2, N + 1)  # *2 because we store odd only

        # Create segment for odd numbers in [low, high)
        # seg[i] = 1 means (low + 2*i) is potentially prime
        seg_len = (high - low + 1) // 2
        seg = bytearray(b'\x01') * seg_len

        # Sieve with small primes
        for p in small_primes:
            if p < 3:
                continue
            # Find first odd multiple of p >= low
            start = ((low + p - 1) // p) * p
            if start % 2 == 0:
                start += p
            if start < p * p:
                start = p * p
            if start > high:
                continue

            idx = (start - low) // 2
            step = p  # step in index space (p * 2 / 2)
            while idx < seg_len:
                seg[idx] = 0
                idx += p

        # Count primes in this segment
        for i in range(seg_len):
            if seg[i]:
                p = low + 2 * i
                if p > N:
                    break
                if p % 4 == 1:
                    c1 += 1
                else:
                    c3 += 1

                gap = c3 - c1
                now_inert = gap > 0
                if gap != 0 and leader_is_inert is not None and now_inert != leader_is_inert:
                    lead_changes.append((p, c1, c3))
                    leader_is_inert = now_inert
                    # Print lead changes near the Bays-Hudson region
                    if p > 600_000_000_000:
                        print(f"  *** LEAD CHANGE at p={p:,}: split={c1:,}, inert={c3:,}, gap={gap:+,} ***")

                if gap > max_gap_inert:
                    max_gap_inert = gap
                if -gap > max_gap_split:
                    max_gap_split = -gap

        processed = min(high - 1, N)
        total_segments += 1

        # Progress report
        if processed - last_report >= report_interval or processed >= N:
            elapsed = time.time() - t_start
            rate = processed / elapsed if elapsed > 0 else 0
            eta = (N - processed) / rate if rate > 0 else 0
            gap = c3 - c1
            leader = "INERT" if gap > 0 else "SPLIT"
            total = c1 + c3

            print(f"  {processed/1e9:8.1f}B / {N/1e9:.0f}B  "
                  f"split={c1:>13,}  inert={c3:>13,}  gap={gap:>+8,}  {leader:5s}  "
                  f"changes={len(lead_changes):,}  "
                  f"{elapsed:.0f}s  ETA {eta:.0f}s")
            sys.stdout.flush()
            last_report = processed

        low = high
        if low % 2 == 0:
            low += 1

    elapsed_total = time.time() - t_start

    print()
    print("=" * 80)
    print(f"PRIME RACE COMPLETE: N = {N:,}")
    print("=" * 80)
    print()
    print(f"Total primes (odd): {c1+c3:,}")
    print(f"  Split (1 mod 4):  {c1:,}  ({c1/(c1+c3)*100:.6f}%)")
    print(f"  Inert (3 mod 4):  {c3:,}  ({c3/(c1+c3)*100:.6f}%)")
    print(f"  Final gap:        {c3-c1:+,}")
    print(f"  Leader at end:    {'INERT' if c3 > c1 else 'SPLIT'}")
    print()
    print(f"Lead changes:       {len(lead_changes):,}")
    print(f"Max inert advantage: +{max_gap_inert:,}")
    print(f"Max split advantage: +{max_gap_split:,}")
    print()
    print(f"Time: {elapsed_total:.1f} seconds ({elapsed_total/60:.1f} minutes)")
    print()

    if lead_changes:
        # Show lead changes near 609 billion
        bh_changes = [(p, s, i) for p, s, i in lead_changes if p > 500_000_000_000]
        if bh_changes:
            print("LEAD CHANGES NEAR BAYS-HUDSON REGION (p > 500B):")
            for p, s, i in bh_changes[:20]:
                gap = i - s
                leader = "INERT" if gap > 0 else "SPLIT"
                print(f"  p={p:>15,}  split={s:>13,}  inert={i:>13,}  gap={gap:>+6,}  {leader}")
            if len(bh_changes) > 20:
                print(f"  ... {len(bh_changes)-20} more")
        else:
            print("No lead changes found above 500 billion.")
            print("The Bays-Hudson crossover may be at a slightly different location")
            print("than the originally computed ~609 billion.")

    return c1, c3, lead_changes


if __name__ == '__main__':
    # Start with 100 billion as a test, then go to 1 trillion
    import sys

    if len(sys.argv) > 1:
        target = int(float(sys.argv[1]))
    else:
        target = 100_000_000_000  # 100 billion first

    print(f"Target: {target:,}")
    print()
    prime_race(target)
