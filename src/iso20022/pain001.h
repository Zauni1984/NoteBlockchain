// ISO 20022 support for Notecoin
//
// This header defines a small helper to build minimal ISO 20022
// Customer Credit Transfer Initiation messages (pain.001.001.08).
// The aim is to provide an XML representation of a credit transfer
// that can be sent to financial institutions. Note that ISO 20022
// messages are very rich and complex; this implementation covers only
// a subset of the mandatory fields for demonstration purposes.

#ifndef NOTECOIN_ISO20022_PAIN001_H
#define NOTECOIN_ISO20022_PAIN001_H

#include <string>
#include <vector>
#include <cstdint>

namespace iso20022 {

/**
 * Represents a single credit transfer instruction within a pain.001 message.
 * Each transfer moves funds from the debtor to one creditor.
 */
struct CreditTransfer {
    std::string instructionId;    ///< Unique identifier for the instruction
    std::string endToEndId;       ///< End‑to‑end reference for the payment
    std::string amount;           ///< Decimal amount to transfer (as a string)
    std::string currency;         ///< Currency code (e.g. "NTC" for Notecoin)
    std::string debtorName;       ///< Name of the debtor (sender)
    std::string debtorAccount;    ///< Account identifier for the debtor (Notecoin address)
    std::string creditorName;     ///< Name of the creditor (receiver)
    std::string creditorAccount;  ///< Account identifier for the creditor (Notecoin address)
};

/**
 * Build a minimal pain.001 (Customer Credit Transfer Initiation) XML message.
 *
 * @param msgId               Unique message identifier
 * @param creationDateTime    ISO8601 timestamp for when the message is created
 * @param initiatingPartyName Name of the party originating the file (e.g. wallet owner)
 * @param transfers           A list of credit transfers to include in the message
 * @return An XML string representing the pain.001 message
 */
std::string GeneratePain001(const std::string &msgId,
                             const std::string &creationDateTime,
                             const std::string &initiatingPartyName,
                             const std::vector<CreditTransfer> &transfers);

} // namespace iso20022

#endif // NOTECOIN_ISO20022_PAIN001_H
