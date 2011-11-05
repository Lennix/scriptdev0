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
SDName: Feralas
SD%Complete: 100
SDComment: Quest support: 976, 3520, 2767. Special vendor Gregan Brewspewer
SDCategory: Feralas
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_gregan_brewspewer
######*/

bool GossipHello_npc_gregan_brewspewer(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pCreature->isVendor() && pPlayer->GetQuestStatus(3909) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Buy somethin', will ya?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(2433, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_gregan_brewspewer(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        pPlayer->SEND_GOSSIP_MENU(2434, pCreature->GetObjectGuid());
    }
    if (uiAction == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());
    return true;
}

/*######
## npc_oox22fe
######*/

enum
{
    SAY_OOX_START           = -1000287,
    SAY_OOX_AGGRO1          = -1000288,
    SAY_OOX_AGGRO2          = -1000289,
    SAY_OOX_AMBUSH          = -1000290,
    SAY_OOX_END             = -1000292,

    NPC_YETI                = 7848,
    NPC_GORILLA             = 5260,
    NPC_WOODPAW_REAVER      = 5255,
    NPC_WOODPAW_BRUTE       = 5253,
    NPC_WOODPAW_ALPHA       = 5258,
    NPC_WOODPAW_MYSTIC      = 5254,

    QUEST_RESCUE_OOX22FE    = 2767
};

struct MANGOS_DLL_DECL npc_oox22feAI : public npc_escortAI
{
    npc_oox22feAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void WaypointReached(uint32 i)
    {
        switch (i)
        {
            // First Ambush(3 Yetis)
            case 11:
                DoScriptText(SAY_OOX_AMBUSH,m_creature);
                m_creature->SummonCreature(NPC_YETI, -4841.01f, 1593.91f, 73.42f, 3.98f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_YETI, -4837.61f, 1568.58f, 78.21f, 3.13f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_YETI, -4841.89f, 1569.95f, 76.53f, 0.68f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                break;
            //Second Ambush(3 Gorillas)
            case 21:
                DoScriptText(SAY_OOX_AMBUSH,m_creature);
                m_creature->SummonCreature(NPC_GORILLA, -4595.81f, 2005.99f, 53.08f, 3.74f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_GORILLA, -4597.53f, 2008.31f, 52.70f, 3.78f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_GORILLA, -4599.37f, 2010.59f, 52.77f, 3.84f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                break;
            //Third Ambush(4 Gnolls)
            case 30:
                DoScriptText(SAY_OOX_AMBUSH,m_creature);
                m_creature->SummonCreature(NPC_WOODPAW_REAVER, -4425.14f, 2075.87f, 47.77f, 3.77f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_BRUTE , -4426.68f, 2077.98f, 47.57f, 3.77f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_MYSTIC, -4428.33f, 2080.24f, 47.43f, 3.87f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_WOODPAW_ALPHA , -4430.04f, 2075.54f, 46.83f, 3.81f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                break;
            case 37:
                DoScriptText(SAY_OOX_END,m_creature);
                // Award quest credit
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_RESCUE_OOX22FE, m_creature);
                break;
        }
    }

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
    }

    void Aggro(Unit* who)
    {
        //For an small probability the npc says something when he get aggro
        switch(urand(0, 9))
        {
           case 0: DoScriptText(SAY_OOX_AGGRO1, m_creature); break;
           case 1: DoScriptText(SAY_OOX_AGGRO2, m_creature); break;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }
};

CreatureAI* GetAI_npc_oox22fe(Creature* pCreature)
{
    return new npc_oox22feAI(pCreature);
}

bool QuestAccept_npc_oox22fe(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RESCUE_OOX22FE)
    {
        DoScriptText(SAY_OOX_START, pCreature);
        //change that the npc is not lying dead on the ground
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);

        if (pPlayer->GetTeam() == ALLIANCE)
            pCreature->setFaction(FACTION_ESCORT_A_PASSIVE);

        if (pPlayer->GetTeam() == HORDE)
            pCreature->setFaction(FACTION_ESCORT_H_PASSIVE);

        if (npc_oox22feAI* pEscortAI = dynamic_cast<npc_oox22feAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

/*######
## npc_screecher_spirit
######*/

bool GossipHello_npc_screecher_spirit(Player* pPlayer, Creature* pCreature)
{
    pPlayer->SEND_GOSSIP_MENU(2039, pCreature->GetObjectGuid());
    pPlayer->TalkedToCreature(pCreature->GetEntry(), pCreature->GetObjectGuid());
    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    return true;
}

/*######
## npc_feero_ironhand
######*/

enum eFeeroIronhand
{
	QUEST_SUPPLIES_TO_AUBERDINE = 976,

	NPC_DARK_STRAND_ASSASSIN	= 3879, // 1st
	NPC_FORSAKEN_SCOUT			= 3893, // 2st
	NPC_CAEDAKAR_THE_VICIOUS	= 3900, // 3st
	NPC_ALIGAR_THE_TORMENTOR	= 3898,
	NPC_BALIZAR_THE_UMBRAGE		= 3899,

    SAY_FEERO_1                 = -1000585,
    SAY_FEERO_2                 = -1000586,
    SAY_FEERO_3                 = -1000587,
    SAY_FEERO_4                 = -1000588,
    SAY_FEERO_5                 = -1000589,
    SAY_FEERO_6                 = -1000590,
    SAY_FEERO_7                 = -1000591,
    SAY_SCOUT                   = -1000592,
    SAY_BALIZAR                 = -1000593
};

struct MANGOS_DLL_DECL npc_feero_ironhandAI : public npc_escortAI
{
	npc_feero_ironhandAI(Creature* pCreature) : npc_escortAI(pCreature)	{Reset();}

	Creature* Ambushers[10];
	Creature* SummonedAttacker;
	uint32 EventPhase;
	uint32 EventTimer;
	bool CanWalk;

	void Reset()
	{
		if (HasEscortState(STATE_ESCORT_ESCORTING))
			return;

		for(uint8 i = 0; i < 10; ++i)
			Ambushers[i] = 0;
		SummonedAttacker = 0;

		EventPhase = 0;
		EventTimer = 0;

		CanWalk = true;
	};

	void WaypointReached(uint32 uiPoint)
	{
		Player* pPlayer = GetPlayerForEscort();
		if (!pPlayer)
			return;

		switch(uiPoint)
		{
			case 13:
                DoScriptText(SAY_FEERO_1, m_creature);
				Ambushers[0] = SpawnAttacker(NPC_DARK_STRAND_ASSASSIN, 3514.47f, 233.55f, 12.42f);
				Ambushers[1] = SpawnAttacker(NPC_DARK_STRAND_ASSASSIN, 3521.09f, 225.08f, 10.87f);
				Ambushers[2] = SpawnAttacker(NPC_DARK_STRAND_ASSASSIN, 3523.87f, 198.74f, 10.63f);
				Ambushers[3] = SpawnAttacker(NPC_DARK_STRAND_ASSASSIN, 3513.08f, 188.02f, 9.15f);
				CanWalk = false;
				EventTimer = 5000; // start event
				EventPhase = 1;
				break;
			case 17:
                DoScriptText(SAY_FEERO_3, m_creature);
				EventPhase = 10;
				EventTimer = 4000;
				CanWalk = false;
				break;
			case 24:
                DoScriptText(SAY_FEERO_5, m_creature);
				Ambushers[7] = SpawnAttacker(NPC_CAEDAKAR_THE_VICIOUS, 4219.18f, 123.06f, 47.63f);
				Ambushers[8] = SpawnAttacker(NPC_ALIGAR_THE_TORMENTOR, 4249.67f, 121.80f, 39.60f);
				Ambushers[9] = SpawnAttacker(NPC_BALIZAR_THE_UMBRAGE, 4239.72f, 87.07f, 44.19f);
				EventPhase = 20;
				EventTimer = 4000;
				CanWalk = false;
				break;
			case 82:
				m_creature->SetFacingToObject(pPlayer);
                DoScriptText(SAY_FEERO_7, m_creature);
				pPlayer->GroupEventHappens(QUEST_SUPPLIES_TO_AUBERDINE, m_creature);
				m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP + UNIT_NPC_FLAG_QUESTGIVER);
				break;
		}
	}

	/*void JustSummoned(Creature* pSummon)
	{
		if (pSummon)
			pSummon->AI()->AttackStart(m_creature);
	}*/

	Creature* SpawnAttacker(uint32 creature_entry, float x, float y, float z, uint32 despawn_time = 30000)
	{
		if (SummonedAttacker = m_creature->SummonCreature(creature_entry, x, y, z, 0, (TempSummonType)TEMPSUMMON_CORPSE_TIMED_DESPAWN, despawn_time))
			SummonedAttacker->SetFacingToObject(m_creature);
		return SummonedAttacker;
	}

	bool CanContinueInEscort(uint8 which_phase)
	{
		switch(which_phase)
		{
			case 1:
				if (Ambushers[0] && Ambushers[0]->isAlive() && !Ambushers[0]->isDead())
					return false;
				if (Ambushers[1] && Ambushers[1]->isAlive() && !Ambushers[1]->isDead())
					return false;
				if (Ambushers[2] && Ambushers[2]->isAlive() && !Ambushers[2]->isDead())
					return false;
				if (Ambushers[3] && Ambushers[3]->isAlive() && !Ambushers[3]->isDead())
					return false;
				break;
			case 2:
				if (Ambushers[4] && Ambushers[4]->isAlive() && !Ambushers[4]->isDead())
					return false;
				if (Ambushers[5] && Ambushers[5]->isAlive() && !Ambushers[5]->isDead())
					return false;
				if (Ambushers[6] && Ambushers[6]->isAlive() && !Ambushers[6]->isDead())
					return false;
				break;
			case 3:
				if (Ambushers[7] && Ambushers[7]->isAlive() && !Ambushers[7]->isDead())
					return false;
				if (Ambushers[8] && Ambushers[8]->isAlive() && !Ambushers[8]->isDead())
					return false;
				if (Ambushers[9] && Ambushers[9]->isAlive() && !Ambushers[9]->isDead())
					return false;
				break;
		}
		return true;
	}

	void JustDied(Unit*)
	{
		if (Player* pPlayer = GetPlayerForEscort())
			pPlayer->FailQuest(QUEST_SUPPLIES_TO_AUBERDINE);
	}

	void UpdateAI(const uint32 uiDiff)
	{
		Player* pPlayer = GetPlayerForEscort();

		if (EventTimer && pPlayer)
		{
			if (EventTimer <= uiDiff)
			{
				switch(EventPhase)
				{
					case 1:
						if (Ambushers[0])
							Ambushers[0]->AI()->AttackStart(m_creature);
						if (Ambushers[1])
							Ambushers[1]->AI()->AttackStart(m_creature);
						if (Ambushers[2])
							Ambushers[2]->AI()->AttackStart(m_creature);
						if (Ambushers[3])
							Ambushers[3]->AI()->AttackStart(m_creature);
						EventTimer = 1;
						break;
					case 2:
						if (!CanContinueInEscort(1))
							--EventPhase; // repeat this phase
						EventTimer = 1;
						break;
					case 3:
                        DoScriptText(SAY_FEERO_2, m_creature);
						EventTimer = 4500;
						break;
					case 4:
						CanWalk = true;
						EventTimer = 0;
						break;

					case 10:
						Ambushers[4] = SpawnAttacker(NPC_FORSAKEN_SCOUT, 3832.37f, 108.67f, 11.39f);
						Ambushers[5] = SpawnAttacker(NPC_FORSAKEN_SCOUT, 3827.99f, 103.19f, 11.31f);
						EventTimer = 2000;
						break;
					case 11:
						Ambushers[6] = SpawnAttacker(NPC_FORSAKEN_SCOUT, 3832.79f, 102.48f, 11.79f);
						if (Ambushers[6])
						{
                            DoScriptText(SAY_SCOUT, Ambushers[6]);
							Ambushers[6]->SetFacingToObject(m_creature);
							m_creature->SetFacingToObject(Ambushers[6]);
							Ambushers[6]->HandleEmote(EMOTE_ONESHOT_POINT);
						}
						EventTimer = 7000;
						break;
					case 12:
						if (Ambushers[4])
							Ambushers[4]->AI()->AttackStart(m_creature);
						if (Ambushers[5])
							Ambushers[5]->AI()->AttackStart(m_creature);
						if (Ambushers[6])
							Ambushers[6]->AI()->AttackStart(m_creature);
						EventTimer = 1;
						break;
					case 13:
						if (!CanContinueInEscort(2))
							--EventPhase; // repeat this phase
						EventTimer = 1;
						break;
					case 14:
						CanWalk = true;
						EventTimer = 0;
						break;

					case 20:
						if (Ambushers[8])
						{
							m_creature->SetFacingToObject(Ambushers[8]);
                            DoScriptText(SAY_BALIZAR, Ambushers[8]);
						}
						EventTimer = 7000;
						break;
					case 21:
                        DoScriptText(SAY_FEERO_6, m_creature);
						EventTimer = 5000;
						break;
					case 22:
						if (Ambushers[7])
							Ambushers[7]->AI()->AttackStart(m_creature);
						if (Ambushers[8])
							Ambushers[8]->AI()->AttackStart(m_creature);
						if (Ambushers[9])
							Ambushers[9]->AI()->AttackStart(m_creature);
						break;
					case 23:
						if (!CanContinueInEscort(3))
							--EventPhase;
						EventTimer = 1;
						break;
					case 24:
						CanWalk = true;
						EventTimer = 0;
						break;
				}
				++EventPhase;
			}
				else EventTimer -= uiDiff;
		}

		if (CanWalk)
			npc_escortAI::UpdateAI(uiDiff);

		if (m_creature->SelectHostileTarget() || m_creature->getVictim())
			DoMeleeAttackIfReady();
	}
};
CreatureAI* GetAI_npc_feero_ironhand(Creature* pCreature)
{
	return new npc_feero_ironhandAI (pCreature);
}

bool QuestAccept_npc_feero_ironhand(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
	if (pQuest->GetQuestId() == QUEST_SUPPLIES_TO_AUBERDINE)
	{
		if (npc_feero_ironhandAI* pEscortAI = dynamic_cast<npc_feero_ironhandAI*>(pCreature->AI()))
			pEscortAI->Start(true, pPlayer, pQuest, true, false);
	}
	return true;
}

/*######
## AddSC
######*/

void AddSC_feralas()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "npc_gregan_brewspewer";
    pNewscript->pGossipHello = &GossipHello_npc_gregan_brewspewer;
    pNewscript->pGossipSelect = &GossipSelect_npc_gregan_brewspewer;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_oox22fe";
    pNewscript->GetAI = &GetAI_npc_oox22fe;
    pNewscript->pQuestAcceptNPC = &QuestAccept_npc_oox22fe;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_screecher_spirit";
    pNewscript->pGossipHello = &GossipHello_npc_screecher_spirit;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_feero_ironhand";
    pNewscript->GetAI = &GetAI_npc_feero_ironhand;
    pNewscript->pQuestAcceptNPC = &QuestAccept_npc_feero_ironhand;
    pNewscript->RegisterSelf();
}
