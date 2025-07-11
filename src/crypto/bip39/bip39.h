// bip39.h – Public domain BIP39 implementation (minimalistic)
#ifndef BIP39_H
#define BIP39_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Wandelt Entropie in eine BIP39 Mnemonic um.
 * 
 * @param entropy - Eingabe: Byte-Array mit z. B. 16, 20, 24, 28 oder 32 Bytes.
 * @param entropy_len - Länge der Entropie in Bytes.
 * @param mnemonic - Ausgabe: String-Zeiger für Mnemonic (Zeichenkette).
 * @param mnemonic_len - Maximale Länge der Ausgabe.
 * @return 1 bei Erfolg, 0 bei Fehler.
 */
int bip39_mnemonic_from_bytes(const uint8_t *entropy, size_t entropy_len, char *mnemonic, size_t mnemonic_len);

/**
 * Überprüft eine gegebene Mnemonic auf Gültigkeit.
 *
 * @param mnemonic - Eingabe-Mnemonic
 * @return 1 wenn gültig, 0 wenn ungültig.
 */
int bip39_mnemonic_check(const char *mnemonic);

/**
 * Konvertiert eine Mnemonic + optionale Passphrase in einen 512-bit Seed.
 *
 * @param mnemonic - Die Mnemonic Zeichenkette.
 * @param passphrase - Optionales Passwort (z. B. "TREZOR").
 * @param out_seed - Ausgabe-Puffer für den Seed (64 Bytes!).
 * @param out_len - Optionaler Rückgabewert für Seed-Länge.
 * @return 1 bei Erfolg, 0 bei Fehler.
 */
int bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase, uint8_t *out_seed, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif // BIP39_H
