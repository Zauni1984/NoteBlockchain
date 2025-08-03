// ISO 20022 support for Notecoin
//
// This source file contains a simple helper for constructing a minimal
// ISO 20022 Customer Credit Transfer Initiation message (pain.001.001.08).
// It builds an XML document with the required group header and payment
// information blocks. The implementation only covers a subset of the
// mandatory elements. In a real‑world integration you would need to
// populate many more fields and adhere strictly to the ISO 20022 schemas.

#include "iso20022/pain001.h"

#include <sstream>
#include <iomanip>

namespace iso20022 {

static long double parseAmount(const std::string &amount) {
    // Parse a decimal amount string into a long double. No error handling.
    std::istringstream iss(amount);
    long double value = 0;
    iss >> value;
    return value;
}

std::string GeneratePain001(const std::string &msgId,
                             const std::string &creationDateTime,
                             const std::string &initiatingPartyName,
                             const std::vector<CreditTransfer> &transfers)
{
    // Count transactions and calculate control sum (sum of amounts)
    size_t nbTxs = transfers.size();
    long double ctrlSum = 0;
    for (const auto &t : transfers) {
        ctrlSum += parseAmount(t.amount);
    }

    // Format control sum with maximum precision (no scientific notation)
    std::ostringstream ctrlSumStream;
    ctrlSumStream << std::fixed << std::setprecision(2) << ctrlSum;

    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    xml << "<Document xmlns=\"urn:iso:std:iso:20022:tech:xsd:pain.001.001.08\">\n";
    xml << "  <CstmrCdtTrfInitn>\n";
    // Group Header
    xml << "    <GrpHdr>\n";
    xml << "      <MsgId>" << msgId << "</MsgId>\n";
    xml << "      <CreDtTm>" << creationDateTime << "</CreDtTm>\n";
    xml << "      <NbOfTxs>" << nbTxs << "</NbOfTxs>\n";
    xml << "      <CtrlSum>" << ctrlSumStream.str() << "</CtrlSum>\n";
    xml << "      <InitgPty><Nm>" << initiatingPartyName << "</Nm></InitgPty>\n";
    xml << "    </GrpHdr>\n";
    // Payment Information block
    xml << "    <PmtInf>\n";
    xml << "      <PmtInfId>" << msgId << "</PmtInfId>\n";
    xml << "      <PmtMtd>TRF</PmtMtd>\n";
    xml << "      <BtchBookg>false</BtchBookg>\n";
    xml << "      <NbOfTxs>" << nbTxs << "</NbOfTxs>\n";
    xml << "      <CtrlSum>" << ctrlSumStream.str() << "</CtrlSum>\n";
    // Execution date (use date part of creationDateTime)
    if (creationDateTime.size() >= 10) {
        xml << "      <ReqdExctnDt>" << creationDateTime.substr(0, 10) << "</ReqdExctnDt>\n";
    }
    // Debtor and debtor account (use initiating party information)
    xml << "      <Dbtr><Nm>" << initiatingPartyName << "</Nm></Dbtr>\n";
    xml << "      <DbtrAcct><Id><Othr><Id>DEBTOR</Id></Othr></Id></DbtrAcct>\n";
    // Dummy debtor agent BIC. In real scenarios this should be the BIC of the debtor’s bank.
    xml << "      <DbtrAgt><FinInstnId><BICFI>NTCBANK0XXX</BICFI></FinInstnId></DbtrAgt>\n";
    // Iterate over transfers
    for (const auto &t : transfers) {
        xml << "      <CdtTrfTxInf>\n";
        // Payment identification
        xml << "        <PmtId><InstrId>" << t.instructionId << "</InstrId><EndToEndId>" << t.endToEndId << "</EndToEndId></PmtId>\n";
        // Amount with currency
        xml << "        <Amt><InstdAmt Ccy=\"" << t.currency << "\">" << t.amount << "</InstdAmt></Amt>\n";
        // Creditor agent (dummy BIC for Notecoin network)
        xml << "        <CdtrAgt><FinInstnId><BICFI>NTCBANK0XXX</BICFI></FinInstnId></CdtrAgt>\n";
        // Creditor information
        xml << "        <Cdtr><Nm>" << t.creditorName << "</Nm></Cdtr>\n";
        xml << "        <CdtrAcct><Id><Othr><Id>" << t.creditorAccount << "</Id></Othr></Id></CdtrAcct>\n";
        xml << "      </CdtTrfTxInf>\n";
    }
    xml << "    </PmtInf>\n";
    xml << "  </CstmrCdtTrfInitn>\n";
    xml << "</Document>\n";
    return xml.str();
}

} // namespace iso20022
