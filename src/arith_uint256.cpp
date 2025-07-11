// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT

#include <arith_uint256.h>

#include <uint256.h>
#include <utilstrencodings.h>
#include <crypto/common.h>

#include <cassert>
#include <cstring>
#include <string>

template <unsigned int BITS>
base_uint<BITS>::base_uint(const std::string& str) {
    static_assert(BITS % 32 == 0 && BITS > 0, "BITS must be a positive multiple of 32.");
    SetHex(str);
}

template <unsigned int BITS>
base_uint<BITS>& base_uint<BITS>::operator<<=(unsigned int shift) {
    base_uint<BITS> a(*this);
    std::fill(std::begin(pn), std::end(pn), 0);

    int k = shift / 32;
    shift %= 32;

    for (int i = 0; i < WIDTH; ++i) {
        if (i + k + 1 < WIDTH && shift)
            pn[i + k + 1] |= (a.pn[i] >> (32 - shift));
        if (i + k < WIDTH)
            pn[i + k] |= (a.pn[i] << shift);
    }
    return *this;
}

template <unsigned int BITS>
base_uint<BITS>& base_uint<BITS>::operator>>=(unsigned int shift) {
    base_uint<BITS> a(*this);
    std::fill(std::begin(pn), std::end(pn), 0);

    int k = shift / 32;
    shift %= 32;

    for (int i = 0; i < WIDTH; ++i) {
        if (i - k - 1 >= 0 && shift)
            pn[i - k - 1] |= (a.pn[i] << (32 - shift));
        if (i - k >= 0)
            pn[i - k] |= (a.pn[i] >> shift);
    }
    return *this;
}

template <unsigned int BITS>
base_uint<BITS>& base_uint<BITS>::operator*=(uint32_t b32) {
    uint64_t carry = 0;
    for (int i = 0; i < WIDTH; ++i) {
        uint64_t n = carry + static_cast<uint64_t>(b32) * pn[i];
        pn[i] = n & 0xffffffff;
        carry = n >> 32;
    }
    return *this;
}

template <unsigned int BITS>
base_uint<BITS>& base_uint<BITS>::operator*=(const base_uint& b) {
    base_uint<BITS> a = *this;
    *this = 0;

    for (int j = 0; j < WIDTH; ++j) {
        uint64_t carry = 0;
        for (int i = 0; i + j < WIDTH; ++i) {
            uint64_t n = carry + pn[i + j] + static_cast<uint64_t>(a.pn[j]) * b.pn[i];
            pn[i + j] = n & 0xffffffff;
            carry = n >> 32;
        }
    }
    return *this;
}

template <unsigned int BITS>
base_uint<BITS>& base_uint<BITS>::operator/=(const base_uint& b) {
    if (b.IsNull()) throw uint_error("Division by zero");

    base_uint<BITS> div = b;
    base_uint<BITS> num = *this;
    *this = 0;

    int shift = num.bits() - div.bits();
    if (shift < 0) return *this;

    div <<= shift;

    while (shift >= 0) {
        if (num >= div) {
            num -= div;
            pn[shift / 32] |= (1 << (shift & 31));
        }
        div >>= 1;
        --shift;
    }
    return *this;
}

template <unsigned int BITS>
int base_uint<BITS>::CompareTo(const base_uint<BITS>& b) const {
    for (int i = WIDTH - 1; i >= 0; --i) {
        if (pn[i] < b.pn[i]) return -1;
        if (pn[i] > b.pn[i]) return 1;
    }
    return 0;
}

template <unsigned int BITS>
bool base_uint<BITS>::EqualTo(uint64_t b) const {
    for (int i = WIDTH - 1; i >= 2; --i)
        if (pn[i]) return false;

    return (pn[1] == (b >> 32)) && (pn[0] == (b & 0xffffffffUL));
}

template <unsigned int BITS>
double base_uint<BITS>::getdouble() const {
    double ret = 0.0;
    double factor = 1.0;
    for (int i = 0; i < WIDTH; ++i) {
        ret += factor * pn[i];
        factor *= 4294967296.0;
    }
    return ret;
}

template <unsigned int BITS>
std::string base_uint<BITS>::GetHex() const {
    return ArithToUint256(*this).GetHex();
}

template <unsigned int BITS>
void base_uint<BITS>::SetHex(const std::string& str) {
    *this = UintToArith256(uint256S(str));
}

template <unsigned int BITS>
std::string base_uint<BITS>::ToString() const {
    return GetHex();
}

template <unsigned int BITS>
unsigned int base_uint<BITS>::bits() const {
    for (int pos = WIDTH - 1; pos >= 0; --pos) {
        if (pn[pos]) {
            for (int n = 31; n >= 0; --n) {
                if (pn[pos] & (1U << n))
                    return 32 * pos + n + 1;
            }
        }
    }
    return 0;
}

// Explicit template instantiation for base_uint<256>
template class base_uint<256>;

// Compact format functions
arith_uint256& arith_uint256::SetCompact(uint32_t nCompact, bool* pfNegative, bool* pfOverflow) {
    int size = nCompact >> 24;
    uint32_t word = nCompact & 0x007fffff;

    if (size <= 3) {
        word >>= 8 * (3 - size);
        *this = word;
    } else {
        *this = word;
        *this <<= 8 * (size - 3);
    }

    if (pfNegative) *pfNegative = word && (nCompact & 0x00800000);
    if (pfOverflow)
        *pfOverflow = word && (size > 34 || (word > 0xff && size > 33) || (word > 0xffff && size > 32));

    return *this;
}

uint32_t arith_uint256::GetCompact(bool fNegative) const {
    int size = (bits() + 7) / 8;
    uint32_t compact = 0;

    if (size <= 3) {
        compact = GetLow64() << 8 * (3 - size);
    } else {
        arith_uint256 bn = *this >> 8 * (size - 3);
        compact = bn.GetLow64();
    }

    if (compact & 0x00800000) {
        compact >>= 8;
        ++size;
    }

    compact |= size << 24;
    if (fNegative && (compact & 0x007fffff)) compact |= 0x00800000;

    return compact;
}

// Conversion between arith_uint256 and uint256
uint256 ArithToUint256(const arith_uint256& a) {
    uint256 b;
    for (int x = 0; x < a.WIDTH; ++x)
        WriteLE32(b.begin() + x * 4, a.pn[x]);
    return b;
}

arith_uint256 UintToArith256(const uint256& a) {
    arith_uint256 b;
    for (int x = 0; x < b.WIDTH; ++x)
        b.pn[x] = ReadLE32(a.begin() + x * 4);
    return b;
}
