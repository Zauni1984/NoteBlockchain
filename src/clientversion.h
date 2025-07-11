// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CLIENTVERSION_H
#define BITCOIN_CLIENTVERSION_H

#if defined(HAVE_CONFIG_H)
#  include <config/bitcoin-config.h>
#endif

// === Client-Versionsinformationen prüfen ===
#if !defined(CLIENT_VERSION_MAJOR) || \
    !defined(CLIENT_VERSION_MINOR) || \
    !defined(CLIENT_VERSION_REVISION) || \
    !defined(CLIENT_VERSION_BUILD) || \
    !defined(CLIENT_VERSION_IS_RELEASE) || \
    !defined(COPYRIGHT_YEAR)
#  error "Client version information missing: Define version fields in bitcoin-config.h or elsewhere"
#endif

// === Stringify-Makros ===
#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X

// === Copyright-String für Windows .rc-Dateien ===
#define COPYRIGHT_STR "2009-" STRINGIZE(COPYRIGHT_YEAR) " " COPYRIGHT_HOLDERS_FINAL

// === Nur C++-Code, wenn nicht für windres ===
#if !defined(WINDRES_PREPROC)

#include <string>
#include <vector>

// Integer-Repräsentation der Client-Version (z. B. 1020100 für 1.2.1.0)
static const int CLIENT_VERSION =
      1000000 * CLIENT_VERSION_MAJOR +
         10000 * CLIENT_VERSION_MINOR +
           100 * CLIENT_VERSION_REVISION +
             1 * CLIENT_VERSION_BUILD;

// Externe Variablen (definiert in clientversion.cpp)
extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;

// Formatierung der Versionsinformationen
std::string FormatFullVersion();
std::string FormatSubVersion(const std::string& name, int nClientVersion, const std::vector<std::string>& comments);

#endif // !WINDRES_PREPROC

#endif // BITCOIN_CLIENTVERSION_H
