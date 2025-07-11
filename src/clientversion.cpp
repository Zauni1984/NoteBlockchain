// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <clientversion.h>
#include <tinyformat.h>
#include <sstream>

// Name des Clients, der in der "version"-Nachricht gemeldet wird
const std::string CLIENT_NAME("NotebcCore");

// Optionaler Suffix zur Version
#define CLIENT_VERSION_SUFFIX ""

// === Build-Beschreibung bestimmen ===
// Optionen (in dieser Reihenfolge):
// 1. BUILD_DESC (z. B. aus build.h)
// 2. GIT_COMMIT_ID
// 3. Fallback: "vX.Y.Z.W-Notebc-A"

#ifdef HAVE_BUILD_INFO
#  include <obj/build.h>
#endif

// Für Archive-Fallback
#define GIT_ARCHIVE 1
#ifdef GIT_ARCHIVE
#  define GIT_COMMIT_ID "436aa72"
#  define GIT_COMMIT_DATE "Fri, 8 Mar 2019 17:10:11 +0530"
#endif

// Makros zur Formatierung
#define BUILD_DESC_WITH_SUFFIX(maj, min, rev, build, suffix) \
    "v" DO_STRINGIZE(maj) "." DO_STRINGIZE(min) "." DO_STRINGIZE(rev) "." DO_STRINGIZE(build) "-" DO_STRINGIZE(suffix)

#define BUILD_DESC_FROM_COMMIT(maj, min, rev, build, commit) \
    "v" DO_STRINGIZE(maj) "." DO_STRINGIZE(min) "." DO_STRINGIZE(rev) "." DO_STRINGIZE(build) "-g" commit

#define BUILD_DESC_FROM_UNKNOWN(maj, min, rev, build) \
    "v" DO_STRINGIZE(maj) "." DO_STRINGIZE(min) "." DO_STRINGIZE(rev) "." DO_STRINGIZE(build) "-Notebc-A"

// BUILD_DESC definieren
#ifndef BUILD_DESC
#  ifdef BUILD_SUFFIX
#    define BUILD_DESC BUILD_DESC_WITH_SUFFIX(CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_REVISION, CLIENT_VERSION_BUILD, BUILD_SUFFIX)
#  elif defined(GIT_COMMIT_ID)
#    define BUILD_DESC BUILD_DESC_FROM_COMMIT(CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_REVISION, CLIENT_VERSION_BUILD, GIT_COMMIT_ID)
#  else
#    define BUILD_DESC BUILD_DESC_FROM_UNKNOWN(CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_REVISION, CLIENT_VERSION_BUILD)
#  endif
#endif

// Finaler BUILD-String
const std::string CLIENT_BUILD(BUILD_DESC CLIENT_VERSION_SUFFIX);

// Interne Version formatieren
static std::string FormatVersion(int nVersion)
{
    int major = nVersion / 1000000;
    int minor = (nVersion / 10000) % 100;
    int revision = (nVersion / 100) % 100;
    int build = nVersion % 100;

    if (build == 0)
        return strprintf("%d.%d.%d", major, minor, revision);
    else
        return strprintf("%d.%d.%d.%d", major, minor, revision, build);
}

// Öffentliche Version (komplett)
std::string FormatFullVersion()
{
    return CLIENT_BUILD;
}

// Subversion nach BIP 14 (z. B. /NotebcCore:1.2.3(comment1; comment2)/ )
std::string FormatSubVersion(const std::string& name, int nClientVersion, const std::vector<std::string>& comments)
{
    std::ostringstream ss;
    ss << "/" << name << ":" << FormatVersion(nClientVersion);
    
    if (!comments.empty()) {
        ss << "(" << comments.front();
        for (size_t i = 1; i < comments.size(); ++i)
            ss << "; " << comments[i];
        ss << ")";
    }

    ss << "/";
    return ss.str();
}
