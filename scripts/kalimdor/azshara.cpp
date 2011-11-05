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
SDName: Azshara
SD%Complete: 90
SDComment: Quest support: 2744, 3141, 9364, 3382
SDCategory: Azshara
EndScriptData */

/* ContentData
mobs_spitelashes
npc_duke_hydraxis
npc_loramus_thalipedes
npc_captain_vanessa_beltis
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"

/*######
## mobs_spitelashes
######*/

struct MANGOS_DLL_DECL mobs_spitelashesAI : public ScriptedAI
{
    mobs_spitelashesAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 morphtimer;
    bool spellhit;

    void Reset()
    {
        morphtimer = 0;
        spellhit = false;
    }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)
    {
        if (!spellhit &&
            Hitter->GetTypeId() == TYPEID_PLAYER &&
            ((Player*)Hitter)->GetQuestStatus(9364) == QUEST_STATUS_INCOMPLETE &&
            (Spellkind->Id==118 || Spellkind->Id== 12824 || Spellkind->Id== 12825 || Spellkind->Id== 12826))
        {
            spellhit=true;
            DoCastSpellIfCan(m_creature,29124);                       //become a sheep
        }
    }

    void UpdateAI(const uint32 diff)
    {
        // we mustn't remove the creature in the same round in which we cast the summon spell, otherwise there will be no summons
        if (spellhit && morphtimer>=5000)
        {
            m_creature->ForcedDespawn();
            return;
        }

        // walk 5 seconds before summoning
        if (spellhit && morphtimer<5000)
        {
            morphtimer+=diff;
            if (morphtimer>=5000)
            {
                DoCastSpellIfCan(m_creature,28406);                   //summon copies
                DoCastSpellIfCan(m_creature,6924);                    //visual explosion
            }
        }
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        //TODO: add abilities for the different creatures
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mobs_spitelashes(Creature* pCreature)
{
    return new mobs_spitelashesAI(pCreature);
}

/*######
## npc_duke_hydraxis
######*/

enum eDukeHydraxis
{
    FACTION_HYDRAXIAN_WATERLORDS        = 749,
    ITEM_AQUAL_QUINTESSENCE             = 17333,
    ITEM_ETERNAL_QUINTESSENCE           = 22754,
    QUEST_HANDS_OF_THE_ENEMY            = 6824,
    SPELL_CREATE_AQUAL_QUINTESSENCE     = 21357,
    SPELL_CREATE_ETERNAL_QUINTESSENCE   = 28439,
};

bool GossipHello_npc_duke_hydraxis(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestRewardStatus(QUEST_HANDS_OF_THE_ENEMY) && !pPlayer->HasItemCount(ITEM_AQUAL_QUINTESSENCE, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Can you give me Aqual Quintessence?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (pPlayer->GetReputationRank(FACTION_HYDRAXIAN_WATERLORDS) >= REP_REVERED && !pPlayer->HasItemCount(ITEM_ETERNAL_QUINTESSENCE, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Can you give me Eternal Quintessence?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_duke_hydraxis(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->CLOSE_GOSSIP_MENU();
            pCreature->CastSpell(pPlayer, SPELL_CREATE_AQUAL_QUINTESSENCE, true);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->CLOSE_GOSSIP_MENU();
            pCreature->CastSpell(pPlayer, SPELL_CREATE_ETERNAL_QUINTESSENCE, true);
            break;
    }
    return true;
}

/*######
## npc_loramus_thalipedes
######*/

bool GossipHello_npc_loramus_thalipedes(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(2744) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Can you help me?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    if (pPlayer->GetQuestStatus(3141) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tell me your story", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_loramus_thalipedes(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(2744);
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Please continue", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            pPlayer->SEND_GOSSIP_MENU(1813, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I do not understand", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            pPlayer->SEND_GOSSIP_MENU(1814, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Indeed", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
            pPlayer->SEND_GOSSIP_MENU(1815, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+23:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I will do this with or your help, Loramus", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
            pPlayer->SEND_GOSSIP_MENU(1816, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+24:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Yes", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
            pPlayer->SEND_GOSSIP_MENU(1817, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+25:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(3141);
            break;
    }
    return true;
}

/*######
## npc_captain_vanessa_beltis
######*/

enum eCaptainVanessaBeltis
{
    /*NPC_SPITELASH_WARRIOR       = 6190,
    NPC_SPITELASH_SCREAMER      = 6193,
    NPC_SPITELASH_SERPENT_GUARD = 6194,
    NPC_SPITELASH_SIREN         = 6195,
    NPC_SPITELASH_MYRMIDON      = 6196,
    NPC_SPITELASH_BATTLEMASTER  = 7885,
    NPC_SPITELASH_ENCHANTRESS   = 7886,*/
    NPC_SPITELASH_RAIDER        = 12204,    // currently used
    NPC_SPITELASH_WITCH         = 12205,    // currently used
    NPC_HORIZON_SCOUT_CREWMAN   = 8386,     // friendly
    NPC_PATRICK_MILLS           = 8382,     // friendly
    NPC_ALEXANDRA_BLAZEN        = 8378,     // friendly
    NPC_LINDROS                 = 8381,     // friendly

    QUEST_A_CREW_UNDER_FIRE     = 3382,

    MAX_WAVES                   = 6,    // default: 6
    MAX_MOBS_IN_WAVE            = 5,    // max 6 (or add more SpawnLocations into definition below)
};

static float SpawnLocations[6][3]=
{
    {2378.67f, -5902.19f, 10.74f},
    {2374.49f, -5901.83f, 11.22f},
    {2374.84f, -5897.79f, 10.66f},
    {2378.55f, -5898.11f, 10.38f},
    {2382.61f, -5898.46f, 11.06f},
    {2382.15f, -5903.72f, 10.77f},
};

struct MANGOS_DLL_DECL npc_captain_vanessa_beltisAI : public FollowerAI
{
    npc_captain_vanessa_beltisAI(Creature* pCreature) : FollowerAI(pCreature) {Reset();}

    uint32 m_uiSpawnNewWaveTimer;
    uint8  m_uiWavesSpawnedCount;
    uint8  m_uiWaveMobsKilledCount;

    void Reset()
    {
        if (HasFollowState(STATE_FOLLOW_INPROGRESS) || HasFollowState(STATE_FOLLOW_PAUSED))
            return;

        m_uiSpawnNewWaveTimer = 5000;
        m_uiWavesSpawnedCount = 0;
        m_uiWaveMobsKilledCount = 0;
    }

    void DoSpawnNewWave()
    {
        debug_log("SD0: Azshara: Escort/Follower quest \"A Crew Under Fire\" - DoSpawnNewWave()");

        ++m_uiWavesSpawnedCount;

        for(uint8 itr = 0; itr < MAX_MOBS_IN_WAVE; ++itr)
        {
            m_creature->SummonCreature(rand()%2 == 0 ? NPC_SPITELASH_RAIDER : NPC_SPITELASH_WITCH, SpawnLocations[itr][0], SpawnLocations[itr][1], SpawnLocations[itr][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        switch(pSummoned->GetEntry())
        {
            case NPC_SPITELASH_RAIDER:
            case NPC_SPITELASH_WITCH:
                if (HasFollowState(STATE_FOLLOW_INPROGRESS) || HasFollowState(STATE_FOLLOW_PAUSED) && GetLeaderForFollower())
                    pSummoned->AI()->AttackStart(GetLeaderForFollower());
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        switch(pSummoned->GetEntry())
        {
            case NPC_SPITELASH_RAIDER:
            case NPC_SPITELASH_WITCH:
                if (++m_uiWaveMobsKilledCount == (MAX_WAVES * MAX_MOBS_IN_WAVE))
                {
                    if (GetLeaderForFollower())
                        GetLeaderForFollower()->GroupEventHappens(QUEST_A_CREW_UNDER_FIRE, m_creature);
                    SetFollowComplete();

                    m_creature->SetHealth(0);
                    m_creature->SetDeathState(JUST_DIED);
                    m_creature->Respawn();
                }
                break;
        }
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (!GetLeaderForFollower())
            return;

        if (Group* pGroup = GetLeaderForFollower()->GetGroup())
        {
            for(GroupReference* pRef = pGroup->GetFirstMember(); pRef != NULL; pRef = pRef->next())
            {
                if (Player* pMember = pRef->getSource())
                {
                    pMember->FailQuest(QUEST_A_CREW_UNDER_FIRE);
                }
            }
        }
        else
        {
            GetLeaderForFollower()->FailQuest(QUEST_A_CREW_UNDER_FIRE);
        }
    }

    void UpdateFollowerAI(const uint32 uiDiff)
    {
        if (HasFollowState(STATE_FOLLOW_PAUSED))
        {
            if (m_uiWavesSpawnedCount < MAX_WAVES)
            {
                if (m_uiSpawnNewWaveTimer)
                {
                    if (m_uiSpawnNewWaveTimer <= uiDiff)
                    {
                        DoSpawnNewWave();
                        m_uiSpawnNewWaveTimer = 50000;
                    }
                    else
                        m_uiSpawnNewWaveTimer -= uiDiff;
                }
            }
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captain_vanessa_beltis(Creature* pCreature)
{
    return new npc_captain_vanessa_beltisAI(pCreature);
}

bool QuestAccept_npc_captain_vanessa_beltis(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_A_CREW_UNDER_FIRE)
    {
	    if (npc_captain_vanessa_beltisAI* pEscortAI = dynamic_cast<npc_captain_vanessa_beltisAI*>(pCreature->AI()))
        {
            // Respawn dead crew.
            std::list<Creature*> lBeltissScouts;
            GetCreatureListWithEntryInGrid(lBeltissScouts, pCreature, NPC_HORIZON_SCOUT_CREWMAN, 30.0f);
            if (!lBeltissScouts.empty())
                for(std::list<Creature*>::const_iterator itr = lBeltissScouts.begin(); itr != lBeltissScouts.end(); ++itr)
                    if ((*itr)->isDead())
                        (*itr)->Respawn();

            if (Creature* pPatrickMills = GetClosestCreatureWithEntry(pCreature, NPC_PATRICK_MILLS, 30.0f))
                if (pPatrickMills->isDead())
                    pPatrickMills->Respawn();

            if (Creature* pAlexandraBlazen = GetClosestCreatureWithEntry(pCreature, NPC_ALEXANDRA_BLAZEN, 30.0f))
                if (pAlexandraBlazen->isDead())
                    pAlexandraBlazen->Respawn();

            if (Creature* pLindros = GetClosestCreatureWithEntry(pCreature, NPC_LINDROS, 30.0f))
                if (pLindros->isDead())
                    pLindros->Respawn();

            // Start event.
            pEscortAI->StartFollow(pPlayer, 0, pQuest);
            pEscortAI->SetFollowPaused(true);
        }
    }
    return true;
}

void AddSC_azshara()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "mobs_spitelashes";
    pNewscript->GetAI = &GetAI_mobs_spitelashes;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_duke_hydraxis";
    pNewscript->pGossipHello = &GossipHello_npc_duke_hydraxis;
    pNewscript->pGossipSelect = &GossipSelect_npc_duke_hydraxis;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_loramus_thalipedes";
    pNewscript->pGossipHello = &GossipHello_npc_loramus_thalipedes;
    pNewscript->pGossipSelect = &GossipSelect_npc_loramus_thalipedes;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_captain_vanessa_beltis";
    pNewscript->GetAI = &GetAI_npc_captain_vanessa_beltis;
    pNewscript->pQuestAcceptNPC = &QuestAccept_npc_captain_vanessa_beltis;
    pNewscript->RegisterSelf();
}
