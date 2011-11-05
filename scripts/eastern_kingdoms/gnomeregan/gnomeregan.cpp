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
SDName: Gnomeregan
SD%Complete: 90
SDComment: Quest support: 2904, Kernobee escort, find correct Emi's spell for create an explosive barrel
SDCategory: Gnomeregan
EndScriptData */

/* ContentData
npc_kernobee
at_clockwerk_run
go_thermaplug_button
npc_blastmaster_emi_shortfuse
EndContentData */

#include "precompiled.h"
#include "gnomeregan.h"
#include "escort_ai.h"
#include "follower_ai.h"

/*######
## npc_kernobee
######*/

enum eKernobee
{
    QUEST_A_FINE_MESS   = 2904,
};

struct MANGOS_DLL_DECL npc_kernobeeAI : public FollowerAI
{
	npc_kernobeeAI(Creature* pCreature) : FollowerAI(pCreature) {Reset();}

    void Reset() {}

	void UpdateAI(const uint32 uiDiff)
	{
        FollowerAI::UpdateAI(uiDiff);

		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_kernobee(Creature* pCreature)
{
	return new npc_kernobeeAI(pCreature);
}

bool QuestAccept_npc_kernobee(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_A_FINE_MESS)
    {
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);

        if (npc_kernobeeAI* pFollowAI = dynamic_cast<npc_kernobeeAI*>(pCreature->AI()))
        {
            pFollowAI->StartFollow(pPlayer, FACTION_ESCORT_N_FRIEND_ACTIVE, pQuest);
        }
    }
    return true;
}

/*######
## at_clockwerk_run
######*/

bool AreaTrigger_at_clockwerk_run(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    ScriptedInstance* m_pInstance = (ScriptedInstance*)pPlayer->GetInstanceData();
    if (!m_pInstance)
        return false;

	if (pPlayer->GetQuestStatus(QUEST_A_FINE_MESS) == QUEST_STATUS_INCOMPLETE)
	{
        if (Creature* pKernobee = m_pInstance->instance->GetCreature(m_pInstance->GetData64(DATA_KERNOBEE)))
        {
            if (pKernobee->isAlive() && pPlayer->IsWithinDistInMap(pKernobee, 20.0f))
            {
                if (npc_kernobeeAI* pFollowAI = dynamic_cast<npc_kernobeeAI*>(pKernobee->AI()))
                {
                    pPlayer->GroupEventHappens(QUEST_A_FINE_MESS, pKernobee);
                    pFollowAI->SetFollowComplete();
                }
            }
        }
        else
            error_log("SD0: Gnomeregan: Player which have active quest \"A Fine Mess\" to step up at an areatrigger, which will to complete that quest, but NPC needed for that (Kernobee) is in the instance not spawned. Quest will never be completed!");
	}
	return false;
}

/*######
## npc_blastmaster_emi_shortfuse
######*/

enum eEmiShortfuse
{
    SPELL_EXPLOSION            = 12158,
    SPELL_EXPLOSION_           = 12159,
    SPELL_RED_STREAKS_FIREWORK = 11542,
    SPELL_EXPLOSIVE_BARREL     = 10772,         // 10772 = Weegli's explosive barrel from Zul'Farrak

    // Creatures & GameObjects
    NPC_CAVERNDEEP_AMBUSHER    = 6207,
    NPC_GRUBBIS                = 7361,
    NPC_CHOMPER                = 6215,
    GO_RED_ROCKET              = 103820,

    // says & yells
    SAY_AGGRO                  = -1090004,
    SAY_EMI_01                 = -1090005,
    SAY_EMI_02                 = -1090006,
    SAY_EMI_03                 = -1090007,
    SAY_EMI_04                 = -1090008,
    SAY_EMI_05                 = -1090009,
    SAY_EMI_06                 = -1090010,
    SAY_EMI_07                 = -1090011,
    SAY_EMI_08                 = -1090012,
    SAY_EMI_09                 = -1090013,
    SAY_EMI_10                 = -1090014,
    SAY_EMI_11                 = -1090015,      //yell
    SAY_EMI_12                 = -1090016,      //yell
    SAY_EMI_13                 = -1090017,      //yell
    SAY_EMI_14                 = -1090018,
    SAY_EMI_15                 = -1090019,
    SAY_EMI_16                 = -1090020,
    SAY_EMI_17                 = -1090021,
    SAY_EMI_18                 = -1090022,
    SAY_EMI_19                 = -1090023,      //yell
    SAY_EMI_20                 = -1090024,      //yell
    SAY_EMI_21                 = -1090025,      //yell
    SAY_EMI_22                 = -1090026
};

struct MANGOS_DLL_DECL npc_blastmaster_emi_shortfuseAI : npc_escortAI
{
    npc_blastmaster_emi_shortfuseAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_pInstance = (instance_gnomeregan*)pCreature->GetInstanceData();
        Reset();
    }

    instance_gnomeregan* m_pInstance;
    uint32 m_uiLastReachedWaypoint;
    uint32 m_uiInvadersSummoned;

    GUIDList lCaverndeepInvaders;

    uint32 m_uiEventTimer;
    uint8 m_uiEventPhase;

    bool bCanWalk;

    void Reset()
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        m_uiLastReachedWaypoint = 0;
        m_uiInvadersSummoned = 0;

        m_uiEventTimer = 0;
        m_uiEventPhase = 0;

        bCanWalk = true;
    }

    void Aggro(Unit* /*pAttacker*/)
    {
        if (m_uiLastReachedWaypoint > 6 && m_uiLastReachedWaypoint < 9)
            DoScriptText(SAY_AGGRO, m_creature);
    }

    void WaypointStart(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 0:
                DoScriptText(SAY_EMI_01, m_creature);
                break;
        }
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_EMI, NOT_STARTED);
    }

    void WaypointReached(uint32 uiPointId)
    {
        m_uiLastReachedWaypoint = uiPointId;

        switch(uiPointId)
        {
            case 1: DoScriptText(SAY_EMI_02, m_creature); break;
            case 2:
                DoScriptText(SAY_EMI_03, m_creature);
                SetRun(true);
                bCanWalk = false;
                m_uiEventPhase = 1;
                m_uiEventTimer = 3500;
                break;
            case 3:
                DoScriptText(SAY_EMI_08, m_creature);
                DoCastSpellIfCan(m_creature, SPELL_EXPLOSIVE_BARREL);
                //m_creature->HandleEmote(EMOTE_ONESHOT_LOOT);
                SetRun(false);
                bCanWalk = false;
                m_uiEventPhase = 10;
                m_uiEventTimer = 4000;
                break;
            case 4:
                DoScriptText(SAY_EMI_09, m_creature);
                DoCastSpellIfCan(m_creature, SPELL_EXPLOSIVE_BARREL);
                //m_creature->HandleEmote(EMOTE_ONESHOT_LOOT);
                SetRun(true);
                bCanWalk = false;
                m_uiEventPhase = 20;
                m_uiEventTimer = 4000;
                break;
            case 5:
                m_creature->SetFacingTo(3.80f);
                SetRun(false);
                bCanWalk = false;
                m_uiEventPhase = 30;
                m_uiEventTimer = 1500;
                break;
            case 6:
                if (GetPlayerForEscort())
                    m_creature->SetFacingToObject(GetPlayerForEscort());
                DoScriptText(SAY_EMI_14, m_creature);       // well done, without your help ...
                SetRun(true);
                bCanWalk = false;
                m_uiEventPhase = 40;
                m_uiEventTimer = 6000;
                break;
            case 7:
                DoCastSpellIfCan(m_creature, SPELL_EXPLOSIVE_BARREL);
                //m_creature->HandleEmote(EMOTE_ONESHOT_LOOT);
                SetRun(false);
                bCanWalk = false;
                m_uiEventPhase = 50;
                m_uiEventTimer = 4000;
                break;
            case 8:
                DoSummonCaverndeepIncursion(false, 6);
                DoCastSpellIfCan(m_creature, SPELL_EXPLOSIVE_BARREL);
                //m_creature->HandleEmote(EMOTE_ONESHOT_LOOT);
                SetRun(true);
                bCanWalk = false;
                m_uiEventPhase = 60;
                m_uiEventTimer = 4000;
                break;
            case 9:
                m_creature->SetFacingTo(0.56f);
                SetRun(false);
                bCanWalk = false;
                m_uiEventPhase = 70;
                m_uiEventTimer = 1500;
                break;
            case 10:
                if (GetPlayerForEscort())
                    m_creature->SetFacingToObject(GetPlayerForEscort());
                DoScriptText(SAY_EMI_22, m_creature);
                m_uiEventPhase = 80;
                m_uiEventTimer = 1500;
                m_pInstance->SetData(TYPE_EMI, DONE);
                bCanWalk = false;
                break;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        switch(pSummoned->GetEntry())
        {
            case NPC_CAVERNDEEP_AMBUSHER:
                ++m_uiInvadersSummoned;
                pSummoned->AI()->AttackStart(m_creature);
				lCaverndeepInvaders.push_back(pSummoned->GetObjectGuid());
                break;
            case NPC_GRUBBIS:
            case NPC_CHOMPER:
                pSummoned->AI()->AttackStart(m_creature);
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_CAVERNDEEP_AMBUSHER)
            --m_uiInvadersSummoned;
        if (pSummoned->GetEntry() == NPC_GRUBBIS)
            bCanWalk = true;
    }

    void DoSummonCaverndeepIncursion(bool first_cave, uint8 troggs_count_for_summon, bool summon_grubbis = false)
    {
        if (summon_grubbis)
        {
            m_creature->SummonCreature(NPC_GRUBBIS, -485.96f, -86.89f, -147.19f, 0.0f, TEMPSUMMON_DEAD_DESPAWN, NULL);
            m_creature->SummonCreature(NPC_CHOMPER, -484.50f, -85.50f, -147.50f, 0.0f, TEMPSUMMON_DEAD_DESPAWN, NULL);
        }
        for(uint8 i = 0; i < troggs_count_for_summon; ++i)
        {
            m_creature->SummonCreature(NPC_CAVERNDEEP_AMBUSHER, (first_cave ? -568.79f : -485.96f)+rand()%3, (first_cave ? -117.70f : -86.89f)+rand()%3, (first_cave ? -151.29f : -147.19f)+rand()%2, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
        }
    }

    void DoResetPhaseAndTimer()
    {
        m_uiEventPhase = 0;
        m_uiEventTimer = 0;
    }

    void DoKillAliveInvaders()
    {
        if (!lCaverndeepInvaders.empty())
        {
            m_uiInvadersSummoned = 0;

            for (GUIDList::iterator itr = lCaverndeepInvaders.begin(); itr != lCaverndeepInvaders.end(); ++itr)
            {
                if (Creature* pInvader = m_creature->GetMap()->GetCreature(*itr))
                    if (pInvader->isAlive())
                    {
                        pInvader->SetHealth(0);
                        pInvader->SetDeathState(JUST_DIED);
                    }
                //itr = lCaverndeepInvaders.erase(itr);
            }
        }
        else
            debug_log("SD0: Gnomeregan: List of Emi's invaders looks empty!");
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (bCanWalk)
            npc_escortAI::UpdateAI(uiDiff);

        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                if (!m_pInstance)
                    return;

				GameObject* GoCaveIn1 = m_pInstance->GetSingleGameObjectFromStorage(GO_CAVE_IN_1);
                GameObject* GoCaveIn2 = m_pInstance->GetSingleGameObjectFromStorage(GO_CAVE_IN_2);

                if (!GoCaveIn1 || !GoCaveIn2)
                {
                    error_log("SD0: Gnomeregan: A problem with selecting \"Cave In\" gameobjects. Returning..");
                    return;
                }

                switch(m_uiEventPhase)
                {
                    case 1:
                        DoScriptText(SAY_EMI_04, m_creature);
                        m_uiEventTimer = 3000;
                        break;
                    case 2:
                        DoScriptText(SAY_EMI_05, m_creature);
                        m_uiEventTimer = 3000;
                        break;
                    case 3:
                        m_creature->SetFacingTo(3.67f);
                        DoScriptText(SAY_EMI_06, m_creature);
                        m_uiEventTimer = 3000;
                        break;
                    case 4:
                        DoScriptText(SAY_EMI_07, m_creature);
                        GoCaveIn1->SetGoState(GO_STATE_ACTIVE);
                        DoSummonCaverndeepIncursion(true, 7);
                        m_uiEventTimer = 1500;
                        break;
                    case 5:
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;

                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                        DoSummonCaverndeepIncursion(true, 1);
                        m_uiEventTimer = 2000;
                        break;
                    case 15:
                        if (m_uiInvadersSummoned)
                        {
                            --m_uiEventPhase;
                            m_uiEventTimer = 3000;
                            break;
                        }
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;

                    case 20:
                        if (m_uiInvadersSummoned)
                        {
                            --m_uiEventPhase;
                            m_uiEventTimer = 3000;
                            break;
                        }
                        DoScriptText(SAY_EMI_10, m_creature);
                        m_uiEventTimer = 5000;
                        break;
                    case 21:
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;

                    case 30:
                        DoScriptText(SAY_EMI_11, m_creature);
                        m_uiEventTimer = 5000;
                        break;
                    case 31:
                        DoScriptText(SAY_EMI_12, m_creature);
                        m_uiEventTimer = 3000;
                        break;
                    case 32:
                        DoSummonCaverndeepIncursion(true, 7);
                        m_uiEventTimer = 2000;
                        break;
                    case 33:
                        DoScriptText(SAY_EMI_13, m_creature);
                        GoCaveIn1->SetGoState(GO_STATE_READY);
                        m_pInstance->SetData(TYPE_EMI, SPECIAL);
                        /*if (m_creature->getVictim())
                            DoCastSpellIfCan(m_creature->getVictim(), SPELL_EXPLOSION);*/
                        m_uiEventTimer = 800;
                        break;
                    case 34:
                        DoKillAliveInvaders();
                        m_uiEventTimer = 3000;
                        break;
                    case 35:
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;

                    case 40:
                        if (GetPlayerForEscort())
                            m_creature->SetFacingToObject(GetPlayerForEscort());
                        DoScriptText(SAY_EMI_15, m_creature);
                        m_uiEventTimer = 3000;
                        break;
                    case 41:
                        m_creature->SetFacingTo(0.56f);
                        DoScriptText(SAY_EMI_16, m_creature);
                        m_uiEventTimer = 3000;
                        break;
                    case 42:
                        GoCaveIn2->SetGoState(GO_STATE_ACTIVE);
                        DoSummonCaverndeepIncursion(false, 3);
                        DoScriptText(SAY_EMI_17, m_creature);
                        m_uiEventTimer = 1500;
                        break;
                    case 43:
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;

                    case 50:
                    case 51:
                    case 52:
                    case 53:
                    case 54:
                    case 55:
                        DoSummonCaverndeepIncursion(false, 1);
                        m_uiEventTimer = 2000;
                        break;
                    case 56:
                        if (m_uiInvadersSummoned)
                        {
                            --m_uiEventPhase;
                            m_uiEventTimer = 3000;
                            break;
                        }
                        m_uiEventTimer = 1500;
                        break;
                    case 57:
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;

                    case 60:
                        if (m_uiInvadersSummoned)
                        {
                            --m_uiEventPhase;
                            m_uiEventTimer = 3000;
                            break;
                        }
                        DoScriptText(SAY_EMI_18, m_creature);
                        m_uiEventTimer = 4000;
                        break;
                    case 61:
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;

                    case 70:
                        DoScriptText(SAY_EMI_19, m_creature);
                        m_uiEventTimer = 5000;
                        break;
                    case 71:
                        DoScriptText(SAY_EMI_20, m_creature);
                        m_uiEventTimer = 5000;
                        break;
                    case 72:
                        DoScriptText(SAY_EMI_21, m_creature);
                        DoSummonCaverndeepIncursion(false, 0, true);            // summon Grubbis and his basilisk Chomper
                        DoResetPhaseAndTimer();
                        break;
                    
                    case 80:
                    case 81:
                    case 82:
                        m_creature->HandleEmote(EMOTE_ONESHOT_CHEER);
                        m_uiEventTimer = 1500;
                        break;
                    case 83:
                        bCanWalk = true;
                        DoResetPhaseAndTimer();
                        break;
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

CreatureAI* GetAI_npc_blastmaster_emi_shortfuse(Creature* pCreature)
{
	return new npc_blastmaster_emi_shortfuseAI(pCreature);
}

bool GossipHello_npc_blastmaster_emi_shortfuse(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->GetInstanceData())
        if (pCreature->GetInstanceData()->GetData(TYPE_EMI) == NOT_STARTED)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I help you find source of the troggs [start event]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

	pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_blastmaster_emi_shortfuse(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiSender == GOSSIP_SENDER_MAIN)
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            pPlayer->CLOSE_GOSSIP_MENU();

            if (npc_blastmaster_emi_shortfuseAI* pEscortAI = dynamic_cast<npc_blastmaster_emi_shortfuseAI*>(pCreature->AI()))
                pEscortAI->Start(false, pPlayer);
        }
    }
    return true;
}

/*######
## go_thermaplug_button
######*/

bool GOUse_go_thermaplug_button(Player* pPlayer, GameObject* pGo)
{
    if (instance_gnomeregan* m_pInstance = (instance_gnomeregan*)pGo->GetInstanceData())
    {
        switch(pGo->GetEntry())
        {
            case GO_BUTTON_01:
                m_pInstance->SetData(TYPE_GNOME_FACE_01, SPECIAL);
                break;
            case GO_BUTTON_02:
                m_pInstance->SetData(TYPE_GNOME_FACE_02, SPECIAL);
                break;
            case GO_BUTTON_03:
                m_pInstance->SetData(TYPE_GNOME_FACE_03, SPECIAL);
                break;
            case GO_BUTTON_04:
                m_pInstance->SetData(TYPE_GNOME_FACE_04, SPECIAL);
                break;
            case GO_BUTTON_05:
                m_pInstance->SetData(TYPE_GNOME_FACE_05, SPECIAL);
                break;
            case GO_BUTTON_06:
                m_pInstance->SetData(TYPE_GNOME_FACE_06, SPECIAL);
                break;
        }
    }
    else
        debug_log("SD0: Gnomeregan: A Gnome Face button pushed, but no instance script connection. Spawned bombs will not be force killed.");

    return false;
}

/*######
## go_matrix_punchograph_3005D
######*/

enum eMatrix
{
    ITEM_RED_PUNCH_CARD                     = 9281,
    ITEM_SECURITY_DELTA_DATA_ACCESS_CARD    = 9327,
    SPELL_DISCOMBOBULATOR_RAY               = 3959,
    SPELL_LEARN_DISCOMBOBULATOR_RAY         = 11595,
    SPELL_CREATE_PRISMATIC_PUNCH_CARD       = 11545,
};

bool GossipHello_go_matrix_punchograph_3005D(Player* pPlayer, GameObject* pGo)
{
    if (!pPlayer->HasSpell(SPELL_DISCOMBOBULATOR_RAY) && pPlayer->HasItemCount(ITEM_SECURITY_DELTA_DATA_ACCESS_CARD, 1) &&
        pPlayer->HasSkill(SKILL_ENGINEERING) && pPlayer->GetSkillValue(SKILL_ENGINEERING) >= 160)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, "Schematic: Discombobulator Ray", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (pPlayer->HasItemCount(ITEM_RED_PUNCH_CARD, 1))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "Rescue data from the machine...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

	pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pGo), pGo->GetObjectGuid());
    return true;
}

bool GossipSelect_go_matrix_punchograph_3005D(Player* pPlayer, GameObject* pGo, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            if (pPlayer->HasItemCount(ITEM_SECURITY_DELTA_DATA_ACCESS_CARD, 1))
            {
                pPlayer->CastSpell(pPlayer, SPELL_LEARN_DISCOMBOBULATOR_RAY, false);
                pPlayer->DestroyItemCount(ITEM_SECURITY_DELTA_DATA_ACCESS_CARD, 1, true);
            }
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->CastSpell(pPlayer, SPELL_CREATE_PRISMATIC_PUNCH_CARD, false);
            break;
    }

    pPlayer->CLOSE_GOSSIP_MENU();
    return true;
}

/*######
## AddSC
######*/

void AddSC_gnomeregan()
{
    Script* pNewScript;
    
    pNewScript = new Script;
    pNewScript->Name = "at_clockwerk_run";
    pNewScript->pAreaTrigger = &AreaTrigger_at_clockwerk_run;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_kernobee";
    pNewScript->GetAI = &GetAI_npc_kernobee;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_kernobee;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_blastmaster_emi_shortfuse";
    pNewScript->GetAI = &GetAI_npc_blastmaster_emi_shortfuse;
    pNewScript->pGossipHello = &GossipHello_npc_blastmaster_emi_shortfuse;
    pNewScript->pGossipSelect = &GossipSelect_npc_blastmaster_emi_shortfuse;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "go_thermaplug_button";
    pNewScript->pGOUse = &GOUse_go_thermaplug_button;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_matrix_punchograph_3005D";
    pNewScript->pGossipHelloGO = &GossipHello_go_matrix_punchograph_3005D;
    pNewScript->pGossipSelectGO = &GossipSelect_go_matrix_punchograph_3005D;
    pNewScript->RegisterSelf();
}
