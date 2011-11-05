/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Item_Scripts
SD%Complete: 100
SDComment: Items for a range of different items. See content below (in script)
SDCategory: Items
EndScriptData */

/* ContentData
item_essence_of_hakkar
EndContentData */

#include "precompiled.h"
#include "Spell.h"

/*######
## item_essence_of_hakkar       temp script to create filled egg
######*/

enum eEssenceOfHakkar
{
    ITEM_EGG_OF_HAKKAR            = 10465,
    ITEM_FILLED_EGG_OF_HAKKAR     = 10662,
    ITEM_ESSENCE_OF_HAKKAR        = 10663,
    QUEST_THE_GOD_HAKKAR          = 3528
};

bool ItemUse_item_essence_of_hakkar(Player* pPlayer, Item* pItem, SpellCastTargets const& targets)
{
    if (pPlayer->GetQuestStatus(QUEST_THE_GOD_HAKKAR) == QUEST_STATUS_INCOMPLETE)
        if (pPlayer->HasItemCount(ITEM_EGG_OF_HAKKAR, 1) && pPlayer->HasItemCount(ITEM_ESSENCE_OF_HAKKAR, 1))
        {
            pPlayer->DestroyItemCount(ITEM_ESSENCE_OF_HAKKAR, 1, true);
            pPlayer->DestroyItemCount(ITEM_EGG_OF_HAKKAR, 1, true);
            if (Item* pFilledEgg = pPlayer->StoreNewItemInInventorySlot(ITEM_FILLED_EGG_OF_HAKKAR, 1))
                pPlayer->SendNewItem(pFilledEgg, 1, true, false);
            return true;
        }
    return false;
}

/*######
## AddSC
######*/

void AddSC_item_scripts()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name="item_essence_of_hakkar";
    pNewscript->pItemUse = &ItemUse_item_essence_of_hakkar;
    pNewscript->RegisterSelf();
}
