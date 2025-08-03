// ISO 20022 support for Notecoin
//
// This header defines a helper to build simplified ISO 20022 FI‑to‑FI
// Customer Credit Transfer messages (pacs.008.001.08).  The goal of this
// module is to provide an XML representation of a credit transfer
// between financial institutions.  The pacs.008 message carries the
// actual payment data and is exchanged between the debtor and creditor
// agents (banks)【741064023444592†L30-L42】.  Real implementations must
// adhere strictly to the ISO 20022 schemas; this helper only covers a
// subset of mandatory fields for demonstration.

#ifndef NOTECOIN_ISO20022_PACS008_H
#define NOTECOIN_ISO20022_PACS008_H

#include <string>
#include <vector>

namespace iso20022 {

/**
 * Represents a single credit transfer between financial institutions.
 * Each transaction specifies references, amounts and party details.
 */
struct Pacs008Transaction {
    std::string instructionId;    ///< Unique identifier for the instruction
    std::string endToEndId;       ///< End‑to‑end reference carried through the payment chain
    std::string uetr;             ///< Unique End‑to‑End Transaction Reference (UETR)
    std::string amount;           ///< Settlement amount (decimal) to transfer
    std::string currency;         ///< Currency code (e.g. "NTC" for Notecoin)
    std::string debtorName;       ///< Name of the debtor customer
    std::string debtorAccount;    ///< Account identifier (e.g. IBAN or Notecoin address) of the debtor
    std::string debtorAgentBic;   ///< BIC of the debtor’s financial institution
    std::string creditorName;     ///< Name of the creditor customer
    std::string creditorAccount;  ///< Account identifier of the creditor
    std::string creditorAgentBic; ///< BIC of the creditor’s financial institution
    std::string remittanceInfo;   ///< Free‑format remittance information
};

/**
 * Build a minimal pacs.008 (FIToFICustomerCreditTransfer) XML message.
 *
 * @param msgId            Unique message identifier for the group header
 * @param creationDateTime ISO8601 timestamp for when the message is created
 * @param settlementDate   Date on which interbank settlement is expected (YYYY‑MM‑DD)
 * @param settlementMethod Settlement method code (e.g. "CLRG" for clearing, "INDA" for direct)
 * @param transactions     A list of interbank credit transfers
 * @return An XML string representing the pacs.008 message
 */
std::string GeneratePacs008(const std::string &msgId,
                             const std::string &creationDateTime,
                             const std::string &settlementDate,
                             const std::string &settlementMethod,
                             const std::vector<Pacs008Transaction> &transactions);

} // namespace iso20022

#endif // NOTECOIN_ISO20022_PACS008_H
