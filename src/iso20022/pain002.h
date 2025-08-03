// ISO 20022 support for Notecoin
//
// This header declares a helper for building ISO 20022 Customer
// Payment Status Report messages (pain.002.001.10).  A pain.002
// message is sent in response to a payment initiation (pain.001)
// and informs the previous party in the chain about the positive or
// negative status of an instruction【474498897476160†L520-L536】.  Only a
// minimal subset of fields is implemented here for demonstration.

#ifndef NOTECOIN_ISO20022_PAIN002_H
#define NOTECOIN_ISO20022_PAIN002_H

#include <string>
#include <vector>

namespace iso20022 {

/**
 * Represents the status of a single transaction reported in a pain.002
 * message.  Each status references the original identifiers from the
 * payment initiation and provides a status code and optional reason.
 */
struct Pain002Status {
    std::string originalInstructionId; ///< Original instruction ID from pain.001
    std::string originalEndToEndId;    ///< Original end‑to‑end reference from pain.001
    std::string originalUETR;          ///< Original Unique End‑to‑End Transaction Reference
    std::string transactionStatus;     ///< ISO payment transaction status code (e.g. ACSP, RJCT, ACCC)
    std::string statusReasonCode;      ///< Optional ISO reason code for the status (e.g. MS03)
    std::string additionalInfo;        ///< Optional human‑readable explanation
};

/**
 * Build a minimal pain.002 (Customer Payment Status Report) XML message.
 *
 * @param msgId             Unique identifier for this status report
 * @param creationDateTime  ISO8601 timestamp for when the report is generated
 * @param originalMsgId     Message ID of the original pain.001 being reported upon
 * @param originalMsgNmId   Message name of the original message (e.g. "pain.001.001.08")
 * @param statuses          List of transaction statuses to include in the report
 * @return An XML string representing the pain.002 message
 */
std::string GeneratePain002(const std::string &msgId,
                             const std::string &creationDateTime,
                             const std::string &originalMsgId,
                             const std::string &originalMsgNmId,
                             const std::vector<Pain002Status> &statuses);

} // namespace iso20022

#endif // NOTECOIN_ISO20022_PAIN002_H
