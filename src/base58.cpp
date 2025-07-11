// Modernized base58.cpp
// Copyright (c) 2014-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#include <base58.h>
#include <bech32.h>
#include <hash.h>
#include <script/script.h>
#include <uint256.h>
#include <utilstrencodings.h>

#include <boost/variant.hpp>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>

namespace {

const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch) {
    while (*psz && isspace(*psz)) ++psz;

    int zeroes = 0;
    while (*psz == '1') {
        ++zeroes;
        ++psz;
    }

    int size = strlen(psz) * 733 / 1000 + 1;
    std::vector<unsigned char> b256(size);

    int length = 0;
    while (*psz && !isspace(*psz)) {
        const char* ch = strchr(pszBase58, *psz);
        if (!ch) return false;

        int carry = ch - pszBase58;
        for (int i = 0; carry != 0 || i < length; ++i) {
            carry += 58 * b256[size - 1 - i];
            b256[size - 1 - i] = carry % 256;
            carry /= 256;
        }
        length++;
        ++psz;
    }

    while (isspace(*psz)) ++psz;
    if (*psz != 0) return false;

    auto it = b256.begin() + (size - length);
    while (it != b256.end() && *it == 0) ++it;

    vch.assign(zeroes, 0x00);
    vch.insert(vch.end(), it, b256.end());
    return true;
}

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend) {
    int zeroes = 0;
    while (pbegin != pend && *pbegin == 0) {
        ++pbegin;
        ++zeroes;
    }

    int size = (pend - pbegin) * 138 / 100 + 1;
    std::vector<unsigned char> b58(size);

    int length = 0;
    while (pbegin != pend) {
        int carry = *pbegin;
        for (int i = 0; carry != 0 || i < length; ++i) {
            carry += 256 * b58[size - 1 - i];
            b58[size - 1 - i] = carry % 58;
            carry /= 58;
        }
        length++;
        ++pbegin;
    }

    auto it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0) ++it;

    std::string result(zeroes, '1');
    while (it != b58.end()) result += pszBase58[*(it++)];
    return result;
}

} // namespace

std::string EncodeBase58(const std::vector<unsigned char>& vch) {
    return EncodeBase58(vch.data(), vch.data() + vch.size());
}

bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet) {
    return DecodeBase58(str.c_str(), vchRet);
}

std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn) {
    std::vector<unsigned char> vch(vchIn);
    uint256 hash = Hash(vch.begin(), vch.end());
    vch.insert(vch.end(), (unsigned char*)&hash, (unsigned char*)&hash + 4);
    return EncodeBase58(vch);
}

bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet) {
    if (!DecodeBase58(psz, vchRet) || vchRet.size() < 4) {
        vchRet.clear();
        return false;
    }

    uint256 hash = Hash(vchRet.begin(), vchRet.end() - 4);
    if (memcmp(&hash, &vchRet[vchRet.size() - 4], 4) != 0) {
        vchRet.clear();
        return false;
    }

    vchRet.resize(vchRet.size() - 4);
    return true;
}

bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet) {
    return DecodeBase58Check(str.c_str(), vchRet);
}
