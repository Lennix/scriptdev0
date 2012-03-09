#include "precompiled.h"
#include "escort_ai.h"
/*author: zero
 *script complete: 80% 
 *need test
 *we need a closer look at the deathdoors at wave2
 *we need a trigger to keep eventplayer in combat
 *maybe we have to implement a full working random spawn calculation for peasants
 *soldier spawn is probably wrong
 */

/*######
## npc_infected_peasant
######*/

enum
{
    SPELL_DEATHS_DOOR = 23127, // Green (weak)
    SPELL_SEETHING_PLAGUE = 23072, // Purple (strong)

    NPC_INJURED_PEASANT = 14484,
    NPC_PLAGUED_PEASANT = 14485,
    
    QUEST_INVISIBILITY = 23196,
    SPELL_ENTER_THE_LIGHT_DND = 23107
};

struct MANGOS_DLL_DECL npc_infected_peasantAI : public ScriptedAI
{
    npc_infected_peasantAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
        Reset();
    }

    uint32 m_uiDiseaseTimer, rnd;

    void Reset() 
    {
        
        m_uiDiseaseTimer = urand(1000,10000);
        rnd = urand(0,100);

        //Only Plagued Peasants get the Seething Plague
        if(m_creature->GetEntry() == NPC_PLAGUED_PEASANT)
             m_creature->CastSpell(m_creature, SPELL_SEETHING_PLAGUE, false);
        if(!m_creature->HasAura(SPELL_SEETHING_PLAGUE) && rnd <= 30)
                m_creature->CastSpell(m_creature, SPELL_DEATHS_DOOR, false);
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
        
        if (!m_creature->HasAura(QUEST_INVISIBILITY))
            m_creature->CastSpell(m_creature, QUEST_INVISIBILITY, false);

        rnd = urand(0,100);

        if (m_uiDiseaseTimer <= uiDiff)
        {
            //30% Chance to get Diseased
            if(!m_creature->HasAura(SPELL_SEETHING_PLAGUE) && rnd <= 30)
                m_creature->CastSpell(m_creature, SPELL_DEATHS_DOOR, false);
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
    NPC_CLEANER = 14503,

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

    bool    m_bIsQuestInProgress, 
            m_bFootsoldiersSpawned, 
            m_bCleaningInProgress;

    uint64  m_uiMainTimer;

    uint32  m_uiDoomCheck,
            m_uiFootsoldierSpawnTimer,
            m_uiFootsoldierTimer1,
            m_uiFootsoldierTimer2,
            m_uiFootsoldierTimer3,
            m_uiKillCounter[5],
            m_uiSaveCounter[5],
            m_uiPeasantCount[5];

    uint8   m_uiPhase, 
            m_uiCurrentWave, 
            m_uiTotalSaved, 
            m_uiTotalKilled, 
            m_uiFootsoldiersSpawnCount;

    ObjectGuid  m_playerGuid;
    GUIDList    m_lSummonedGUIDList;

    std::list<Player*> m_lToCleanPlayers;
    std::list<Creature*> m_lCleaner;

    void Reset()
    {
        // No need to continue while 'WE ARE IN'!!
        if (m_bIsQuestInProgress)
            return;

        //Declarations
        m_bFootsoldiersSpawned = false;
        m_bIsQuestInProgress = false;
        m_bCleaningInProgress = false;
       
        m_uiMainTimer = 5000;
        m_uiFootsoldierSpawnTimer = 1000;
        m_uiFootsoldierTimer1 = urand(0,1)*5000 + 20000;
        m_uiFootsoldierTimer2 = 30000;
        m_uiFootsoldierTimer3 = 60000;
        m_uiDoomCheck = 5000;
        m_uiPhase = 1;
        m_uiCurrentWave = 0;
        m_uiTotalSaved = 0;
        m_uiTotalKilled = 0;
        m_uiFootsoldiersSpawnCount = 0;
       
        for(uint8 i = 0; i < 5; i++)
        {
            m_uiKillCounter[i] = 0;
            m_uiSaveCounter[i] = 0;
            m_uiPeasantCount[i] = 10+i;
        }

        m_playerGuid.Clear();
        m_lCleaner.clear();
        m_lToCleanPlayers.clear();
        
        //Invisiblility
        m_creature->CastSpell(m_creature, QUEST_INVISIBILITY, true);

        //Questgiver Flags
        if (!m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

        //Despawn if something is/was spawned
        if (!m_lSummonedGUIDList.empty())
            for (GUIDList::const_iterator itr = m_lSummonedGUIDList.begin(); itr != m_lSummonedGUIDList.end(); ++itr)
                if (Creature* pSummoned = m_creature->GetMap()->GetCreature(*itr))
                    pSummoned->ForcedDespawn();
       
        //Cler Summons from list
        m_lSummonedGUIDList.clear();
    }

    //One Phase finished
    void PhaseEnded(bool bFailed, bool bWave)
    {
        //Get Player
        Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);

        //Continue only when we have starting player could cause problems with ALT-F4
        if (!pPlayer)
        {
            DoScriptText(urand(0, 1) ? -1000706 : -1000707, m_creature);
            if (pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW);
            //Despawn Eris
            m_creature->ForcedDespawn(0);
            //2 Hours respawn timer
            m_creature->SetRespawnTime(7200);
      
            m_bIsQuestInProgress = false;

            Reset();

            return;
        }

        // Failed
        if (bFailed && !bWave)
        {
            DoScriptText(urand(0, 1) ? -1000706 : -1000707, m_creature);
            if (pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
                pPlayer->FailQuest(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW);
            //Despawn Eris
            m_creature->ForcedDespawn(0);
            //2 Hours respawn timer
            m_creature->SetRespawnTime(7200);
      
            m_bIsQuestInProgress = false;

            Reset();

            return;
        }
        // Wave completed
        if (!bFailed && bWave)
        {
            DoScriptText(-1000709, m_creature);

            //Blessing
            m_creature->CastSpell(pPlayer, SPELL_BLESSING_OF_NORDRASSIL, false);

            //Summon Soldiers (Wave 1 -> 8, Wave 2 -> 6, Wave 3 -> 8, ...)
            m_uiFootsoldiersSpawnCount += (6 + ((m_uiCurrentWave%2)*2));

            //Footsoldiers are Spawned continously after the first Wave
            m_bFootsoldiersSpawned = true;

            //Wave Done next inc.
            m_uiCurrentWave++;

            return;
        }

        // Whole event completed
        if (!bFailed && !bWave)
        {
            DoScriptText(-1000708, m_creature);
            if (pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
            {
                /*Dont use that: 
                    AreaExploredOrEventHappens(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW);
                it spwans millions of Peasants*/

                //Set Flags
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

                //Quest done
                pPlayer->CompleteQuest(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW); 
            }

            m_bIsQuestInProgress = false;

            Reset();

            return;
        }
    }

    //Spawn next peasant wave
    void DoNextWave(uint32 uiWaveNumber)
    {
        //Random Peasant for Sayscript
        uint8 uiRandomPeasant = urand(0, m_uiPeasantCount[uiWaveNumber-1]);

        for(uint8 i = 0; i < m_uiPeasantCount[uiWaveNumber-1]; ++i)
        {
            //Injured or plagued Peasant
            uint32 m_uiPeasantType = NPC_INJURED_PEASANT;

            //Plagued Peasants have a 5% + Nr of Wave % Chance (10%, 15%, ...)
            if(urand(0,100) <= 5 + 5 * uiWaveNumber)
                m_uiPeasantType = NPC_PLAGUED_PEASANT;

            //Spawn peasant
            if (Creature* pTemp = m_creature->SummonCreature(m_uiPeasantType, aPeasantSpawn[i][0], aPeasantSpawn[i][1], aPeasantSpawn[i][2], 0, TEMPSUMMON_DEAD_DESPAWN, 0))
            {    
                //Push peasant into summoned creatures list
                m_lSummonedGUIDList.push_back(pTemp->GetGUID());

                //Waypoint, to avoid peasants going up the hills
                float fX, fY, fZ;
                pTemp->GetRandomPoint(3343.270f, -3018.100f, 161.72f, 5.0f, fX, fY, fZ);
                pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);

                //Sayscript
                if (i == uiRandomPeasant)
                    DoScriptText(aPeasantSpawnYell[urand(0,2)], pTemp);
            }
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 /*uiMotionType*/, uint32 uiPointId)
    {
        //First Waypoint reached
        if(uiPointId == 0)
        {
            //Set Waypoint 2
            float fX, fY,fZ;
            pSummoned->GetRandomPoint(3332.767f, -2979.002f, 160.97f, 5.0f, fX, fY, fZ);
            pSummoned->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
        }
        //Second Waypoint reached
        if (uiPointId == 1)
        {
            //Not started yet
            if(m_uiCurrentWave == 0)
                return;

            // When saved peasants exceed maximum peasants, something went wrong 
            if (m_uiSaveCounter[m_uiCurrentWave-1] >= m_uiPeasantCount[m_uiCurrentWave-1])
                debug_log("SD0: npc_eris_havenfire: Current wave %u was not reset properly in void WaveFinished().", m_uiCurrentWave);

            ++m_uiSaveCounter[m_uiCurrentWave-1];
            ++m_uiTotalSaved;

            // When counted, force despawn. I don't know exactly when they should disappear
            pSummoned->GetMotionMaster()->Clear(false);

            //Remove Auras
            if (pSummoned->HasAura(SPELL_DEATHS_DOOR, EFFECT_INDEX_0))
                pSummoned->RemoveAurasDueToSpell(SPELL_DEATHS_DOOR);
            if (pSummoned->HasAura(SPELL_SEETHING_PLAGUE, EFFECT_INDEX_0))
                pSummoned->RemoveAurasDueToSpell(SPELL_SEETHING_PLAGUE);

            //Random Sayscript
            uint8 uiRandomPeasant = urand(1,10);
            if (uiRandomPeasant == 5)
                DoScriptText(aPeasantSaveSay[urand(0,3)], pSummoned);

            //Cast spell
            pSummoned->CastSpell(pSummoned, SPELL_ENTER_THE_LIGHT_DND, false);

            //Despawn
            pSummoned->ForcedDespawn(4000);
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        /* We do not want to count staying peasants.
        It means that they are saved. Saved peasants are ForcedDespawn(),
        which triggers SummonedCreatureJustDied.
        */

        //Not started yet
        if(m_uiCurrentWave == 0)
            return;

        //Moving peasant
        if (pSummoned->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE && (pSummoned->GetEntry() == NPC_INJURED_PEASANT || pSummoned->GetEntry() == NPC_PLAGUED_PEASANT))
        {
            ++m_uiKillCounter[m_uiCurrentWave-1];
            ++m_uiTotalKilled;
        }
        pSummoned->RemoveCorpse();
    }

    //Summon footsoldier
    void DoSummonFootsoldier()
    {
        //Random spawnpoint
        uint8 uiRandomForArray = urand(0,2);
        float fX, fY, fZ;
        m_creature->GetRandomPoint(aFootsoldieSpawn[uiRandomForArray][0], aFootsoldieSpawn[uiRandomForArray][1], aFootsoldieSpawn[uiRandomForArray][2], 5.0f, fX, fY, fZ);

        //Spawn soldier
        if (Creature* pTemp = m_creature->SummonCreature(NPC_SCOURGE_FOOTSOLDIER, fX, fY, fZ, aFootsoldieSpawn[uiRandomForArray][3], TEMPSUMMON_DEAD_DESPAWN, 5000))
            //Push soldiers into summoned creatures list
            m_lSummonedGUIDList.push_back(pTemp->GetGUID());
    }

    //Spawn Archers
    void DoSpawnArchers ()
    {
        //10 Archers
        for(uint8 i = 0; i < 10; ++i)
            //Spawn
            if (Creature* pTemp = m_creature->SummonCreature(NPC_SCOURGE_ARCHER, aArcherSpawn[i][0], aArcherSpawn[i][1], aArcherSpawn[i][2], aArcherSpawn[i][3], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 360000))
                //Push archers into summoned creatures list
                m_lSummonedGUIDList.push_back(pTemp->GetGUID());
    }

    //Function of Doom is watching YOU
    void functionOfDoom(Creature* creature)
    {
        //If the creature cant have a threadlist this wont work
        if (!creature->CanHaveThreatList())
            return;
       
        //Declaration
        GUIDVector vGuids;

        //Fill the list with GUIDs
        creature->FillGuidsListFromThreatList(vGuids);

        if (!vGuids.empty())
            for (GUIDVector::const_iterator itr = vGuids.begin(); itr != vGuids.end(); ++itr)
                if (Unit* pTarget = creature->GetMap()->GetUnit(*itr))
                    //Entry is a player
                    if(pTarget->GetTypeId() == TYPEID_PLAYER)
                        //If its not the right one sharpen your Damocles Sword
                        if(((Player*)pTarget)->GetGUID() != m_playerGuid)
                        {
                            //Declarations
                            bool bInCleanerList = false, bDoneAlready = false;

                            //Player is dead, a shame. Get the player out of the list
                            if(((Player*)pTarget)->isDead())
                            {
                                if(!m_lToCleanPlayers.empty())
                                    for(std::list<Player*>::iterator i = m_lToCleanPlayers.begin(); i != m_lToCleanPlayers.end(); ++i)
                                        if((*i)->GetGUID() == ((Player*)pTarget)->GetGUID())
                                            bDoneAlready = true;
                                if(!bDoneAlready)
                                    m_lToCleanPlayers.remove((Player*)pTarget);
                            }
                            //Player is alive, put him on the cleaner list.
                            else
                            {
                                if(!m_lToCleanPlayers.empty())
                                    for(std::list<Player*>::iterator i = m_lToCleanPlayers.begin(); i != m_lToCleanPlayers.end(); ++i)
                                        if((*i)->GetGUID() == ((Player*)pTarget)->GetGUID())
                                            bInCleanerList = true;
                                if(!bInCleanerList)
                                    m_lToCleanPlayers.push_back((Player*)pTarget);
                            }
                        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Nothing is needed to update when the event is not in progress
        if (!m_bIsQuestInProgress || m_uiCurrentWave == 0)
            return;

        //Get Player
        Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid);

        //Get some cleaner-food
        for (GUIDList::const_iterator itr = m_lSummonedGUIDList.begin(); itr != m_lSummonedGUIDList.end(); ++itr)
            if (Creature* pSummoned = m_creature->GetMap()->GetCreature(*itr))
                functionOfDoom(pSummoned);

        //Is our bad boy out already?
        if(m_bCleaningInProgress)
        {
            //Where is our boy
            m_lCleaner.clear();
            GetCreatureListWithEntryInGrid(m_lCleaner, m_creature, NPC_CLEANER, 300.f);
            //Hes not there
            if(m_lCleaner.empty())
                m_bCleaningInProgress = false;
        }

        //Get the frakkin Cleaner started
        if(!m_bCleaningInProgress)
            if(!m_lToCleanPlayers.empty())
                for(std::list<Player*>::iterator i = m_lToCleanPlayers.begin(); i != m_lToCleanPlayers.end(); ++i)
                    if((*i)->isAlive() && (*i)->isTargetableForAttack())
                    {
                        float fX, fY, fZ;
                        m_creature->GetRandomPoint( (*i)->GetPositionX(), (*i)->GetPositionY(), (*i)->GetPositionZ(), 5.0f, fX, fY, fZ );
                        Creature* pCleaner = m_creature->SummonCreature(NPC_CLEANER, fX, fY, fZ, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);		 
                        pCleaner->AI()->AttackStart((*i));
                        m_bCleaningInProgress = true;
                    }

        //Different Footsoldier spawns
        if(m_bFootsoldiersSpawned)
        {
            //Spawntimers
            if (m_uiFootsoldierTimer1 <= uiDiff)
            {
                m_uiFootsoldiersSpawnCount += 1;
                m_uiFootsoldierTimer1 = urand(0,1)*5000 + 20000;
            }
            else
                m_uiFootsoldierTimer1 -= uiDiff;

            if (m_uiFootsoldierTimer2 <= uiDiff)
            {
                m_uiFootsoldiersSpawnCount += 2;
                m_uiFootsoldierTimer2 = 30000;
            }
            else
                m_uiFootsoldierTimer2 -= uiDiff;

            if (m_uiFootsoldierTimer3 <= uiDiff)
            {
                m_uiFootsoldiersSpawnCount += 3+((m_uiCurrentWave%2)*2);
                m_uiFootsoldierTimer3 = 60000;
            }
            else
                m_uiFootsoldierTimer3 -= uiDiff;

            //Function call depending on count
            if(m_uiFootsoldiersSpawnCount > 0)
            {
                if(m_uiFootsoldierSpawnTimer <= uiDiff)
                {
                    DoSummonFootsoldier();
                    m_uiFootsoldiersSpawnCount--;
                    m_uiFootsoldierSpawnTimer = 500;
                }
                else
                    m_uiFootsoldierSpawnTimer -= uiDiff;
            }
        }

        //Our player is dead
        if (pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) != QUEST_STATUS_INCOMPLETE || pPlayer->isDead())
        {
            PhaseEnded(true, false);
            return;
        }

        //Phases
        if (m_uiPhase)
        {
            // Reaching 15 dead peasants means fail for us
            if (m_uiTotalKilled >= 15)
            {
                PhaseEnded(true, false);
                return;
            }

            // Do next step
            else if ((m_uiKillCounter[m_uiCurrentWave-1] + m_uiSaveCounter[m_uiCurrentWave-1] >= m_uiPeasantCount[m_uiCurrentWave-1]))
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
            if (m_uiPhase > 6 || m_uiCurrentWave == 0)
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
                        DoNextWave(1);
                        m_uiMainTimer = 38000;
                        break;
                    case 3: // Wave 2
                        DoNextWave(2);
                        m_uiMainTimer = 38000;
                        break;
                    case 4: // Wave 3
                        DoNextWave(3);
                        m_uiMainTimer = 38000;
                        break;
                    case 5: // Wave 4
                        DoNextWave(4);
                        m_uiMainTimer = 38000;
                        break;
                    case 6: // Wave 5
                        DoNextWave(5);
                        break;
                }
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
        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        Reset();
    }

    std::list<Creature*> pPeasants;
    uint32 m_uiShotTimer;

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
        //Invisibility
        //if (!m_creature->HasAura(QUEST_INVISIBILITY))
        //    m_creature->CastSpell(m_creature, QUEST_INVISIBILITY, false);

        if (Player* pPlayer = GetPlayerAtMinimumRange(300.0f))
        {
            if(pPlayer->isAlive() && !pPlayer->isInCombat() && pPlayer->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
                pPlayer->SetInCombatWith(m_creature);
        }

        //Peasant list
        pPeasants.clear();
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_INJURED_PEASANT, 60.0f);
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_PLAGUED_PEASANT, 60.0f);

        if (m_uiShotTimer)
        {
            if(m_uiShotTimer <= uiDiff)
            {
                //Get random Peasants for every shot
                uint32 pPeasantRandom = urand(0,pPeasants.size()), j = 1;
                for(std::list<Creature*>::iterator i = pPeasants.begin(); i != pPeasants.end(); ++i)
                {
                    if((*i)->isAlive() && j == pPeasantRandom && m_creature->IsInRange((*i), 0.0f, 59.0f, true))
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

    void Reset()
    {
        //Values need to be set
        m_creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 108);
        m_creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 126);
        m_creature->UpdateDamagePhysical(BASE_ATTACK);
        m_creature->SetArmor(3435);
        m_creature->CastSpell(m_creature, SEE_PRIEST_INVIS, false);
    }

    void UpdateAI(const uint32 uiDiff)
    {   
        //Peasant list
        std::list<Creature*> pPeasants;
        pPeasants.clear();
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_INJURED_PEASANT, 300.0f);
        GetCreatureListWithEntryInGrid(pPeasants, m_creature, NPC_PLAGUED_PEASANT, 300.0f);
        
        if(Player* pPlayer = GetPlayerAtMinimumRange(300.0f))
            m_creature->AddThreat(pPlayer);

	    if (!pPeasants.empty())
	        for(std::list<Creature*>::iterator i = pPeasants.begin(); i != pPeasants.end(); ++i)
                if((*i)->isAlive())
                    m_creature->AddThreat((*i));
	    
        DoMeleeAttackIfReady();       
    }
};

CreatureAI* GetAI_mob_scourge_footsoldier(Creature* pCreature)
{
    return new mob_scourge_footsoldierAI(pCreature);
}
void AddSC_Priest_Epic_Quest()
{
    Script* pNewscript;

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
