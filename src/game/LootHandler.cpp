/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "WorldPacket.h"
#include "Log.h"
#include "Corpse.h"
#include "GameObject.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "WorldSession.h"
#include "LootMgr.h"
#include "Object.h"
#include "Group.h"
#include "World.h"
#include "Util.h"
#include "DBCStores.h"
#include "ObjectMgr.h"

void WorldSession::HandleAutostoreLootItemOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: CMSG_AUTOSTORE_LOOT_ITEM");

    uint8 lootSlot;
    Item* pItem = NULL;

    recv_data >> lootSlot;

    Loot* loot = sLootMgr.GetLoot(_player);

    if (!loot)
    {
        sLog.outError("HandleAutostoreLootItemOpcode> Cannot retrieve loot for player %s", _player->GetObjectGuid().GetString().c_str());
        return;
    }

    ObjectGuid const& lguid = loot->GetLootGuid();

    QuestItem* qitem = NULL;
    QuestItem* ffaitem = NULL;
    QuestItem* conditem = NULL;

    LootItem* item = loot->LootItemInSlot(lootSlot, _player, &qitem, &ffaitem, &conditem);

    if (!item)
    {
        _player->SendEquipError(EQUIP_ERR_ALREADY_LOOTED, NULL, NULL);
        return;
    }

    // questitems use the blocked field for other purposes
    if (!qitem && item->is_blocked)
    {
        loot->SendReleaseFor(_player);
        return;
    }

    ItemPosCountVec dest;
    InventoryResult msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item->itemid, item->count);
    if (msg == EQUIP_ERR_OK)
    {
        loot->SendItem(_player, lootSlot);
        //Item* newitem = _player->StoreNewItem(dest, item->itemid, true, item->randomPropertyId);

        if (qitem)
        {
            qitem->is_looted = true;
            // freeforall is 1 if everyone's supposed to get the quest item.
            if (item->freeforall || loot->GetPlayerQuestItems().size() == 1)
                loot->NotifyItemRemoved(lootSlot);
            else
                loot->NotifyQuestItemRemoved(qitem->index);
        }
        else
        {
            if (ffaitem)
            {
                // freeforall case, notify only one player of the removal
                ffaitem->is_looted = true;
                loot->NotifyItemRemoved(lootSlot);
            }
            else
            {
                // not freeforall, notify everyone
                if (conditem)
                    conditem->is_looted = true;
                loot->NotifyItemRemoved(lootSlot);
            }
        }

/*
        // if only one person is supposed to loot the item, then set it to looted
        if (!item->freeforall)
        {
            item->is_looted = true;
        }
*/

    }
    else
        _player->SendEquipError(msg, NULL, NULL, item->itemid);

    if (lguid.IsItem())
    {
        if (Item* item = _player->GetItemByGuid(lguid))
            item->SetLootState(ITEM_LOOT_CHANGED);
    }
}

void WorldSession::HandleLootMoneyOpcode(WorldPacket& /*recv_data*/)
{
    DEBUG_LOG("WORLD: CMSG_LOOT_MONEY");

    Loot* pLoot = sLootMgr.GetLoot(_player);

    if (!pLoot)
    {
        sLog.outError("HandleLootMoneyOpcode> Cannot retrieve loot for player %s", _player->GetObjectGuid().GetString().c_str());
        return;
    }

    ObjectGuid const& lguid = pLoot->GetLootGuid();

    if (pLoot)
    {
        pLoot->NotifyMoneyRemoved();

        if (!lguid.IsItem() && pLoot->lootMethod != NOT_GROUP_TYPE_LOOT)           // item can be looted only single player
        {
            uint32 money_per_player = uint32((pLoot->gold) / (pLoot->ownerSet.size()));

            for (GuidSet::const_iterator itr = pLoot->ownerSet.begin(); itr != pLoot->ownerSet.end(); ++itr)
            {
                Player* plr = sObjectMgr.GetPlayer(*itr);
                if (!plr || !plr->GetSession())
                    continue;

                plr->ModifyMoney(money_per_player);
                plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, money_per_player);

                WorldPacket data(SMSG_LOOT_MONEY_NOTIFY, 4 + 1);
                data << uint32(money_per_player);
                data << uint8(0);// 0 is "you share of loot..."

                plr->GetSession()->SendPacket(&data);
            }
        }
        else
        {
            _player->ModifyMoney(pLoot->gold);
            _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, pLoot->gold);

            WorldPacket data(SMSG_LOOT_MONEY_NOTIFY, 4 + 1);
            data << uint32(pLoot->gold);
            data << uint8(1);                               // 1 is "you loot..."
            _player->GetSession()->SendPacket(&data);

            if (lguid.IsItem())
            {
                if (Item* item = _player->GetItemByGuid(lguid))
                    item->SetLootState(ITEM_LOOT_CHANGED);
            }
        }
        pLoot->gold = 0;

        if (pLoot->IsLootedFor(_player->GetObjectGuid()))
        {
            pLoot->SendReleaseFor(_player->GetObjectGuid());
        }
        else
        {
            pLoot->ForceLootAnimationCLientUpdate();
        }
    }
}

void WorldSession::HandleLootOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: CMSG_LOOT");

    ObjectGuid lguid;
    recv_data >> lguid;

    // Check possible cheat
    if (!_player->isAlive())
        return;

    if (Loot* loot = sLootMgr.GetLoot(_player, lguid))
        loot->ShowContentTo(_player);
}

void WorldSession::HandleLootReleaseOpcode(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: CMSG_LOOT_RELEASE");

    ObjectGuid lguid;
    recv_data >> lguid;

    if (Loot* loot = sLootMgr.GetLoot(_player, lguid))
        loot->Release(_player);

}

void WorldSession::HandleLootMasterGiveOpcode(WorldPacket& recv_data)
{
    uint8 slotid;
    ObjectGuid lootguid;
    ObjectGuid target_playerguid;

    recv_data >> lootguid >> slotid >> target_playerguid;

    Player* target = ObjectAccessor::FindPlayer(target_playerguid);
    if (!target)
        return;

    DEBUG_LOG("WorldSession::HandleLootMasterGiveOpcode> Giver = %s, Target = %s.", _player->GetObjectGuid().GetString().c_str(), target_playerguid.GetString().c_str());

    Loot* pLoot = sLootMgr.GetLoot(_player, lootguid);

    if (!pLoot || _player->GetObjectGuid() != pLoot->masterOwnerGuid)
        return;

    if (slotid > pLoot->items.size())
    {
        DEBUG_LOG("WorldSession::HandleLootMasterGiveOpcode> Player %s might be using a hack! (slot %d, size " SIZEFMTD ")", _player->GetName(), slotid, pLoot->items.size());
        return;
    }

    InventoryResult msg = pLoot->SendItem(target_playerguid, slotid);

    // Don't have to use LootItemInSlot because i doubt quest item may be distributable by the master of the loot
    LootItem& item = pLoot->items[slotid];

    if (msg != EQUIP_ERR_OK)
    {
        // send duplicate of error massage to master looter
        _player->SendEquipError(msg, NULL, NULL, item.itemid);
        return;
    }

    // mark as looted
    item.count = 0;
    item.is_looted = true;

    pLoot->NotifyItemRemoved(slotid);
    --pLoot->unlootedCount;
}

void WorldSession::HandleLootMethodOpcode(WorldPacket& recv_data)
{
    uint32 lootMethod;
    ObjectGuid lootMaster;
    uint32 lootThreshold;
    recv_data >> lootMethod >> lootMaster >> lootThreshold;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()))
        return;
    /********************/

    // everything is fine, do it
    group->SetLootMethod((LootMethod)lootMethod);
    group->SetLooterGuid(lootMaster);
    group->SetLootThreshold((ItemQualities)lootThreshold);
    group->SendUpdate();
}

void WorldSession::HandleLootRoll(WorldPacket& recv_data)
{
    ObjectGuid lootedTarget;
    uint32 itemSlot;
    uint8  rollType;
    recv_data >> lootedTarget;                              // guid of the item rolled
    recv_data >> itemSlot;
    recv_data >> rollType;

    sLog.outDebug("WORLD RECIEVE CMSG_LOOT_ROLL, From:%s, rollType:%u", lootedTarget.GetString().c_str(), uint32(rollType));

    Group* group = _player->GetGroup();
    if (!group)
        return;

    if (rollType >= ROLL_NOT_EMITED_YET)
        return;

    sLootMgr.PlayerVote(GetPlayer(), lootedTarget, itemSlot, RollVote(rollType));
}

