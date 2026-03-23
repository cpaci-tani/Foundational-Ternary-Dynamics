#pragma once
// tritium/trit_vector.h — Dynamic-length packed trit arrays with fast operations
//
// Backed by TritWord64 (compute format). Supports dot product via popcount
// (no multiply loop needed), Hamming distance, L0 norm, and serialization.

#include "trit.h"
#include "packing.h"
#include "threshold.h"
#include <vector>
#include <cstddef>
#include <cassert>

namespace tritium {

class TritVector {
    std::vector<TritWord64> words_;
    size_t size_;

public:
    // Construct zero-initialized vector of n trits
    explicit TritVector(size_t n = 0)
        : words_(word_count(n)), size_(n) {}

    // Construct from raw trits
    TritVector(const Trit* data, size_t n) : TritVector(n) {
        for (size_t i = 0; i < n; ++i)
            set(i, data[i]);
    }

    // Construct by quantizing doubles
    TritVector(const double* data, size_t n, double threshold) : TritVector(n) {
        batch_quantize(data, words_.data(), n, threshold);
    }

    size_t size() const { return size_; }
    size_t num_words() const { return words_.size(); }
    const TritWord64* data() const { return words_.data(); }
    TritWord64* data() { return words_.data(); }

    // Element access
    Trit operator[](size_t i) const {
        assert(i < size_);
        return words_[i / TritWord64::CAPACITY].get(
            static_cast<int>(i % TritWord64::CAPACITY));
    }

    void set(size_t i, Trit t) {
        assert(i < size_);
        words_[i / TritWord64::CAPACITY].set(
            static_cast<int>(i % TritWord64::CAPACITY), t);
    }

    // ========================================================================
    // Dot product: Σ(ai * bi) -> int
    // ========================================================================
    // Uses the 2-bit encoding property:
    //   For each trit pair (a, b), the product is:
    //     0 if either is zero
    //     +1 if both have same sign (both Pos or both Neg)
    //     -1 if signs differ (one Pos, one Neg)
    //
    // We count "agree" positions (both nonzero, same sign) -> contribute +1
    // and "disagree" positions (both nonzero, different sign) -> contribute -1
    // dot = agree_count - disagree_count

    int dot(const TritVector& other) const {
        assert(size_ == other.size_);
        int result = 0;

        for (size_t w = 0; w < words_.size(); ++w) {
            uint64_t a = words_[w].bits;
            uint64_t b = other.words_[w].bits;

            // Decompose into even/odd bits
            uint64_t a_lo = a & TritWord64::EVEN_BITS;
            uint64_t a_hi = (a >> 1) & TritWord64::EVEN_BITS;
            uint64_t b_lo = b & TritWord64::EVEN_BITS;
            uint64_t b_hi = (b >> 1) & TritWord64::EVEN_BITS;

            // Nonzero masks (1 per nonzero trit, in even bit positions)
            uint64_t a_nz = a_lo | a_hi;
            uint64_t b_nz = b_lo | b_hi;
            uint64_t both_nz = a_nz & b_nz;

            // Sign bits for nonzero trits (Neg has high bit set: 11)
            // Sign = high & low (true only for 11 = Neg)
            uint64_t a_neg = a_hi & a_lo;
            uint64_t b_neg = b_hi & b_lo;

            // Same sign: both neg or both non-neg (among nonzero pairs)
            // XOR of neg flags: 0 = same sign, 1 = different sign
            uint64_t sign_differ = (a_neg ^ b_neg) & both_nz;
            uint64_t sign_agree  = both_nz & ~sign_differ;

            int agree = popcount64(sign_agree);
            int differ = popcount64(sign_differ);
            result += agree - differ;
        }

        return result;
    }

    // ========================================================================
    // Hamming distance: count positions where a[i] != b[i]
    // ========================================================================
    int hamming(const TritVector& other) const {
        assert(size_ == other.size_);
        int count = 0;
        for (size_t w = 0; w < words_.size(); ++w) {
            uint64_t diff = words_[w].bits ^ other.words_[w].bits;
            // A pair differs if either bit differs
            uint64_t diff_lo = diff & TritWord64::EVEN_BITS;
            uint64_t diff_hi = (diff >> 1) & TritWord64::EVEN_BITS;
            uint64_t any_diff = diff_lo | diff_hi;
            count += popcount64(any_diff);
        }
        return count;
    }

    // L0 norm: count of non-zero trits
    int l0_norm() const {
        int count = 0;
        for (size_t w = 0; w < words_.size(); ++w)
            count += words_[w].popcount_nonzero();
        return count;
    }

    // ========================================================================
    // Serialization to/from storage format
    // ========================================================================
    std::vector<TritPack> pack_to() const {
        std::vector<TritPack> packs(packed_size(size_));
        pack(words_.data(), packs.data(), size_);
        return packs;
    }

    static TritVector unpack_from(const TritPack* src, size_t n) {
        TritVector v(n);
        unpack(src, v.words_.data(), n);
        return v;
    }

    // Element-wise operations returning new vectors
    TritVector operator-() const {
        TritVector result(size_);
        for (size_t w = 0; w < words_.size(); ++w) {
            uint64_t lo = words_[w].bits & TritWord64::EVEN_BITS;
            uint64_t hi = (words_[w].bits >> 1) & TritWord64::EVEN_BITS;
            uint64_t nz = (lo | hi);
            uint64_t nz_mask = nz | (nz << 1);
            result.words_[w].bits = words_[w].bits ^ (TritWord64::ODD_BITS & nz_mask);
        }
        return result;
    }
};

} // namespace tritium
