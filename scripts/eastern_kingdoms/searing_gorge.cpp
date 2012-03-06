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
SDName: Searing_Gorge
SD%Complete: 80
SDComment: Quest support: 3377, 3441 (More accurate info on Kalaran needed). Lothos Riftwaker teleport to Molten Core.
SDCategory: Searing Gorge
EndScriptData */

/* ContentData
npc_dying_archaeologist
npc_kalaran_windblade
npc_lothos_riftwaker
npc_zamael_lunthistle
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_kalaran_windblade
######*/

bool GossipHello_npc_kalaran_windblade(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(3441) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tell me what drives this vengance?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_kalaran_windblade(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Continue please", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(1954, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Let me confer with my colleagues", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(1955, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(3441);
            break;
    }
    return true;
}

/*######
## npc_lothos_riftwaker
######*/

bool GossipHello_npc_lothos_riftwaker(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestRewardStatus(7487) || pPlayer->GetQuestRewardStatus(7848))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Teleport me to the Molten Core", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_lothos_riftwaker(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        if(pPlayer->isDead())
        {
		    pPlayer->ResurrectPlayer(0.5f);
            Corpse* corpse = pPlayer->GetCorpse();
            corpse->RemoveFromWorld();
        }
        pPlayer->TeleportTo(409, 1096.00f, -467.00f, -104.6f, 3.64f);	
    }

    return true;
}

/*######
## npc_zamael_lunthistle
######*/

bool GossipHello_npc_zamael_lunthistle(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(3377) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tell me your story", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(1920, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_zamael_lunthistle(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Please continue...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(1921, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Goodbye", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(1922, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(3377);
            break;
    }
    return true;
}

/*######
## npc_dorius_stonetender
######*/

enum eDoriousStonetender
{
    QUEST_SUNTARA_STONES    = 3367,
    GO_SINGED_LETTER        = 175704,
    NPC_HITMAN              = 6523,     // Dark Iron Rifleman, probably wrong, but he have a rifle :)
    SPELL_SHOT              = 7105,
    NPC_DARK_IRON_GEOLOGIST = 5839,
    NPC_DARK_IRON_WATCHMAN  = 8637,
};

struct MANGOS_DLL_DECL npc_dorius_stonetenderAI : public npc_escortAI
{
    npc_dorius_stonetenderAI(Creature* pCreature) : npc_escortAI(pCreature) {Reset();}

    ObjectGuid m_uiHitmanGUID;
    uint8  m_uiAmbushersCount;

    uint8  m_uiEventPhase;
    uint32 m_uiEventTimer;

    bool m_bCanWalk;

    void Reset()
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING) || HasEscortState(STATE_ESCORT_PAUSED))
            return;

        m_uiAmbushersCount = 0;
		m_uiHitmanGUID.Clear();
        m_uiEventPhase = 0;
        m_uiEventTimer = 0;
        m_bCanWalk = true;
    }

    void JustStartedEscort()
    {
        // !!! REMOVE IT WHEN SCRIPT BE DONE !!!
        /*SetRun(true);
        m_creature->SetSpeedRate(MOVE_RUN, 4.0f);*/

        m_creature->setFaction(FACTION_ESCORT_A_NEUTRAL_PASSIVE);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            // TODO: "..say something about the cauldron.."
            case 17:
                m_bCanWalk = false;
                m_uiEventPhase = 10;
                m_uiEventTimer = 2000;
                break;
            case 37:
                m_bCanWalk = false;
                m_uiEventPhase = 20;
                m_uiEventTimer = 2000;
                break;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        switch(pSummoned->GetEntry())
        {
            case NPC_DARK_IRON_GEOLOGIST:
            case NPC_DARK_IRON_WATCHMAN:
                ++m_uiAmbushersCount;
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    // 70% chance, that the Escorter will be attacked
                    pSummoned->AI()->AttackStart(rand()%100 <= 70 ? pPlayer : (Unit*)m_creature);
                }
                else
                    pSummoned->AI()->AttackStart(m_creature);
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        switch(pSummoned->GetEntry())
        {
            case NPC_DARK_IRON_GEOLOGIST:
            case NPC_DARK_IRON_WATCHMAN:
                --m_uiAmbushersCount;
                break;
        }
    }

    /*void SpellHit(Unit* pUnit, const SpellEntry* pSpell)
    {
        if (pSpell->Id == SPELL_SHOT)
        {
            // Do nothing
        }
    }*/

    void UpdateAI(const uint32 uiDiff)
    {
        if (Player* pPlayer = GetPlayerForEscort())
        {
            if (m_uiEventTimer)
            {
                if (m_uiEventTimer <= uiDiff)
                {
                    m_uiEventTimer = 0;

                    switch(m_uiEventPhase)
                    {
                        // Ambush
                        case 10:
                            m_creature->MonsterSay("Wait, $N. I need to breathe for a while.", LANG_UNIVERSAL, pPlayer);
                            m_creature->SetStandState(UNIT_STAND_STATE_SIT);
                            m_uiEventTimer = 6000;
                            break;
                        case 11:
                            m_creature->MonsterSay("Okay, let's go..", LANG_UNIVERSAL, pPlayer);
                            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                            m_uiEventTimer = 1500;
                            break;
                        case 12:
                            m_creature->SummonCreature(NPC_DARK_IRON_GEOLOGIST, -6774.96f, -1737.75f, 259.70f, 2.02f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            m_creature->SummonCreature(NPC_DARK_IRON_GEOLOGIST, -6770.99f, -1730.24f, 261.21f, 2.30f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            m_creature->SummonCreature(NPC_DARK_IRON_WATCHMAN,  -6815.39f, -1672.17f, 250.52f, 2.02f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            m_creature->SummonCreature(NPC_DARK_IRON_WATCHMAN,  -6822.61f, -1676.98f, 251.54f, 2.02f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                            m_uiEventTimer = 5000;
                            break;
                        case 13:
                            if (m_uiAmbushersCount > 0)
                                --m_uiEventPhase;
                            m_uiEventTimer = 2500;
                            break;
                        case 14:
                            m_bCanWalk = true;
                            break;

                        // Hitman's shot
                        case 20:
                            if (Creature* pHitman = m_creature->SummonCreature(NPC_HITMAN, -6368.77f, -1974.88f, 256.72f, 3.56f, TEMPSUMMON_TIMED_DESPAWN, 7000))
                            {
                                m_uiHitmanGUID = pHitman->GetObjectGuid();
                                pHitman->setFaction(35);
                                pHitman->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                pHitman->SetSheath(SHEATH_STATE_RANGED);
                            }
                            m_uiEventTimer = 3000;
                            break;
                        case 21:
                            if (Creature* pHitman = m_creature->GetMap()->GetCreature(m_uiHitmanGUID))
                                pHitman->CastSpell(m_creature, SPELL_SHOT, false);
                            m_uiEventTimer = 500;
                            break;
                        case 22:
                            // Fake death
                            m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
                            // Spawn "Singed Letter" gameobject
                            if (GameObject* GoSingedLetter = GetClosestGameObjectWithEntry(m_creature, GO_SINGED_LETTER, 15.0f))
                            {
                                // Always visible, currently useless script
                            }
                            m_uiEventTimer = 2000;
                            break;
                        case 23:
                            pPlayer->GroupEventHappens(QUEST_SUNTARA_STONES, m_creature);
                            if (Creature* pHitman = m_creature->GetMap()->GetCreature(m_uiHitmanGUID))
                                pHitman->ForcedDespawn();
                            m_uiEventTimer = 10000;
                            break;
                        case 24:
                            // Complete escort
                            m_uiEventPhase = 0;
                            m_bCanWalk = true;
                            break;
                    }
                    ++m_uiEventPhase;
                }
                else
                    m_uiEventTimer -= uiDiff;
            }
        }

        if (m_bCanWalk)
            npc_escortAI::UpdateAI(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_dorius_stonetender(Creature* pCreature)
{
    return new npc_dorius_stonetenderAI(pCreature);
}

bool QuestAccept_npc_dorius_stonetender(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_SUNTARA_STONES)
    {
        if (npc_dorius_stonetenderAI* pEscortAI = dynamic_cast<npc_dorius_stonetenderAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

/*######
## mob_lathoric_the_black
######*/

enum eLathoricTheBlack
{
    SAY_DORIUS_1            = -1000668,
    SAY_DORIUS_2            = -1000669,
    SAY_DORIUS_3            = -1000670,
    SAY_DORIUS_4            = -1000671,
    SAY_DORIUS_5            = -1000672,
    SAY_DORIUS_6            = -1000673,
    SAY_DORIUS_EMOTE        = -1000674,
    SAY_LATHORIC_1          = -1000675,
    SAY_LATHORIC_2          = -1000676,

    NPC_DORIUS              = 8421,
    NPC_OBSIDION            = 8400,
    NPC_LATHORIC_THE_BLACK  = 8391,
};

struct MANGOS_DLL_DECL mob_lathoric_the_blackAI : public ScriptedAI
{
    mob_lathoric_the_blackAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiEventTimer;
    uint8  m_uiEventPhase;

    ObjectGuid m_uiPlayerGUID;
    ObjectGuid m_uiObsidionGUID;

    void Reset() {}

    Creature* GetObsidion()
    {
        Creature* pObsidion = NULL;
        pObsidion = m_creature->GetMap()->GetCreature(m_uiObsidionGUID);

        if (!pObsidion)
            pObsidion = GetClosestCreatureWithEntry(m_creature, NPC_OBSIDION, 20.0f);

        return pObsidion;
    }

    void StartObsidionEvent(ObjectGuid player_guid)
    {
        m_uiPlayerGUID = player_guid;
        m_creature->UpdateEntry(NPC_DORIUS);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
        m_uiEventPhase = 1;
        m_uiEventTimer = 2000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                m_uiEventTimer = 0;

                switch(m_uiEventPhase)
                {
                    case 1:
                        DoScriptText(SAY_DORIUS_1, m_creature);
                        m_uiEventTimer = 5000;
                        break;
                    case 2:
                        DoScriptText(SAY_DORIUS_2, m_creature);
                        m_uiEventTimer = 8000;
                        break;
                    case 3:
                        DoScriptText(SAY_DORIUS_3, m_creature);
                        m_uiEventTimer = 9000;
                        break;
                    case 4:
                        DoScriptText(SAY_DORIUS_4, m_creature);
                        m_uiEventTimer = 11000;
                        break;
                    case 5:
                        DoScriptText(SAY_DORIUS_5, m_creature);
                        m_uiEventTimer = 11000;
                        break;
                    case 6:
                        DoScriptText(SAY_DORIUS_6, m_creature);
                        m_uiEventTimer = 7000;
                        break;
                    case 7:
                        DoScriptText(SAY_DORIUS_EMOTE, m_creature);
                        m_uiEventTimer = 2500;
                        break;
                    case 8:
                        m_creature->UpdateEntry(NPC_LATHORIC_THE_BLACK);
                        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
                        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                        DoScriptText(SAY_LATHORIC_1, m_creature);
                        m_uiEventTimer = 4000;
                        break;
                    case 9:
                        if (Creature* pObsidion = GetObsidion())
                            m_creature->SetFacingToObject(pObsidion);
                        m_uiEventTimer = 2000;
                        break;
                    case 10:
                        DoScriptText(SAY_LATHORIC_2, m_creature);
                        m_uiEventTimer = 3000;
                        break;
                    case 11:
                        if (Creature* pObsidion = GetObsidion())
                        {
                            pObsidion->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
                            pObsidion->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                            pObsidion->SetStandState(UNIT_STAND_STATE_STAND);
                        }
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                        m_uiEventTimer = 3000;
                        break;
                    case 12:
                        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_uiPlayerGUID))
                        {
                            if (m_creature->IsWithinDist(pPlayer, 40.0f))
                            {
                                if (Creature* pObsidion = GetObsidion())
                                    if (!pObsidion->SelectHostileTarget() || !pObsidion->getVictim())
                                        pObsidion->AI()->AttackStart(pPlayer);
                                if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                                    AttackStart(pPlayer);
                            }
                        }
                        m_creature->SetFacingTo(2.56f);
                        m_uiEventPhase = 0;
                        return;
                }
                ++m_uiEventPhase;
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_lathoric_the_black(Creature* pCreature)
{
    return new mob_lathoric_the_blackAI(pCreature);
}

/*######
## mob_obsidion
######*/

struct MANGOS_DLL_DECL mob_obsidionAI : public ScriptedAI
{
    mob_obsidionAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        // Aggro if can
        if (!m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE) && !m_creature->getVictim())
            ScriptedAI::MoveInLineOfSight(pWho);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_obsidion(Creature* pCreature)
{
    return new mob_obsidionAI(pCreature);
}

/*######
## npc_dying_archaeologist
######*/

#define QUEST_RISE_OBSIDION 3566

bool QuestAccept_npc_dying_archaeologist(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RISE_OBSIDION)
    {
        if (!GetClosestCreatureWithEntry(pCreature, NPC_LATHORIC_THE_BLACK, DEFAULT_VISIBILITY_DISTANCE))
        {
            if (Creature* pDorius = pCreature->SummonCreature(NPC_LATHORIC_THE_BLACK, -6465.37f, -1256.21f, 180.48f, 2.56f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000))
            {
                if (mob_lathoric_the_blackAI* pLathoricAI = dynamic_cast<mob_lathoric_the_blackAI*>(pDorius->AI()))
                    pLathoricAI->StartObsidionEvent(pPlayer->GetObjectGuid());
            }
        }
        else
            error_log("SD0: Searing Gorge - Player (guid %s) tried to start quest \"Rise, Obsidion!\", but another player is still on this quest. Quest taken, but no effect..", pPlayer->GetObjectGuid());
    }
    return true;
}

/*######
##
######*/

void AddSC_searing_gorge()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_kalaran_windblade";
    pNewScript->pGossipHello =  &GossipHello_npc_kalaran_windblade;
    pNewScript->pGossipSelect = &GossipSelect_npc_kalaran_windblade;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_lothos_riftwaker";
    pNewScript->pGossipHello =  &GossipHello_npc_lothos_riftwaker;
    pNewScript->pGossipSelect = &GossipSelect_npc_lothos_riftwaker;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_zamael_lunthistle";
    pNewScript->pGossipHello =  &GossipHello_npc_zamael_lunthistle;
    pNewScript->pGossipSelect = &GossipSelect_npc_zamael_lunthistle;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_dorius_stonetender";
    pNewScript->GetAI = &GetAI_npc_dorius_stonetender;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_dorius_stonetender;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_lathoric_the_black";
    pNewScript->GetAI = &GetAI_mob_lathoric_the_black;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_obsidion";
    pNewScript->GetAI = &GetAI_mob_obsidion;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_dying_archaeologist";
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_dying_archaeologist;
    pNewScript->RegisterSelf();
}
