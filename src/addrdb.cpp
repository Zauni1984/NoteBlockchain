// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <addrdb.h>
#include <addrman.h>
#include <chainparams.h>
#include <clientversion.h>
#include <hash.h>
#include <random.h>
#include <streams.h>
#include <tinyformat.h>
#include <util.h>
#include <logging.h> // optional, falls LogPrintf genutzt wird

namespace {

template <typename Stream, typename Data>
bool SerializeDB(Stream& stream, const Data& data)
{
    try {
        CHashWriter hasher(SER_DISK, CLIENT_VERSION);
        stream << FLATDATA(Params().MessageStart()) << data;
        hasher << FLATDATA(Params().MessageStart()) << data;
        stream << hasher.GetHash();
    } catch (const std::exception& e) {
        LogPrintf("SerializeDB() failed: %s\n", e.what());
        return error("%s: Serialize or I/O error - %s", __func__, e.what());
    }
    return true;
}

template <typename Data>
bool SerializeFileDB(const std::string& prefix, const fs::path& path, const Data& data)
{
    // Sicheren temporären Dateinamen generieren
    uint16_t randv = 0;
    GetRandBytes(reinterpret_cast<unsigned char*>(&randv), sizeof(randv));
    std::string tmpfn = strprintf("%s.%04x_%llu", prefix, randv, GetTimeMicros());
    fs::path pathTmp = GetDataDir() / tmpfn;

    FILE* file = fsbridge::fopen(pathTmp, "wb");
    if (!file) {
        return error("%s: Failed to open file %s", __func__, pathTmp.string());
    }

    CAutoFile fileout(file, SER_DISK, CLIENT_VERSION);
    if (!SerializeDB(fileout, data)) return false;
    FileCommit(fileout.Get());
    fileout.fclose();

    if (!RenameOver(pathTmp, path)) {
        return error("%s: Rename-into-place failed", __func__);
    }

    return true;
}

template <typename Stream, typename Data>
bool DeserializeDB(Stream& stream, Data& data, bool fCheckSum = true)
{
    try {
        CHashVerifier<Stream> verifier(&stream);
        unsigned char pchMsgTmp[4];
        verifier >> FLATDATA(pchMsgTmp);

        if (memcmp(pchMsgTmp, Params().MessageStart(), sizeof(pchMsgTmp)) != 0) {
            return error("%s: Invalid network magic number", __func__);
        }

        verifier >> data;

        if (fCheckSum) {
            uint256 hashTmp;
            stream >> hashTmp;
            if (hashTmp != verifier.GetHash()) {
                return error("%s: Checksum mismatch, data corrupted", __func__);
            }
        }
    } catch (const std::exception& e) {
        LogPrintf("DeserializeDB() failed: %s\n", e.what());
        return error("%s: Deserialize or I/O error - %s", __func__, e.what());
    }

    return true;
}

template <typename Data>
bool DeserializeFileDB(const fs::path& path, Data& data)
{
    FILE* file = fsbridge::fopen(path, "rb");
    if (!file) {
        LogPrintf("DeserializeFileDB(): Failed to open file %s\n", path.string());
        return error("%s: Failed to open file %s", __func__, path.string());
    }

    CAutoFile filein(file, SER_DISK, CLIENT_VERSION);
    return DeserializeDB(filein, data);
}

} // namespace


CBanDB::CBanDB()
{
    pathBanlist = GetDataDir() / "banlist.dat";
}

bool CBanDB::Write(const banmap_t& banSet)
{
    return SerializeFileDB("banlist", pathBanlist, banSet);
}

bool CBanDB::Read(banmap_t& banSet)
{
    if (!fs::exists(pathBanlist)) {
        LogPrintf("CBanDB::Read(): No banlist found.\n");
        return true;
    }

    return DeserializeFileDB(pathBanlist, banSet);
}


CAddrDB::CAddrDB()
{
    pathAddr = GetDataDir() / "peers.dat";
}

bool CAddrDB::Write(const CAddrMan& addr)
{
    return SerializeFileDB("peers", pathAddr, addr);
}

bool CAddrDB::Read(CAddrMan& addr)
{
    if (!fs::exists(pathAddr)) {
        LogPrintf("CAddrDB::Read(): No peers.dat found, starting with empty address list.\n");
        return true;
    }

    return DeserializeFileDB(pathAddr, addr);
}

bool CAddrDB::Read(CAddrMan& addr, CDataStream& ssPeers)
{
    bool ret = DeserializeDB(ssPeers, addr, false);
    if (!ret) {
        LogPrintf("CAddrDB::Read(): Failed to deserialize peer data from stream, clearing addrman.\n");
        addr.Clear();
    }
    return ret;
}

// Erweiterung: optional zusätzliche Methoden
bool CAddrDB::Exists() const {
    return fs::exists(pathAddr);
}

bool CAddrDB::Remove() const {
    return fs::remove(pathAddr);
}
