// Modernized Bech32 implementation for NoteChain
// Copyright (c) 2017 Pieter Wuille
// Distributed under the MIT software license.

#include <bech32.h>

namespace {

using data = std::vector<uint8_t>;

const char* CHARSET = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

const int8_t CHARSET_REV[128] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    15,-1,10,17,21,20,26,30, 7, 5,-1,-1,-1,-1,-1,-1,
    -1,29,-1,24,13,25, 9, 8,23,-1,18,22,31,27,19,-1,
     1, 0, 3,16,11,28,12,14, 6, 4, 2,-1,-1,-1,-1,-1,
    -1,29,-1,24,13,25, 9, 8,23,-1,18,22,31,27,19,-1,
     1, 0, 3,16,11,28,12,14, 6, 4, 2,-1,-1,-1,-1,-1
};

data Cat(data x, const data& y) {
    x.insert(x.end(), y.begin(), y.end());
    return x;
}

uint32_t PolyMod(const data& v) {
    uint32_t c = 1;
    for (uint8_t v_i : v) {
        uint8_t c0 = c >> 25;
        c = ((c & 0x1FFFFFF) << 5) ^ v_i;
        if (c0 & 1)  c ^= 0x3b6a57b2;
        if (c0 & 2)  c ^= 0x26508e6d;
        if (c0 & 4)  c ^= 0x1ea119fa;
        if (c0 & 8)  c ^= 0x3d4233dd;
        if (c0 & 16) c ^= 0x2a1462b3;
    }
    return c;
}

inline unsigned char LowerCase(unsigned char c) {
    return (c >= 'A' && c <= 'Z') ? (c - 'A') + 'a' : c;
}

data ExpandHRP(const std::string& hrp) {
    data ret(hrp.size() * 2 + 1);
    for (size_t i = 0; i < hrp.size(); ++i) {
        ret[i] = hrp[i] >> 5;
        ret[i + hrp.size() + 1] = hrp[i] & 0x1F;
    }
    ret[hrp.size()] = 0;
    return ret;
}

bool VerifyChecksum(const std::string& hrp, const data& values) {
    return PolyMod(Cat(ExpandHRP(hrp), values)) == 1;
}

data CreateChecksum(const std::string& hrp, const data& values) {
    data enc = Cat(ExpandHRP(hrp), values);
    enc.resize(enc.size() + 6);
    uint32_t mod = PolyMod(enc) ^ 1;
    data ret(6);
    for (size_t i = 0; i < 6; ++i)
        ret[i] = (mod >> (5 * (5 - i))) & 31;
    return ret;
}

} // namespace

namespace bech32 {

std::string Encode(const std::string& hrp, const data& values) {
    data combined = Cat(values, CreateChecksum(hrp, values));
    std::string ret = hrp + '1';
    for (uint8_t c : combined)
        ret += CHARSET[c];
    return ret;
}

std::pair<std::string, data> Decode(const std::string& str) {
    if (str.size() < 8 || str.size() > 90) return {};

    bool lower = false, upper = false;
    for (unsigned char c : str) {
        if (c < 33 || c > 126) return {};
        if (c >= 'a' && c <= 'z') lower = true;
        if (c >= 'A' && c <= 'Z') upper = true;
    }
    if (lower && upper) return {};

    size_t pos = str.rfind('1');
    if (pos == std::string::npos || pos == 0 || pos + 7 > str.size()) return {};

    data values(str.size() - 1 - pos);
    for (size_t i = 0; i < values.size(); ++i) {
        int8_t rev = CHARSET_REV[str[i + pos + 1]];
        if (rev == -1) return {};
        values[i] = rev;
    }

    std::string hrp;
    for (size_t i = 0; i < pos; ++i)
        hrp += LowerCase(str[i]);

    if (!VerifyChecksum(hrp, values)) return {};
    return {hrp, data(values.begin(), values.end() - 6)};
}

} // namespace bech32
