// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying file COPYING or https://opensource.org/licenses/MIT

#ifndef NOTECHAIN_ARITH_UINT256_H
#define NOTECHAIN_ARITH_UINT256_H

#include <cstdint>
#include <stdexcept>
#include <string>
#include <cstring>

class uint256;

class uint_error : public std::runtime_error {
public:
    explicit uint_error(const std::string& str) : std::runtime_error(str) {}
};

template<unsigned int BITS>
class base_uint {
protected:
    static constexpr int WIDTH = BITS / 32;
    uint32_t pn[WIDTH] = {0};

public:
    base_uint() = default;
    base_uint(uint64_t b) {
        static_assert(BITS % 32 == 0 && BITS >= 32, "BITS must be a positive multiple of 32.");
        pn[0] = static_cast<uint32_t>(b);
        pn[1] = static_cast<uint32_t>(b >> 32);
    }
    base_uint(const base_uint& b) {
        std::memcpy(pn, b.pn, sizeof(pn));
    }

    base_uint& operator=(const base_uint& b) {
        std::memcpy(pn, b.pn, sizeof(pn));
        return *this;
    }

    explicit base_uint(const std::string& str);
    base_uint& operator=(uint64_t b);

    bool operator!() const;
    double getdouble() const;

    base_uint operator~() const;
    base_uint operator-() const;

    base_uint& operator^=(const base_uint& b);
    base_uint& operator|=(const base_uint& b);
    base_uint& operator&=(const base_uint& b);
    base_uint& operator^=(uint64_t b);
    base_uint& operator|=(uint64_t b);
    base_uint& operator<<=(unsigned int shift);
    base_uint& operator>>=(unsigned int shift);

    base_uint& operator+=(const base_uint& b);
    base_uint& operator-=(const base_uint& b);
    base_uint& operator+=(uint64_t b64);
    base_uint& operator-=(uint64_t b64);
    base_uint& operator*=(uint32_t b32);
    base_uint& operator*=(const base_uint& b);
    base_uint& operator/=(const base_uint& b);

    base_uint& operator++();        // Prefix
    base_uint operator++(int);      // Postfix
    base_uint& operator--();        // Prefix
    base_uint operator--(int);      // Postfix

    int CompareTo(const base_uint& b) const;
    bool EqualTo(uint64_t b) const;

    std::string GetHex() const;
    void SetHex(const char* psz);
    void SetHex(const std::string& str);
    std::string ToString() const;

    unsigned int bits() const;
    uint64_t GetLow64() const {
        static_assert(WIDTH >= 2, "WIDTH must be at least 2.");
        return pn[0] | (static_cast<uint64_t>(pn[1]) << 32);
    }

    // Operators
    friend base_uint operator+(base_uint a, const base_uint& b) { return a += b; }
    friend base_uint operator-(base_uint a, const base_uint& b) { return a -= b; }
    friend base_uint operator*(base_uint a, const base_uint& b) { return a *= b; }
    friend base_uint operator/(base_uint a, const base_uint& b) { return a /= b; }
    friend base_uint operator|(base_uint a, const base_uint& b) { return a |= b; }
    friend base_uint operator&(base_uint a, const base_uint& b) { return a &= b; }
    friend base_uint operator^(base_uint a, const base_uint& b) { return a ^= b; }
    friend base_uint operator>>(base_uint a, int shift) { return a >>= shift; }
    friend base_uint operator<<(base_uint a, int shift) { return a <<= shift; }
    friend base_uint operator*(base_uint a, uint32_t b) { return a *= b; }

    friend bool operator==(const base_uint& a, const base_uint& b) { return std::memcmp(a.pn, b.pn, sizeof(a.pn)) == 0; }
    friend bool operator!=(const base_uint& a, const base_uint& b) { return !(a == b); }
    friend bool operator<(const base_uint& a, const base_uint& b) { return a.CompareTo(b) < 0; }
    friend bool operator>(const base_uint& a, const base_uint& b) { return a.CompareTo(b) > 0; }
    friend bool operator<=(const base_uint& a, const base_uint& b) { return a.CompareTo(b) <= 0; }
    friend bool operator>=(const base_uint& a, const base_uint& b) { return a.CompareTo(b) >= 0; }

    friend bool operator==(const base_uint& a, uint64_t b) { return a.EqualTo(b); }
    friend bool operator!=(const base_uint& a, uint64_t b) { return !a.EqualTo(b); }

    unsigned int size() const { return sizeof(pn); }
};

// Specific 256-bit unsigned integer
class arith_uint256 : public base_uint<256> {
public:
    using base_uint::base_uint;

    arith_uint256& SetCompact(uint32_t nCompact, bool* pfNegative = nullptr, bool* pfOverflow = nullptr);
    uint32_t GetCompact(bool fNegative = false) const;

    friend uint256 ArithToUint256(const arith_uint256&);
    friend arith_uint256 UintToArith256(const uint256&);
};

// Conversion functions
uint256 ArithToUint256(const arith_uint256&);
arith_uint256 UintToArith256(const uint256&);

#endif // NOTECHAIN_ARITH_UINT256_H
