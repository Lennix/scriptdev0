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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 5211, 5742. Argent Medic, Special vendor Augustus the Touched, Wind Master William Kielar
SDCategory: Eastern Plaguelands
EndScriptData */

/* ContentData
mobs_ghoul_flayer
npc_argent_medic
npc_augustus_the_touched
npc_betina_bigglezink
npc_darrowshire_spirit
npc_tirion_fordring
npc_highlord_taelan_fordring
npc_william_kielar
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

//id8530 - cannibal ghoul
//id8531 - gibbering ghoul
//id8532 - diseased flayer

struct MANGOS_DLL_DECL mobs_ghoul_flayerAI : public ScriptedAI
{
    mobs_ghoul_flayerAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset() { }

    void JustDied(Unit* pKiller)
    {
        if (pKiller->GetTypeId() == TYPEID_PLAYER)
            m_creature->SummonCreature(11064, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 40000);
    }

};

CreatureAI* GetAI_mobs_ghoul_flayer(Creature* pCreature)
{
    return new mobs_ghoul_flayerAI(pCreature);
}

/*######
## npc_argent_medic
######*/

enum eArgentMedic
{
    FACTION_ARGENT_DAWN     = 529,

    GOSSIP_BASE             = 8454,
    GOSSIP_HONORED          = 8455,

    SPELL_GHOUL_ROT         = 12541,
    SPELL_MAGGOT_SLIME      = 16449
};

bool GossipHello_npc_argent_medic(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetReputationRank(FACTION_ARGENT_DAWN) >= REP_HONORED)
    {
        if (pPlayer->HasAura(SPELL_GHOUL_ROT) || pPlayer->HasAura(SPELL_MAGGOT_SLIME))
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I'm infected, help me please!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_HONORED, pCreature->GetObjectGuid());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_BASE, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_argent_medic(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (pPlayer->HasAura(SPELL_GHOUL_ROT))
            pPlayer->RemoveAurasDueToSpell(SPELL_GHOUL_ROT);

        if (pPlayer->HasAura(SPELL_MAGGOT_SLIME))
            pPlayer->RemoveAurasDueToSpell(SPELL_MAGGOT_SLIME);
    }
    return true;
}

/*######
## npc_augustus_the_touched
######*/

bool GossipHello_npc_augustus_the_touched(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pCreature->isVendor() && pPlayer->GetQuestRewardStatus(6164))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_augustus_the_touched(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());
    return true;
}

/*######
## npc_betina_bigglezink
######*/

#define GOSSIP_GAMBIT     "Could you give me one more Dawn's Gambit?"

enum eBetina
{
    ITEM_DAWNS_GAMBIT                 = 12368,
    SPELL_CREATE_ITEM_DAWNS_GAMBIT    = 18367,
    QUEST_DAWNS_GAMBIT                = 4771
};

bool GossipHello_npc_betina_bigglezink(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (!pPlayer->HasItemCount(ITEM_DAWNS_GAMBIT, 1, true) && pPlayer->GetQuestRewardStatus(QUEST_DAWNS_GAMBIT))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_GAMBIT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_betina_bigglezink(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->CastSpell(pPlayer, SPELL_CREATE_ITEM_DAWNS_GAMBIT, false);
    }
    return true;
}

/*######
## npc_darrowshire_spirit
######*/

enum eSpirit
{
    SPELL_SPIRIT_SPAWNIN      = 17321,
    GOSSIP_SPIRIT             = 3873
};

struct MANGOS_DLL_DECL npc_darrowshire_spiritAI : public ScriptedAI
{
    npc_darrowshire_spiritAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset()
    {
        DoCastSpellIfCan(m_creature, SPELL_SPIRIT_SPAWNIN);
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }
};

CreatureAI* GetAI_npc_darrowshire_spirit(Creature* pCreature)
{
    return new npc_darrowshire_spiritAI(pCreature);
}

bool GossipHello_npc_darrowshire_spirit(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Farewell, my friend.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->SEND_GOSSIP_MENU(GOSSIP_SPIRIT, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_darrowshire_spirit(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->TalkedToCreature(pCreature->GetEntry(), pCreature->GetObjectGuid());
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }
    return true;
}

/*######
## npc_tirion_fordring
######*/

enum eTirionFordring
{
    SPELL_DEVOTION_AURA     = 8285,
    SPELL_EXORCISM          = 17149,
    SPELL_HAMMER_OF_JUSTICE = 13005,

    NPC_TAELAN              = 1842
};

struct MANGOS_DLL_DECL npc_tirion_fordringAI : public ScriptedAI
{
    npc_tirion_fordringAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiExorcismTimer;
    uint32 m_uiHammerOfJusticeTimer;

    void Reset()
    {
        m_uiExorcismTimer = 7000;
        m_uiHammerOfJusticeTimer = 4000;
    }

    void Aggro(Unit* /*pWho*/)
    {
        DoCastSpellIfCan(m_creature, SPELL_DEVOTION_AURA);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (GetClosestCreatureWithEntry(m_creature, NPC_TAELAN, 40.0f))
            if (pDoneBy->GetTypeId() == TYPEID_UNIT)
                uiDamage = (uiDamage >= m_creature->GetHealth() ? 0 : urand(8,12));
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Exorcism
        if (m_uiHammerOfJusticeTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_HAMMER_OF_JUSTICE);
            m_uiHammerOfJusticeTimer = 10000;
        }
        else
            m_uiHammerOfJusticeTimer -= uiDiff;

        // Hammer of Justice
        if (m_uiHammerOfJusticeTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_HAMMER_OF_JUSTICE);
            m_uiHammerOfJusticeTimer = 18000;
        }
        else
            m_uiHammerOfJusticeTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_tirion_fordring(Creature* pCreature)
{
    return new npc_tirion_fordringAI(pCreature);
}

bool GossipHello_npc_tirion_fordring(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(5742) == QUEST_STATUS_INCOMPLETE && pPlayer->getStandState() == UNIT_STAND_STATE_SIT)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I am ready to hear your tale, Tirion.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_tirion_fordring(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Thank you, Tirion.  What of your identity?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(4493, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "That is terrible.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(4494, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I will, Tirion.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            pPlayer->SEND_GOSSIP_MENU(4495, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(5742);
            break;
    }
    return true;
}

/*######
## npc_william_kielar
######*/

bool GossipHello_npc_william_kielar(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Take me to Northpass Tower!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Take me to Eastwall Tower!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Take me to Crown Guard Tower!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_william_kielar(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->ActivateTaxiPathTo(494);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->ActivateTaxiPathTo(495);
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->ActivateTaxiPathTo(496);
            break;
    }    
    return true;
}

void AddSC_eastern_plaguelands()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "mobs_ghoul_flayer";
    pNewscript->GetAI = &GetAI_mobs_ghoul_flayer;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_argent_medic";
    pNewscript->pGossipHello = &GossipHello_npc_argent_medic;
    pNewscript->pGossipSelect = &GossipSelect_npc_argent_medic;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_augustus_the_touched";
    pNewscript->pGossipHello = &GossipHello_npc_augustus_the_touched;
    pNewscript->pGossipSelect = &GossipSelect_npc_augustus_the_touched;
    pNewscript->RegisterSelf();
    
    pNewscript = new Script;
    pNewscript->Name = "npc_betina_bigglezink";
    pNewscript->pGossipHello = &GossipHello_npc_betina_bigglezink;
    pNewscript->pGossipSelect = &GossipSelect_npc_betina_bigglezink;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_darrowshire_spirit";
    pNewscript->pGossipHello = &GossipHello_npc_darrowshire_spirit;
    pNewscript->pGossipSelect = &GossipSelect_npc_darrowshire_spirit;
    pNewscript->GetAI = &GetAI_npc_darrowshire_spirit;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_tirion_fordring";
    pNewscript->GetAI = &GetAI_npc_tirion_fordring;
    pNewscript->pGossipHello = &GossipHello_npc_tirion_fordring;
    pNewscript->pGossipSelect = &GossipSelect_npc_tirion_fordring;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_william_kielar";
    pNewscript->pGossipHello = &GossipHello_npc_william_kielar;
    pNewscript->pGossipSelect = &GossipSelect_npc_william_kielar;
    pNewscript->RegisterSelf();
}
