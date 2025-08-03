// ISO 20022 support for Notecoin
//
// This source file implements a helper for constructing minimal
// ISO 20022 FI‑to‑FI Customer Credit Transfer messages (pacs.008.001.08).
// The generated XML contains a group header with settlement
// information and a list of credit transfer transaction blocks.  Only a
// subset of fields are populated to illustrate the basic structure of
// a pacs.008 message used to move funds between financial
// institutions【741064023444592†L30-L42】.  Real systems should map all
// mandatory and optional elements defined by the ISO 20022 schema.

#include "iso20022/pacs008.h"

#include <sstream>
#include <iomanip>

namespace iso20022 {

static long double parseAmount(const std::string &amount) {
    // Parse a decimal amount string into a long double. This helper does not
    // handle locale‑specific separators or invalid input.
    std::istringstream iss(amount);
    long double value = 0;
    iss >> value;
    return value;
}

std::string GeneratePacs008(const std::string &msgId,
                             const std::string &creationDateTime,
                             const std::string &settlementDate,
                             const std::string &settlementMethod,
                             const std::vector<Pacs008Transaction> &transactions)
{
    // Compute number of transactions and total settlement amount
    size_t nbTxs = transactions.size();
    long double totalAmount = 0;
    std::string totalCurrency;
    if (!transactions.empty()) {
        totalCurrency = transactions[0].currency;
    }
    for (const auto &tx : transactions) {
        totalAmount += parseAmount(tx.amount);
    }
    std::ostringstream totalStream;
    totalStream << std::fixed << std::setprecision(2) << totalAmount;

    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    // Business Application Header (BAH).  This header conveys routing
    // information such as sender and receiver BICs, the business
    // message identifier and message definition identifier.  Using
    // debtor and creditor agent BICs from the first transaction as
    // proxies for From/To addresses【741064023444592†L30-L42】.
    if (!transactions.empty()) {
        const auto &firstTx = transactions.front();
        xml << "<AppHdr>\n";
        xml << "  <Fr><FIId><FinInstnId><BICFI>" << firstTx.debtorAgentBic << "</BICFI></FinInstnId></FIId></Fr>\n";
        xml << "  <To><FIId><FinInstnId><BICFI>" << firstTx.creditorAgentBic << "</BICFI></FinInstnId></FIId></To>\n";
        xml << "  <BizMsgIdr>" << msgId << "</BizMsgIdr>\n";
        xml << "  <MsgDefIdr>pacs.008.001.08</MsgDefIdr>\n";
        xml << "  <BizSvc>swift.cbprplus.02</BizSvc>\n";
        // Use creationDateTime date portion for AppHdr creation date
        xml << "  <CreDt>" << creationDateTime << "</CreDt>\n";
        xml << "</AppHdr>\n";
    }
    xml << "<Document xmlns=\"urn:iso:std:iso:20022:tech:xsd:pacs.008.001.08\">\n";
    xml << "  <FIToFICstmrCdtTrf>\n";
    // Group Header
    xml << "    <GrpHdr>\n";
    xml << "      <MsgId>" << msgId << "</MsgId>\n";
    xml << "      <CreDtTm>" << creationDateTime << "</CreDtTm>\n";
    xml << "      <NbOfTxs>" << nbTxs << "</NbOfTxs>\n";
    // Settlement method and date
    xml << "      <SttlmInf><SttlmMtd>" << settlementMethod << "</SttlmMtd></SttlmInf>\n";
    // Total interbank settlement amount (optional for illustration)
    if (nbTxs > 0) {
        xml << "      <TtlIntrBkSttlmAmt Ccy=\"" << totalCurrency << "\">"
            << totalStream.str() << "</TtlIntrBkSttlmAmt>\n";
    }
    xml << "      <IntrBkSttlmDt>" << settlementDate << "</IntrBkSttlmDt>\n";
    xml << "    </GrpHdr>\n";
    // Transactions
    for (const auto &tx : transactions) {
        xml << "    <CdtTrfTxInf>\n";
        // Payment identification with UETR
        xml << "      <PmtId>"
            << "<InstrId>" << tx.instructionId << "</InstrId>"
            << "<EndToEndId>" << tx.endToEndId << "</EndToEndId>"
            << "<UETR>" << tx.uetr << "</UETR>"
            << "</PmtId>\n";
        // Settlement amount
        // Payment type information.  Specifies priority and service level.
        xml << "      <PmtTpInf>\n";
        xml << "        <InstrPrty>NORM</InstrPrty>\n";
        // Service level set to SEPA for illustrative purposes.  Other codes
        // may include URGP (urgent) or SDVA (same day value).
        xml << "        <SvcLvl><Cd>SEPA</Cd></SvcLvl>\n";
        xml << "      </PmtTpInf>\n";
        // Settlement amount
        xml << "      <IntrBkSttlmAmt Ccy=\"" << tx.currency << "\">"
            << tx.amount << "</IntrBkSttlmAmt>\n";
        // Debtor
        xml << "      <Dbtr><Nm>" << tx.debtorName << "</Nm></Dbtr>\n";
        xml << "      <DbtrAcct><Id><Othr><Id>" << tx.debtorAccount << "</Id></Othr></Id></DbtrAcct>\n";
        xml << "      <DbtrAgt><FinInstnId><BICFI>" << tx.debtorAgentBic << "</BICFI></FinInstnId></DbtrAgt>\n";
        // Creditor agent and creditor
        xml << "      <CdtrAgt><FinInstnId><BICFI>" << tx.creditorAgentBic << "</BICFI></FinInstnId></CdtrAgt>\n";
        xml << "      <Cdtr><Nm>" << tx.creditorName << "</Nm></Cdtr>\n";
        xml << "      <CdtrAcct><Id><Othr><Id>" << tx.creditorAccount << "</Id></Othr></Id></CdtrAcct>\n";
        // Remittance information
        if (!tx.remittanceInfo.empty()) {
            xml << "      <RmtInf><Ustrd>" << tx.remittanceInfo << "</Ustrd></RmtInf>\n";
        }
        // Charge bearer.  Defines who pays transaction fees.  'SLEV' means
        // charges are applied at service level and is widely used in SEPA.
        xml << "      <ChrgBr>SLEV</ChrgBr>\n";
        // Category purpose code.  'OTHR' indicates other/unclassified purposes.
        xml << "      <CtgyPurp><Cd>OTHR</Cd></CtgyPurp>\n";
        xml << "    </CdtTrfTxInf>\n";
    }
    xml << "  </FIToFICstmrCdtTrf>\n";
    xml << "</Document>\n";
    return xml.str();
}

} // namespace iso20022
