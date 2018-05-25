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

#ifndef OPENTXS_UI_PROFILE_SECTION_HPP
#define OPENTXS_UI_PROFILE_SECTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/ListRow.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
#include <algorithm>
#include <tuple>
#include <vector>

// clang-format off
%template(UIProfileItemType) std::pair<int, std::string>;
%template(UIProfileTypeList) std::vector<std::pair<int, std::string>>;
%extend opentxs::ui::ProfileSection {
    bool AddClaim(
        const int type,
        const std::string& value,
        const bool primary,
        const bool active) const
    {
        return $self->AddClaim(
            static_cast<opentxs::proto::ContactItemType>(type),
            value,
            primary,
            active);
    }
    static std::vector<std::pair<int, std::string>> AllowedItemTypes(
        const int section,
        const std::string& lang)
    {
        const auto types = opentxs::ui::ProfileSection::AllowedItems(
            static_cast<opentxs::proto::ContactSectionName>(section),
            lang);
        std::vector<std::pair<int, std::string>> output;
        std::transform(
            types.begin(),
            types.end(),
            std::inserter(output, output.end()),
            [](std::pair<opentxs::proto::ContactItemType, std::string> type) ->
                std::pair<int, std::string> {
                    return {static_cast<int>(type.first), type.second};} );

        return output;
    }
    std::vector<std::pair<int, std::string>> Items(
        const std::string& lang) const
    {
        const auto types =
            opentxs::ui::ProfileSection::AllowedItems($self->Type(), lang);
        std::vector<std::pair<int, std::string>> output;
        std::transform(
            types.begin(),
            types.end(),
            std::inserter(output, output.end()),
            [](std::pair<opentxs::proto::ContactItemType, std::string> type) ->
                std::pair<int, std::string> {
                    return {static_cast<int>(type.first), type.second};} );

        return output;
    }
    int Type() const
    {
        return static_cast<int>($self->Type());
    }
}
%ignore opentxs::ui::ProfileSection::AddClaim;
%ignore opentxs::ui::ProfileSection::AllowedItems;
%ignore opentxs::ui::ProfileSection::Items;
%ignore opentxs::ui::ProfileSection::Type;
%ignore opentxs::ui::ProfileSection::Update;
%shared_ptr(opentxs::ui::ProfileSubsection);
%rename(UIProfileSection) opentxs::ui::ProfileSection;
// clang-format on
#endif  // SWIG

extern template struct std::pair<int, std::string>;

namespace opentxs
{
namespace ui
{
class ProfileSection : virtual public ListRow
{
public:
    using ItemType = std::pair<proto::ContactItemType, std::string>;
    using ItemTypeList = std::vector<ItemType>;

    EXPORT static ItemTypeList AllowedItems(
        const proto::ContactSectionName section,
        const std::string& lang);

    EXPORT virtual bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const = 0;
    EXPORT virtual bool Delete(const int type, const std::string& claimID)
        const = 0;
    EXPORT virtual ItemTypeList Items(const std::string& lang) const = 0;
    EXPORT virtual std::string Name(const std::string& lang) const = 0;
    EXPORT virtual std::shared_ptr<const ProfileSubsection> First() const = 0;
    EXPORT virtual std::shared_ptr<const ProfileSubsection> Next() const = 0;
    EXPORT virtual bool SetActive(
        const int type,
        const std::string& claimID,
        const bool active) const = 0;
    EXPORT virtual bool SetPrimary(
        const int type,
        const std::string& claimID,
        const bool primary) const = 0;
    EXPORT virtual bool SetValue(
        const int type,
        const std::string& claimID,
        const std::string& value) const = 0;
    EXPORT virtual proto::ContactSectionName Type() const = 0;

    virtual void Update(const opentxs::ContactSection& section) = 0;

    EXPORT virtual ~ProfileSection() = default;

protected:
    ProfileSection() = default;

private:
    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    ProfileSection& operator=(const ProfileSection&) = delete;
    ProfileSection& operator=(ProfileSection&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_PROFILE_SECTION_HPP
