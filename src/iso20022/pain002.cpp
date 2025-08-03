// ISO 20022 support for Notecoin
//
// This source file implements a helper to construct simplified
// Customer Payment Status Report messages (pain.002.001.10).  A
// pain.002 message is used to inform the previous party in the
// payment chain about the positive or negative status of a
// Customer Credit Transfer Initiation【474498897476160†L520-L536】.  The
// implementation here generates only a handful of mandatory elements
// for demonstration purposes.

#include "iso20022/pain002.h"

#include <sstream>

namespace iso20022 {

std::string GeneratePain002(const std::string &msgId,
                             const std::string &creationDateTime,
                             const std::string &originalMsgId,
                             const std::string &originalMsgNmId,
                             const std::vector<Pain002Status> &statuses)
{
    size_t nbTxs = statuses.size();
    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    // Business Application Header.  For simplicity use fixed BICs for
    // sender and receiver since this helper is not aware of the bank
    // participants.  BizMsgIdr is set to msgId and message definition
    // identifier to pain.002.001.10【474498897476160†L520-L536】.
    xml << "<AppHdr>\n";
    xml << "  <Fr><FIId><FinInstnId><BICFI>NTCBANK0XXX</BICFI></FinInstnId></FIId></Fr>\n";
    xml << "  <To><FIId><FinInstnId><BICFI>NTCBANK0YYY</BICFI></FinInstnId></FIId></To>\n";
    xml << "  <BizMsgIdr>" << msgId << "</BizMsgIdr>\n";
    xml << "  <MsgDefIdr>pain.002.001.10</MsgDefIdr>\n";
    xml << "  <BizSvc>swift.cbprplus.02</BizSvc>\n";
    xml << "  <CreDt>" << creationDateTime << "</CreDt>\n";
    xml << "</AppHdr>\n";
    xml << "<Document xmlns=\"urn:iso:std:iso:20022:tech:xsd:pain.002.001.10\">\n";
    xml << "  <CstmrPmtStsRpt>\n";
    // Group Header
    xml << "    <GrpHdr>\n";
    xml << "      <MsgId>" << msgId << "</MsgId>\n";
    xml << "      <CreDtTm>" << creationDateTime << "</CreDtTm>\n";
    xml << "      <NbOfTxs>" << nbTxs << "</NbOfTxs>\n";
    // Initiating party for the report – using the notecoin network name
    xml << "      <InitgPty><Nm>Notecoin</Nm></InitgPty>\n";
    xml << "    </GrpHdr>\n";
    // Original group information and status
    xml << "    <OrgnlGrpInfAndSts>\n";
    xml << "      <OrgnlMsgId>" << originalMsgId << "</OrgnlMsgId>\n";
    xml << "      <OrgnlMsgNmId>" << originalMsgNmId << "</OrgnlMsgNmId>\n";
    // For simplicity take group status from first transaction if available
    if (!statuses.empty()) {
        xml << "      <GrpSts>" << statuses.front().transactionStatus << "</GrpSts>\n";
    }
    xml << "    </OrgnlGrpInfAndSts>\n";
    // Original payment information and status
    xml << "    <OrgnlPmtInfAndSts>\n";
    xml << "      <OrgnlPmtInfId>" << originalMsgId << "</OrgnlPmtInfId>\n";
    xml << "      <NbOfTxs>" << nbTxs << "</NbOfTxs>\n";
    // Iterate through individual transaction statuses
    for (const auto &st : statuses) {
        xml << "      <TxInfAndSts>\n";
        xml << "        <OrgnlInstrId>" << st.originalInstructionId << "</OrgnlInstrId>\n";
        xml << "        <OrgnlEndToEndId>" << st.originalEndToEndId << "</OrgnlEndToEndId>\n";
        // UETR is not formally part of pain.002 TxInfAndSts, but include as proprietary element
        xml << "        <OrgnlUETR>" << st.originalUETR << "</OrgnlUETR>\n";
        xml << "        <TxSts>" << st.transactionStatus << "</TxSts>\n";
        // Include reason code if provided
        if (!st.statusReasonCode.empty() || !st.additionalInfo.empty()) {
            xml << "        <StsRsnInf>\n";
            if (!st.statusReasonCode.empty()) {
                xml << "          <Rsn><Cd>" << st.statusReasonCode << "</Cd></Rsn>\n";
            }
            if (!st.additionalInfo.empty()) {
                xml << "          <AddtlInf>" << st.additionalInfo << "</AddtlInf>\n";
            }
            xml << "        </StsRsnInf>\n";
        }
        // Original transaction reference.  Includes the originally instructed
        // amount and requested execution date.  Since this helper does not
        // track original amounts, a zero amount is provided as a placeholder.
        xml << "        <OrgnlTxRef>\n";
        xml << "          <Amt><InstdAmt Ccy=\"NTC\">0.00</InstdAmt></Amt>\n";
        // Use the date portion of creationDateTime as requested execution date
        if (creationDateTime.size() >= 10) {
            xml << "          <ReqdExctnDt>" << creationDateTime.substr(0, 10) << "</ReqdExctnDt>\n";
        }
        xml << "        </OrgnlTxRef>\n";
        xml << "      </TxInfAndSts>\n";
    }
    xml << "    </OrgnlPmtInfAndSts>\n";
    xml << "  </CstmrPmtStsRpt>\n";
    xml << "</Document>\n";
    return xml.str();
}

} // namespace iso20022
