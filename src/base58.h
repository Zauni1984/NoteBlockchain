// Modernized base58.h
// Copyright (c) 2009-2017 Bitcoin Core Developers
// Distributed under the MIT software license.

#ifndef NOTECHAIN_BASE58_H
#define NOTECHAIN_BASE58_H

#include <string>
#include <vector>
#include <cstdint>

/** 
 * Base58 encoding/decoding:
 * - No 0, O, I, l to avoid visual ambiguity
 * - Alphanumeric only (better UX)
 * - E-mail and double-click friendly
 */

// Core encoding/decoding
std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);
std::string EncodeBase58(const std::vector<unsigned char>& vch);
bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet);
bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet);

// With checksum
std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn);
bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet);
bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet);

/** Basic base58 wrapper for encoded data (e.g. addresses, keys) */
class CBase58Data {
protected:
    std::vector<unsigned char> vchVersion;
    std::vector<unsigned char> vchData;

    void SetData(const std::vector<unsigned char>& version, const void* pdata, size_t nSize);
    void SetData(const std::vector<unsigned char>& version, const unsigned char* pbegin, const unsigned char* pend);

public:
    CBase58Data() = default;

    bool SetString(const char* psz, unsigned int versionSize = 1);
    bool SetString(const std::string& str);
    std::string ToString() const;
    int CompareTo(const CBase58Data& b58) const;

    bool operator==(const CBase58Data& b58) const { return CompareTo(b58) == 0; }
    bool operator!=(const CBase58Data& b58) const { return CompareTo(b58) != 0; }
    bool operator<(const CBase58Data& b58) const  { return CompareTo(b58) < 0; }
    bool operator<=(const CBase58Data& b58) const { return CompareTo(b58) <= 0; }
    bool operator>(const CBase58Data& b58) const  { return CompareTo(b58) > 0; }
    bool operator>=(const CBase58Data& b58) const { return CompareTo(b58) >= 0; }
};

/** Wrapper for base58-encoded private keys */
class CBitcoinSecret : public CBase58Data {
public:
    CBitcoinSecret() = default;
    explicit CBitcoinSecret(const CKey& secret);

    void SetKey(const CKey& secret);
    CKey GetKey();
    bool IsValid() const;
    bool SetString(const char* pszSecret);
    bool SetString(const std::string& strSecret);
};

/** Extended key base template */
template<typename K, int Size>
class CBitcoinExtKeyBase : public CBase58Data {
public:
    void SetKey(const K& key) {
        unsigned char vch[Size];
        key.Encode(vch);
        SetData(GetVersion(), vch, vch + Size);
    }

    K GetKey() const {
        K ret;
        if (vchData.size() == Size)
            ret.Decode(vchData.data());
        return ret;
    }

    CBitcoinExtKeyBase() = default;
    explicit CBitcoinExtKeyBase(const K& key) { SetKey(key); }
    explicit CBitcoinExtKeyBase(const std::string& base58Str) {
        SetString(base58Str.c_str(), GetVersion().size());
    }

protected:
    virtual std::vector<unsigned char> GetVersion() const = 0;
};

using CBitcoinExtKey = CBitcoinExtKeyBase<CExtKey, 74>;
using CBitcoinExtPubKey = CBitcoinExtKeyBase<CExtPubKey, 74>;

// Address & Destination Utilities
std::string EncodeDestination(const CTxDestination& dest);
CTxDestination DecodeDestination(const std::string& str);
bool IsValidDestinationString(const std::string& str);
bool IsValidDestinationString(const std::string& str, const CChainParams& params);

#endif // NOTECHAIN_BASE58_H
