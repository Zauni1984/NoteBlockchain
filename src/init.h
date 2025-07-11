// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2025 NoteCoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTECOIN_INIT_H
#define NOTECOIN_INIT_H

#include <string>

class CScheduler;

namespace boost {
class thread_group;
} // namespace boost

// Shutdown-Steuerung
void StartShutdown();
bool ShutdownRequested();
void Interrupt();
void Shutdown();

// Logging & Parameterhandling
void InitLogging();                   // Logging-Infrastruktur initialisieren
void InitParameterInteraction();      // Interaktionen und Abhängigkeiten von Parametern prüfen

// Setup-Phasen der Anwendung
bool AppInitBasicSetup();             // Initialer System-Setup (z. B. Umgebungsprüfung)
bool AppInitParameterInteraction();  // Abhängigkeiten von Konfigurationsparametern
bool AppInitSanityChecks();          // Sanity Checks wie ECC, Datei-Zugriffe usw.
bool AppInitLockDataDirectory();     // Sperren des Datenverzeichnisses
bool AppInitMain();                  // Hauptinitialisierung nach Daemonisierung

// SeedPhrase-Manager (für zukünftige Wallet-Wiederherstellung)
bool InitSeedManager();              // Initialisiert das Seed-Verwaltungssystem
void ShutdownSeedManager();          // Beendet das Seed-System kontrolliert

// Hilfe & Lizenzinformationen
enum HelpMessageMode {
    HMM_BITCOIND,
    HMM_BITCOIN_QT
};

std::string HelpMessage(HelpMessageMode mode);
std::string LicenseInfo();

#endif // NOTECOIN_INIT_H
