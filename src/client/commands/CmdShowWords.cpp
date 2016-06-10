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

#include "opentxs/client/commands/CmdShowWords.hpp"
#include "opentxs/client/OTAPI.hpp"
#include "opentxs/core/Log.hpp"
#include <iostream>

namespace opentxs
{
CmdShowWords::CmdShowWords()
{
    command = "showwords";
    category = catWallet;
    help = "Show wallet BIP39 seed as a word list.";
}

CmdShowWords::~CmdShowWords()
{
}

int32_t CmdShowWords::runWithOptions()
{
    return run();
}

int32_t CmdShowWords::run()
{
    const std::string words = OTAPI_Wrap::Wallet_GetWords();
    const bool empty = 1 > words.size();
    std::cout << words << std::endl;

    return empty ? -1 : 0;
}
} // namespace opentxs
