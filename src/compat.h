// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license.

#ifndef BITCOIN_COMPAT_H
#define BITCOIN_COMPAT_H

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

// =======================
// Plattform: Windows
// =======================
#ifdef WIN32

    // Ziel: Windows XP (0x0501)
    #ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
    #endif
    #define _WIN32_WINNT 0x0501

    // Performance-Optimierung und Konfliktvermeidung
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN 1
    #endif

    #ifndef NOMINMAX
    #define NOMINMAX
    #endif

    // Maximale Anzahl an File Deskriptoren für FD_SET
    #ifdef FD_SETSIZE
    #undef FD_SETSIZE
    #endif
    #define FD_SETSIZE 1024

    // Windows-spezifische Includes
    #include <winsock2.h>    // vor mswsock.h und windows.h
    #include <mswsock.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #include <stdint.h>

    // POSIX-Rechte für Windows definieren (falls nicht vorhanden)
    #ifndef S_IRUSR
    #define S_IRUSR 0400
    #define S_IWUSR 0200
    #endif

#else // =======================
// Plattform: POSIX (Linux, macOS, BSD etc.)
// =======================

    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <net/if.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <ifaddrs.h>
    #include <limits.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>

    // Socket-Kompatibilitätsschicht für Nicht-Windows
    typedef unsigned int SOCKET;

    #define INVALID_SOCKET      (SOCKET)(~0)
    #define SOCKET_ERROR        -1

    #define WSAGetLastError()   errno
    #define WSAEINVAL           EINVAL
    #define WSAEALREADY         EALREADY
    #define WSAEWOULDBLOCK      EWOULDBLOCK
    #define WSAEMSGSIZE         EMSGSIZE
    #define WSAEINTR            EINTR
    #define WSAEINPROGRESS      EINPROGRESS
    #define WSAEADDRINUSE       EADDRINUSE
    #define WSAENOTSOCK         EBADF

    #define MAX_PATH            1024

#endif // WIN32

// =======================
// Microsoft Compiler spezifisch
// =======================
#ifdef _MSC_VER
    #if !defined(ssize_t)
        #ifdef _WIN64
            typedef int64_t ssize_t;
        #else
            typedef int32_t ssize_t;
        #endif
    #endif
#endif

// =======================
// Fallback für strnlen (falls nicht vorhanden)
// =======================
#if HAVE_DECL_STRNLEN == 0
#include <cstddef>
size_t strnlen(const char* start, size_t max_len);
#endif

// =======================
// Prüfung, ob ein Socket selektierbar ist
// =======================
static inline bool IsSelectableSocket(const SOCKET& s) {
#ifdef WIN32
    return true;
#else
    return (s < FD_SETSIZE);
#endif
}

#endif // BITCOIN_COMPAT_H
