// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTECOIN_ADDRDB_H
#define NOTECOIN_ADDRDB_H

#include <fs.h>
#include <serialize.h>
#include <string>
#include <map>

// Forward declarations
class CSubNet;
class CAddrMan;
class CDataStream;

/** Gründe für einen Bann */
enum BanReason : uint8_t
{
    BanReasonUnknown         = 0,
    BanReasonNodeMisbehaving = 1,
    BanReasonManuallyAdded   = 2
};

/** Eintrag in der Bannliste */
class CBanEntry
{
public:
    static constexpr int CURRENT_VERSION = 1;

    int nVersion;
    int64_t nCreateTime;
    int64_t nBanUntil;
    BanReason banReason;

    CBanEntry() { SetNull(); }

    explicit CBanEntry(int64_t nCreateTimeIn)
    {
        SetNull();
        nCreateTime = nCreateTimeIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(nVersion);
        READWRITE(nCreateTime);
        READWRITE(nBanUntil);
        READWRITE(banReason);
    }

    void SetNull()
    {
        nVersion = CURRENT_VERSION;
        nCreateTime = 0;
        nBanUntil = 0;
        banReason = BanReasonUnknown;
    }

    std::string banReasonToString() const
    {
        switch (banReason) {
            case BanReasonNodeMisbehaving: return "node misbehaving";
            case BanReasonManuallyAdded:   return "manually added";
            default:                        return "unknown";
        }
    }
};

using banmap_t = std::map<CSubNet, CBanEntry>;

/** Zugriff auf die Peer-Adresse-Datenbank (peers.dat) */
class CAddrDB
{
private:
    fs::path pathAddr;

public:
    CAddrDB();

    bool Write(const CAddrMan& addr);
    bool Read(CAddrMan& addr);
    static bool Read(CAddrMan& addr, CDataStream& ssPeers);

    // Erweiterungsvorschläge:
    bool Exists() const;
    bool Remove() const;
};

/** Zugriff auf die Bannliste (banlist.dat) */
class CBanDB
{
private:
    fs::path pathBanlist;

public:
    CBanDB();

    bool Write(const banmap_t& banSet);
    bool Read(banmap_t& banSet);
};

#endif // NOTECOIN_ADDRDB_H
