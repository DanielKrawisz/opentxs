/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/core/stdafx.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/server/OTServer.hpp"
#include "opentxs/server/ReplyMessage.hpp"
#include "opentxs/server/UserCommandProcessor.hpp"

#define OT_METHOD "opentxs::ReplyMessage::"

namespace opentxs
{

ReplyMessage::ReplyMessage(
    const Identifier& notaryID,
    const Nym& signer,
    const Message& input,
    OTServer& server,
    const MessageType& type,
    Message& output)
    : signer_(signer)
    , original_(input)
    , notary_id_(notaryID)
    , message_(output)
    , server_(server)
    , nymfile_(input.m_strNymID)
    , init_(false)
    , drop_(false)
    , drop_status_(false)
    , sender_nym_(nullptr)
    , context_(nullptr)
{
    message_.m_strRequestNum.Set(original_.m_strRequestNum);
    message_.m_strNotaryID.Set(original_.m_strNotaryID);
    message_.m_strNymID = original_.m_strNymID;
    message_.m_strCommand = Message::ReplyCommand(type).c_str();
    message_.m_bSuccess = false;
    attach_request();
    init_ = init();
}

std::set<RequestNumber> ReplyMessage::Acknowledged() const
{
    std::set<RequestNumber> output{};
    original_.m_AcknowledgedReplies.Output(output);

    return output;
}

void ReplyMessage::attach_request()
{
    const std::string command = original_.m_strCommand.Get();
    const auto type = Message::Type(command);

    switch (type) {
        case MessageType::getMarketOffers:
        case MessageType::getMarketRecentTrades:
        case MessageType::getNymMarketOffers:
        case MessageType::registerContract:
        case MessageType::registerNym:
        case MessageType::unregisterNym:
        case MessageType::checkNym:
        case MessageType::registerInstrumentDefinition:
        case MessageType::queryInstrumentDefinitions:
        case MessageType::issueBasket:
        case MessageType::registerAccount:
        case MessageType::getBoxReceipt:
        case MessageType::getAccountData:
        case MessageType::unregisterAccount:
        case MessageType::notarizeTransaction:
        case MessageType::getNymbox:
        case MessageType::getInstrumentDefinition:
        case MessageType::getMint:
        case MessageType::processInbox:
        case MessageType::processNymbox:
        case MessageType::triggerClause:
        case MessageType::getMarketList:
        case MessageType::requestAdmin:
        case MessageType::addClaim: {
            otInfo << OT_METHOD << __FUNCTION__ << ": Attaching original "
                   << command << " message." << std::endl;
            message_.m_ascInReferenceTo.SetString(String(original_));
        } break;
        case MessageType::pingNotary:
        case MessageType::usageCredits:
        case MessageType::sendNymMessage:
        case MessageType::sendNymInstrument:
        case MessageType::getRequestNumber:
        case MessageType::getTransactionNumbers:
        default: {
        }
    }
}

void ReplyMessage::clear_request()
{
    const std::string command = original_.m_strCommand.Get();
    const auto type = Message::Type(command);

    switch (type) {
        case MessageType::checkNym:
        case MessageType::getNymbox:
        case MessageType::getAccountData:
        case MessageType::getInstrumentDefinition:
        case MessageType::getMint: {
            otInfo << OT_METHOD << __FUNCTION__ << ": Clearing original "
                   << command << " message." << std::endl;
            message_.m_ascInReferenceTo.Release();
        } break;
        case MessageType::getMarketOffers:
        case MessageType::getMarketRecentTrades:
        case MessageType::getNymMarketOffers:
        case MessageType::registerContract:
        case MessageType::registerNym:
        case MessageType::unregisterNym:
        case MessageType::registerInstrumentDefinition:
        case MessageType::queryInstrumentDefinitions:
        case MessageType::issueBasket:
        case MessageType::registerAccount:
        case MessageType::getBoxReceipt:
        case MessageType::unregisterAccount:
        case MessageType::notarizeTransaction:
        case MessageType::processInbox:
        case MessageType::processNymbox:
        case MessageType::triggerClause:
        case MessageType::getMarketList:
        case MessageType::requestAdmin:
        case MessageType::addClaim:
        case MessageType::pingNotary:
        case MessageType::usageCredits:
        case MessageType::sendNymMessage:
        case MessageType::sendNymInstrument:
        case MessageType::getRequestNumber:
        case MessageType::getTransactionNumbers:
        default: {
        }
    }
}

void ReplyMessage::ClearRequest() { message_.m_ascInReferenceTo.Release(); }

ClientContext& ReplyMessage::Context()
{
    OT_ASSERT(context_);

    return context_->It();
}

// REPLY NOTICE TO NYMBOX
//
// After specific messages, we drop a notice with a copy of the server's
// reply into the Nymbox. This way we are GUARANTEED that the Nym will
// receive and process it. (And thus never get out of sync.)
void ReplyMessage::DropToNymbox(const bool success)
{
    drop_ = true;
    drop_status_ = success;
}

bool ReplyMessage::HaveContext() const { return bool(context_); }

bool ReplyMessage::init()
{
    const Identifier senderNymID = Identifier(original_.m_strNymID);
    const Identifier purportedServerID = Identifier(original_.m_strNotaryID);

    bool out = UserCommandProcessor::check_server_lock(senderNymID);

    if (out) {
        out &= UserCommandProcessor::check_message_notary(
            purportedServerID, notary_id_);
    }

    if (out) {
        out &= UserCommandProcessor::check_client_isnt_server(
            senderNymID, signer_);
    }

    return out;
}

const bool& ReplyMessage::Init() const { return init_; }

bool ReplyMessage::init_nym()
{
    sender_nym_ = OT::App().Contract().Nym(Identifier(original_.m_strNymID));

    return bool(sender_nym_);
}

bool ReplyMessage::InitNymfileCredentials()
{
    if (false == bool(sender_nym_)) {

        return false;
    }

    return nymfile_.LoadCredentialIndex(sender_nym_->asPublicNym());
}

bool ReplyMessage::LoadContext()
{
    if (false == init_nym()) {

        return false;
    }

    context_.reset(
        new Editor<ClientContext>(OT::App().Contract().mutable_ClientContext(
            signer_.ID(), sender_nym_->ID())));

    return bool(context_);
}

bool ReplyMessage::LoadNym()
{
    auto serialized = proto::DataToProto<proto::CredentialIndex>(
        Data(original_.m_ascPayload));
    sender_nym_ = OT::App().Contract().Nym(serialized);

    return bool(sender_nym_);
}

const Message& ReplyMessage::Original() const { return original_; }

Nym& ReplyMessage::Nymfile() { return nymfile_; }

void ReplyMessage::OverrideType(const String& replyCommand)
{
    message_.m_strCommand = replyCommand;
}

void ReplyMessage::SetAccount(const String& accountID)
{
    message_.m_strAcctID = accountID;
}

void ReplyMessage::SetAcknowledgments(const ClientContext& context)
{
    message_.SetAcknowledgments(context);
}

void ReplyMessage::SetDepth(const std::int64_t depth)
{
    message_.m_lDepth = depth;
}

void ReplyMessage::SetInboxHash(const Identifier& hash)
{
    message_.m_strInboxHash = String(hash);
}

void ReplyMessage::SetInstrumentDefinitionID(const String& id)
{
    message_.m_strInstrumentDefinitionID = id;
}

void ReplyMessage::SetNymboxHash(const Identifier& hash)
{
    hash.GetString(message_.m_strNymboxHash);
}

void ReplyMessage::SetOutboxHash(const Identifier& hash)
{
    message_.m_strOutboxHash = String(hash);
}

bool ReplyMessage::SetPayload(const String& payload)
{
    return message_.m_ascPayload.SetString(payload);
}

bool ReplyMessage::SetPayload(const Data& payload)
{
    return message_.m_ascPayload.SetData(payload);
}

void ReplyMessage::SetPayload(const OTASCIIArmor& payload)
{
    message_.m_ascPayload = payload;
}

bool ReplyMessage::SetPayload2(const String& payload)
{
    return message_.m_ascPayload2.SetString(payload);
}

bool ReplyMessage::SetPayload3(const String& payload)
{
    return message_.m_ascPayload3.SetString(payload);
}

void ReplyMessage::SetRequestNumber(const RequestNumber number)
{
    message_.m_lNewRequestNum = number;
}

void ReplyMessage::SetSuccess(const bool success)
{
    message_.m_bSuccess = success;

    if (success) {
        clear_request();
    }
}

void ReplyMessage::SetTransactionNumber(const TransactionNumber& number)
{
    message_.m_lTransactionNum = number;
}

const bool& ReplyMessage::Success() const { return message_.m_bSuccess; }

void ReplyMessage::SetTargetNym(const String& nymID)
{
    message_.m_strNymID2 = nymID;
}

ReplyMessage::~ReplyMessage()
{
    message_.SignContract(signer_);
    message_.SaveContract();

    if (drop_ && context_) {
        UserCommandProcessor::drop_reply_notice_to_nymbox(
            String(message_),
            original_.m_strRequestNum.ToLong(),
            drop_status_,
            context_->It(),
            server_,
            &nymfile_);
    }

    if (context_ && context_->It().HaveLocalNymboxHash()) {
        SetNymboxHash(context_->It().LocalNymboxHash());
    }
}
}  // namespace opentxs
