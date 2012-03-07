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

/*######
## npc_infected_peasant
######*/

enum
{
    SPELL_DEATHS_DOOR = 23127, // Green (weak)
    SPELL_SEETHING_PLAGUE = 23072, // Purple (strong)

    NPC_INJURED_PEASANT = 14484,
    NPC_PLAGUED_PEASANT = 14485,

    SPELL_ENTER_THE_LIGHT_DND = 23107
};

struct MANGOS_DLL_DECL npc_infected_peasantAI : public ScriptedAI
{
    npc_infected_peasantAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
        Reset();
    }

    uint32 m_uiDiseaseTimer;
    void Reset() 
    {
        //Instant Disease @ first Spawn
        m_uiDiseaseTimer = 0;
        //Only Plagued Peasants get the Seething Plague
        if(m_creature->GetEntry() == NPC_PLAGUED_PEASANT)
             m_creature->CastSpell(m_creature, SPELL_SEETHING_PLAGUE, false);

        m_creature->CastSpell(m_creature, 23196, false);
    }

    void AttackStart(Unit* pVictim) 
    {
        return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Holding this aura means that this NPC is saved
        if (m_creature->HasAura(SPELL_ENTER_THE_LIGHT_DND, EFFECT_INDEX_0))
            return;
        
       

        uint32 rnd = urand(0,100);

        if (m_uiDiseaseTimer <= uiDiff)
        {
            //30% Chance to get Diseased
            if(!m_creature->HasAura(SPELL_SEETHING_PLAGUE) && rnd <= 30)
            {
                m_creature->CastSpell(m_creature, SPELL_DEATHS_DOOR, false);
            }
            //Disease Timer between 1 und 10 Seconds
            m_uiDiseaseTimer = urand(1000,10000);
        }
        else
            m_uiDiseaseTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_infected_peasant(Creature* pCreature)
{
    return new npc_infected_peasantAI(pCreature);
}

/*######
## npc_eris_havenfire
######*/

// These seem correct
static const float aArcherSpawn[10][4] =
{
    { 3376.750f, -3041.969f, 172.639f, 2.359f },
    { 3383.315f, -3056.466f, 181.094f, 2.371f },
    { 3377.810f, -3059.429f, 180.500f, 2.025f },
    { 3358.776f, -3074.729f, 174.090f, 1.350f },
    { 3371.300f, -3068.288f, 175.841f, 1.279f },
    { 3348.956f, -3070.904f, 177.813f, 3.382f },
    { 3333.764f, -3051.669f, 174.158f, 1.357f },
    { 3313.438f, -3036.754f, 168.531f, 0.265f },
    { 3327.897f, -3021.678f, 170.103f, 6.144f },
    { 3362.131f, -3010.514f, 183.945f, 3.602f }
};

// Looks good
static const float aPeasantSpawn[15][3] =
{
    {3352.44f, -3048.32f, 164.833f},
    {3355.26f, -3052.93f, 165.72f},
    {3358.12f, -3050.71f, 165.307f},
    {3360.07f, -3052.31f, 165.3f},
    {3361.64f, -3055.29f, 165.295f},
    {3361.4f, -3052.17f, 165.261f},
    {3363.13f, -3056.21f, 165.285f},
    {3363.99f, -3054.49f, 165.342f},
    {3366.84f, -3053.95f, 165.541f},
    {3367.61f, -3056.84f, 165.88f},
    {3364.9f, -3052.68f, 165.321f},
    {3363.3f, -3051.2f, 165.266f},
    {3367.61f, -3051.14f, 165.517f},
    {3363.54f, -3049.64f, 165.238f},
    {3360.66f, -3049.14f, 165.261f}
};

// Dont know if these are right
static const float aFootsoldieSpawn[3][4] =
{
    {3347.603271f, -3045.536377f, 164.029877f, 1.814429f},
    {3363.609131f, -3037.187256f, 163.541885f, 2.277649f},
    {3349.105469f, -3056.500977f, 168.079468f, 1.857460f}

    /* Alternative
    { 3349.937f, -3056.875f, 168.141f, 1.622f },
    { 3370.527f, -3048.276f, 165.872f, 2.377f },
    { 3346.987f, -3052.782f, 165.360f, 1.662f }*/
};

static const uint32 aPeasantSpawnYell[] = {-1000696, -1000697, -1000698};
static const uint32 aPeasantRandomSay[] = {-1000699, -1000700, -1000701}; // TODO
static const uint32 aPeasantSaveSay[] = {-1000702, -1000703, -1000704, -1000705};

enum
{
    QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW = 7622,

    NPC_SCOURGE_ARCHER = 14489,
    NPC_SCOURGE_FOOTSOLDIER = 14486,

    QUEST_INVISIBILITY = 23196,
    SEE_PRIEST_INVIS = 23199,
    SPELL_BLESSING_OF_NORDRASSIL = 23108
};


struct npc_eris_havenfireAI;
npc_eris_havenfireAI* npc_global_eris;

struct MANGOS_DLL_DECL npc_eris_havenfireAI : public ScriptedAI
{
    npc_eris_havenfireAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_bIsQuestInProgress = false;
        Reset();
    }

    bool m_bIsQuestInProgress, m_bFootsoldiersSpawned;
    uint64 m_uiMainTimer;
    uint32 m_uiDoomCheck, m_uiFootsoldierTimer1, m_uiFootsoldierTimer2, m_uiFootsoldierTimer3;
    uint8 m_uiPhase, m_uiCurrentWave, m_uiKillCounter, m_uiSaveCounter, m_uiTotalSaved, m_uiTotalKilled;
    ObjectGuid m_playerGuid;
    GUIDList m_lSummonedGUIDList;

    void Reset()
    {
        // No need to continue while 'WE ARE IN'!!
        if (m_bIsQuestInProgress)
            return;

        m_bFootsoldiersSpawned = false;
        m_bIsQuestInProgress = false;
        m_uiMainTimer = 5000;
        m_uiFootsoldierTimer1 = urand(0,1)*5000 + 20000;
        m_uiFootsoldierTimer2 = 30000;
        m_uiFootsoldierTimer3 = 60000;
        m_uiDoomCheck = 5000;
        m_uiPhase = 1;
        m_uiCurrentWave = 0;
        m_uiKillCounter = 0;
        m_uiSaveCounter = 0;
        m_uiTotalSaved = 0;
        m_uiTotalKilled = 0;
        m_playerGuid.Clear();

        m_creature->CastSpell(m_creature, QUEST_INVISIBILITY, true);

        if (!m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

        // Despawn when something is spawned
        if (!m_lSummonedGUIDList.empty())
        {
            for (GUIDList::const_iterator itr = m_lSummonedGUIDList.begin(); itr != m_lSummonedGUIDList.end(); ++itr)
                if (Creature* pSummoned = m_creature->GetMap()->GetCreature(*itr))
                    pSummoned->ForcedDespawn();
        }
        m_lSummonedGUIDList.clear();
    }

    void PhaseEnded(bool bFailed, bool bWave)
    {
        Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);

        // Continue only when we have starting player
        if (!pPlayer)
            return;

        // Failed
        if (bFailed && !bWave)
        {
            DoScriptText(urand(0, 1) ? -1000706 : -1000707, m_creature);
            if (pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW);

            m_bIsQuestInProgress = false;
            Reset();
            return;
        }

        // Wave completed
        if (!bFailed && bWave)
        {
            DoScriptText(-1000709, m_creature);

            m_creature->CastSpell(pPlayer, SPELL_BLESSING_OF_NORDRASSIL, true);

            //Summon Soldiers (Wave 1 -> 9, Wave 2 -> 6, Wave 3 -> 8, ...)
            DoSummonFootsoldier(6 + ((m_uiCurrentWave%2)*2));

            //Timer for the Footsoldier Waves
            m_uiFootsoldierTimer1 = urand(0,1)*5000 + 20000;
            m_uiFootsoldierTimer2 = 30000;
            m_uiFootsoldierTimer3 = 60000;

            //Footsoldiers are Spawned continously after the first Wave
            m_bFootsoldiersSpawned = true;

            // m_uiSaveCounter and m_uiKillCounter are only temporar values
            m_uiSaveCounter = 0;
            m_uiKillCounter = 0;
            return;
        }

        // Whole event completed
        if (!bFailed && !bWave)
        {
            DoScriptText(-1000708, m_creature);
            if (pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
                pPlayer->AreaExploredOrEventHappens(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW);

            m_bIsQuestInProgress = false;
            Reset();
            return; // Formal return?
        }
    }

    void DoNextWave()
    {
        ++m_uiCurrentWave;

        //Count for Peasants 10 - 15
        uint8 uiShorter = 10 + m_uiCurrentWave;

        //Random Peasant for Say
        uint8 uiRandomPeasant = urand(0, uiShorter);

        for(uint8 i = 0; i < uiShorter; ++i)
        {
            //Injured or plagued Peasant
            uint32 m_uiPeasantType = NPC_INJURED_PEASANT;
            //Plagued Peasants have a 5% + Nr of Wave % Chance (10%, 15%, ...)
            if(urand(0,100) <= 5 + 5 * m_uiCurrentWave)
                m_uiPeasantType = NPC_PLAGUED_PEASANT;

            if (Creature* pTemp = m_creature->SummonCreature(m_uiPeasantType, aPeasantSpawn[i][0], aPeasantSpawn[i][1], aPeasantSpawn[i][2], 0, TEMPSUMMON_DEAD_DESPAWN, 0))
            {                
                m_lSummonedGUIDList.push_back(pTemp->GetGUID());

                float fX, fY, fZ;
                pTemp->GetRandomPoint(3343.270f, -3018.100f, 161.72f, 5.0f, fX, fY, fZ);
                pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);

                if (i == uiRandomPeasant)
                    DoScriptText(aPeasantSpawnYell[urand(0,2)], pTemp);
            }
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 /*uiMotionType*/, uint32 uiPointId)
    {
        if(uiPointId == 0)
        {
            float fX, fY,fZ;
            pSummoned->GetRandomPoint(3332.767f, -2979.002f, 160.97f, 5.0f, fX, fY, fZ);
            pSummoned->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
        }
        if (uiPointId == 1)
        {
            if (m_uiSaveCounter >= 10 + m_uiCurrentWave) // When saved peasants exceed maximum peasants, something went wrong ^^
                debug_log("SD0: npc_eris_havenfire: Current wave %u was not reset properly in void WaveFinished().", m_uiCurrentWave);

            ++m_uiSaveCounter;
            ++m_uiTotalSaved;

            // When counted, force despawn. I don't know exactly when they should disappear
            pSummoned->GetMotionMaster()->Clear(false);

            if (pSummoned->HasAura(SPELL_DEATHS_DOOR, EFFECT_INDEX_0))
                pSummoned->RemoveAurasDueToSpell(SPELL_DEATHS_DOOR);

            if (pSummoned->HasAura(SPELL_SEETHING_PLAGUE, EFFECT_INDEX_0))
                pSummoned->RemoveAurasDueToSpell(SPELL_SEETHING_PLAGUE);

            uint8 uiRandomPeasant = urand(1,10);
            if (uiRandomPeasant == 5)
                DoScriptText(aPeasantSaveSay[urand(0,3)], pSummoned);

            pSummoned->CastSpell(pSummoned, SPELL_ENTER_THE_LIGHT_DND, false);
            pSummoned->ForcedDespawn(4000);
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        /* We do not want to count staying peasants.
        It means that they are saved. Saved peasants are ForcedDespawn(),
        which triggers SummonedCreatureJustDied.
        */

        if (pSummoned->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE &&
           (pSummoned->GetEntry() == NPC_INJURED_PEASANT || pSummoned->GetEntry() == NPC_PLAGUED_PEASANT))
        {
            ++m_uiKillCounter;
            ++m_uiTotalKilled;
        }

        pSummoned->RemoveCorpse();
    }

    void DoSummonFootsoldier(uint32 count)
    {
        for(uint8 i = 0; i < count + 1; ++i)
        {
            float fX, fY, fZ;
            m_creature->GetRandomPoint(aFootsoldieSpawn[count%3][0], aFootsoldieSpawn[count%3][1], aFootsoldieSpawn[count%3][2], 5.0f, fX, fY, fZ);
            if (Creature* pTemp = m_creature->SummonCreature(NPC_SCOURGE_FOOTSOLDIER, fX, fY, fZ, aFootsoldieSpawn[count%3][3], TEMPSUMMON_DEAD_DESPAWN, 5000))
            {
                m_lSummonedGUIDList.push_back(pTemp->GetGUID());
            }
        }
    }

    void DoSpawnArchers ()
    {
        for(uint8 i = 0; i < 10; ++i)
        {
            if (Creature* pTemp = m_creature->SummonCreature(NPC_SCOURGE_ARCHER, aArcherSpawn[i][0], aArcherSpawn[i][1], aArcherSpawn[i][2], aArcherSpawn[i][3], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 360000))
            {
                m_lSummonedGUIDList.push_back(pTemp->GetGUID());
            }
        }
    }

    void functionOfDoom(Creature* creature)
    {
        if (!creature->CanHaveThreatList())
            return;

        GUIDVector vGuids;
        creature->FillGuidsListFromThreatList(vGuids);

        if (!vGuids.empty())
        {
            for (GUIDVector::const_iterator itr = vGuids.begin(); itr != vGuids.end(); ++itr)
            {
                if (Unit* pTarget = creature->GetMap()->GetUnit(*itr))
                {
                    if(pTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        if(((Player*)pTarget)->GetGUID() != m_playerGuid)
                        {
                            //Get the frakkin Cleaner to kill this guy
                        }
                    }
                }
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
         // Nothing is needed to update when the event is not in progress
        if (!m_bIsQuestInProgress)
            return;
        
        if (m_uiDoomCheck <= uiDiff)
        {
            for (GUIDList::const_iterator itr = m_lSummonedGUIDList.begin(); itr != m_lSummonedGUIDList.end(); ++itr)
                if (Creature* pSummoned = m_creature->GetMap()->GetCreature(*itr))
                    functionOfDoom(pSummoned);
        }
        else
            m_uiDoomCheck -= uiDiff;

        //Different Footsoldier spawns
        if(m_bFootsoldiersSpawned)
        {
            if (m_uiFootsoldierTimer1 <= uiDiff)
            {
                DoSummonFootsoldier(2);
                m_uiFootsoldierTimer1 = urand(0,1)*5000 + 20000;
            }
            else
                m_uiFootsoldierTimer1 -= uiDiff;

            if (m_uiFootsoldierTimer2 <= uiDiff)
            {
                DoSummonFootsoldier(1);
                m_uiFootsoldierTimer2 = 30000;
            }
            else
                m_uiFootsoldierTimer2 -= uiDiff;

            if (m_uiFootsoldierTimer3 <= uiDiff)
            {
                DoSummonFootsoldier(3+((m_uiCurrentWave%2)*2));
                m_uiFootsoldierTimer3 = 60000;
            }
            else
                m_uiFootsoldierTimer3 -= uiDiff;
        }

        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
        {
            pPlayer->DeleteThreatList();
            if (pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) != QUEST_STATUS_INCOMPLETE || pPlayer->isDead())
            {
                PhaseEnded(true, false);
                Reset();
                return;
            }
        }

        if (m_uiPhase)
        {
            // Reaching 15 dead peasants means fail for us
            if (m_uiTotalKilled >= 15)
            {
                PhaseEnded(true, false);
                return;
            }

            // Do next step
            else if (m_uiKillCounter + m_uiSaveCounter == 10 + m_uiCurrentWave)
            {
                // When we saved 50 peasants
                if (m_uiTotalSaved >= 50)
                    PhaseEnded(false, false);
                // When the wave completed, but still we are not done
                else
                    PhaseEnded(false, true);

                return;
            }

            // No more phases or no wave
            if (m_uiPhase > 6 || !m_uiCurrentWave)
                return;

            if (m_uiMainTimer < uiDiff)
            {
                switch(m_uiPhase)
                {
                    case 1: // Spawn Archers
                        DoSpawnArchers();
                        m_uiMainTimer = 2000;
                        break;
                    case 2: // Wave 1
                        DoNextWave();
                        m_uiMainTimer = 38000;
                        break;
                    case 3: // Wave 2
                        DoNextWave();
                        m_uiMainTimer = 38000;
                        break;
                    case 4: // Wave 3
                        DoNextWave();
                        m_uiMainTimer = 38000;
                        break;
                    case 5: // Wave 4
                        DoNextWave();
                        m_uiMainTimer = 38000;
                        break;
                    case 6: // Wave 5
                        DoNextWave();
                        m_uiMainTimer = 38000;
                        break;
                }

                /* Just a note:
                Phase is NOT Wave. Phase means a part of this event in this script.
                Wave means a wave of peasants. We have maximum of 5 waves. You never can be clear enough :P
                */
                ++m_uiPhase;
            }
            else
                m_uiMainTimer -= uiDiff;
        }
        else // Impossible to have m_bIsQuestInProgress and !m_uiPhase
            debug_log("SD0: npc_eris_havenfire: No phase detected!");
    }
};

bool QuestAccept_npc_eris_havenfire(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW)
    {
        // Everybody loves dynamic casts <3
        if (npc_eris_havenfireAI* pEris = dynamic_cast<npc_eris_havenfireAI*>(pCreature->AI()))
        {
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            pEris->m_bIsQuestInProgress = true;
            pEris->m_uiCurrentWave = 1;
            pEris->m_playerGuid = pPlayer->GetObjectGuid();
        }
    }

    return true;
}

CreatureAI* GetAI_npc_eris_havenfire(Creature* pCreature)
{
    return new npc_eris_havenfireAI(pCreature);
}

/*######
## mob_scourge_archer
######*/

enum
{
    SHOOT = 23073,
};

struct MANGOS_DLL_DECL mob_scourge_archerAI : public ScriptedAI
{
    mob_scourge_archerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        SetCombatMovement(false);
        //Are they not attackable ???
        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        Reset();
    }

    std::list<Creature*> pPeasants;
    uint32 m_uiShotTimer;
    bool m_bSkippedFirstShot;

    void Reset()
    {

        //Values need to be set
        m_creature->SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, 162);
        m_creature->SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, 186);
        m_creature->UpdateDamagePhysical(RANGED_ATTACK);
        m_creature->SetArmor(3791);

        //get a bow
        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_UNEQUIP, 6231);

        //get immune
        m_creature->CastSpell(m_creature, 29230, true);

        m_creature->CastSpell(m_creature, QUEST_INVISIBILITY, false);

        //dont move
        SetCombatMovement(false);

        //shoot timer
        m_uiShotTimer = 2000 + urand(0, 400); 
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (pDoneBy->IsCharmerOrOwnerPlayerOrPlayerItself())
            if (!((Player*)pDoneBy)->isGameMaster())
                uiDamage = 0;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        //Peasant list
        pPeasants.clear();
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_INJURED_PEASANT, 60.0f);
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_PLAGUED_PEASANT, 60.0f);

        if (m_uiShotTimer)
        {
            if(m_uiShotTimer <= uiDiff)
            {
                //Get random Peasants for every Shot???
                uint32 pPeasantRandom = urand(0,pPeasants.size()), j = 1;
                for(std::list<Creature*>::iterator i = pPeasants.begin(); i != pPeasants.end(); ++i)
                {
                    if((*i)->isAlive() && j == pPeasantRandom && m_creature->IsInRange((*i), 0.0f, 40.0f, true))
                    {
                        DoCastSpellIfCan((*i), SHOOT);
                        m_creature->AddThreat((*i));
                    }
                    else if (j == pPeasantRandom)
                         pPeasantRandom = urand(j,pPeasants.size());

                    j++;
                }
                m_uiShotTimer = 2000 + urand(0, 400);
            }
            else
                m_uiShotTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_mob_scourge_archer(Creature* pCreature)
{
    return new mob_scourge_archerAI(pCreature);
}


/*######
## mob_scourge_footsoldier
######*/

struct MANGOS_DLL_DECL mob_scourge_footsoldierAI : public ScriptedAI
{
    mob_scourge_footsoldierAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    std::list<Creature*> pPeasants;
    Player* pPlayer;
    bool m_bSkippedFirstShot;

    void Reset()
    {

        //Values need to be set
        m_creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 108);
        m_creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 126);
        m_creature->UpdateDamagePhysical(BASE_ATTACK);
        m_creature->SetArmor(3435);

        m_creature->CastSpell(m_creature, QUEST_INVISIBILITY, false);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        //Peasant list
        pPeasants.clear();
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_INJURED_PEASANT, 200.0f);
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_PLAGUED_PEASANT, 200.0f);

        Player* pTempPlayer = GetPlayerAtMinimumRange(200.0f);

        if(pTempPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
            pPlayer = pTempPlayer;

        if(!m_creature->isInCombat())
        {
            m_creature->AddThreat(pPlayer);
            for(std::list<Creature*>::iterator i = pPeasants.begin(); i != pPeasants.end(); ++i)
            {
                if((*i)->isAlive())
                {
                    m_creature->AddThreat((*i));
                }
            }
        }
        m_creature->SelectHostileTarget();
    }
};

CreatureAI* GetAI_mob_scourge_footsoldier(Creature* pCreature)
{
    return new mob_scourge_footsoldierAI(pCreature);
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

    pNewscript = new Script;
    pNewscript->Name = "npc_eris_havenfire";
    pNewscript->GetAI = &GetAI_npc_eris_havenfire;
    pNewscript->pQuestAcceptNPC = &QuestAccept_npc_eris_havenfire;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "npc_infected_peasant";
    pNewscript->GetAI = &GetAI_npc_infected_peasant;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "mob_scourge_archer";
    pNewscript->GetAI = &GetAI_mob_scourge_archer;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "mob_scourge_footsoldier";
    pNewscript->GetAI = &GetAI_mob_scourge_footsoldier;
    pNewscript->RegisterSelf();
}
