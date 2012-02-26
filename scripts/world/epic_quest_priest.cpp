#include "precompiled.h"
#include "escort_ai.h"
/*author: zero
 *script complete: 100% 
 *need test
 */

enum
{
    /* Spawn IDs */
    PEASANT = 14485,
    SOLDIER = 14486,
    ARCHER = 14489,
    CLEANER = 14503,

    /* Spells */
    SEETHINGPLAGUE = 23072,
    SHOOT = 23073,
    ENTERTHELIGHT = 23107,
    NORDRASSIL = 23108,
    DEATHSDOOR = 23127,
    INVISIBILITY = 23196,
    IMMUNITY = 29230,

    /* Generic */
    QUESTID = 7622,

    NUM_PEASANTS = 12,
    NUM_ARCHERS = 10,
};

const float archerPosis[NUM_ARCHERS][4] =
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

const float peasantPosis[NUM_PEASANTS][3] =
{
    { 3364.0f, -3054.0f, 165.5f },
    { 3363.0f, -3051.4f, 165.5f },
    { 3362.0f, -3052.5f, 165.5f },
    { 3361.0f, -3054.3f, 165.5f },
    { 3366.0f, -3052.0f, 165.5f },
    { 3365.0f, -3051.0f, 165.5f },
    { 3367.0f, -3054.0f, 165.5f },
    { 3368.0f, -3053.0f, 165.5f },
    { 3369.0f, -3051.6f, 165.5f },
    { 3370.4f, -3052.0f, 165.5f },
    { 3371.0f, -3054.0f, 165.5f },
    { 3360.0f, -3053.9f, 165.5f }
};

const float soldierPosis[3][4] =
{
    { 3349.937f, -3056.875f, 168.141f, 1.622f },
    { 3370.527f, -3048.276f, 165.872f, 2.377f },
    { 3346.987f, -3052.782f, 165.360f, 1.662f }
};

std::vector<Creature*> archers;
std::vector<Creature*> peasants;
std::vector<Creature*> soldiers;
std::vector<Unit*> gotCleaner;

struct Eris;
Eris* g_pEris;

struct Eris : public ScriptedAI
{
    bool running;
    uint32 wave;
    uint32 maxPlagued;
    uint32 saved;
    uint32 died;
    uint32 waveTimer;
    uint32 soldierTimer;
    Player* plr;

    Eris(Creature* creature) : ScriptedAI(creature), running(false), wave(1), maxPlagued(1), saved(0), died(0), waveTimer(40000), soldierTimer(0), plr(0)
    {
        //m_creature->CastSpell(m_creature, ENTERTHELIGHT, true);
        m_creature->CastSpell(m_creature, INVISIBILITY, true);
        m_creature->setFaction(35);
    }

    void JustRespawned() { m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER); }

    void Reset() { DespawnArchers(); }


    void UpdateAI(const uint32 time)
    {
        waveTimer += time;
        soldierTimer += time;
        //quest fails
        if(died >= 15 || (plr && plr->isDead()))
        {
            running = false;
            DespawnArchers();
            if (plr)
                plr->FailQuest(QUESTID);
            DespawnEris();
        }
        //quest complete
        if(saved >= 50)
        {
            running = false;
            DespawnArchers();
            if (plr && plr->HasItemCount(18665,1))
                plr->CompleteQuest(QUESTID);
        }
        if(running && waveTimer >= 35000)
        {
            waveTimer = 0;
            SpawnWave();
        }
        /*
        The footsoldiers spawn after each Blessing of Nordrassil and again after different times after death. The spawn of footsoldiers increases with each blessing.
        X: +25 secounds 2 footsoldiers (will respawn after 20 or 25 secounds alternately)
        X: +45 (25+20) secounds 2 footsoldiers 
        X: +70 (45+25) secounds 2 footsoldiers
        X: +90 (70+20) secounds 2 footsoldiers
        X: ... until quest is finished. (previous amount plus 20 or 25 secounds)

        X: +30 secounds 1 footsoldier spawns (will always spawn each 30 secounds, same like above and until the quest is done)

        X: +60 secounds 3 footsoldiers spawn (will always spawn each 60 secounds, same like above and until the quest is done)
        So basically after each wave the numbers increase by 1.
        */
        if(wave > 2 && soldierTimer >= 15000)
        {
            soldierTimer = 0;
            SpawnSoldiers();
        }
    }

    void SpawnSoldiers()
    {
        uint32 count = 3 + urand(0, 4);

        for(uint32 i = 0, n = 0; i < count; ++i)
        {
            Creature* cr = m_creature->SummonCreature(SOLDIER, soldierPosis[n][0], soldierPosis[n][1], soldierPosis[n][2], soldierPosis[n][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
            if(cr)
            {
                soldiers.push_back(cr);
                if(++n > 2)
                    n = 0;
            }
        }
    }

    void DespawnSoldiers()
    {
        for(std::vector<Creature*>::const_iterator it = soldiers.begin(); it != soldiers.end(); ++it)
        {
            (*it)->ForcedDespawn();
            (*it)->AddObjectToRemoveList();
        }
        soldiers.clear();
    }

    void SpawnWave()
    {
        uint32 plagued = 0;
        for(int i = 0; i < NUM_PEASANTS; ++i)
        {
            Creature* peas = m_creature->SummonCreature(PEASANT, peasantPosis[i][0], peasantPosis[i][1], peasantPosis[i][2], 1.505f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
            if(peas)
            {
                peasants.push_back(peas);
                uint32 rnd = urand(0, 3);
                if(rnd == 1)
                    peas->CastSpell(peas, DEATHSDOOR, false);
                else
                {
                    if(plagued++ < maxPlagued)
                        peas->CastSpell(peas, SEETHINGPLAGUE, false);
                }
            }
        }

        if(++wave % 2 == 1)
            maxPlagued++;
    }

    void SpawnArchers()
    {
        //event starts here after quest was accepted, therefore u will be unable to start the quest again while the event is in progress
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);

        if(running)
            return;

        running = true;
        for(int i = 0; i < NUM_ARCHERS; ++i)
        {
            Creature* creature = m_creature->SummonCreature(ARCHER, archerPosis[i][0], archerPosis[i][1], archerPosis[i][2], archerPosis[i][3], TEMPSUMMON_MANUAL_DESPAWN, DAY*IN_MILLISECONDS);
            if(creature)
                archers.push_back(creature);			
        }
    }

    void DespawnArchers()
    {
        running = false;
        wave = maxPlagued = 1;
        saved = died = 0;

        for(std::vector<Creature*>::const_iterator it = archers.begin(); it != archers.end(); ++it)
        {
            (*it)->ForcedDespawn();
            (*it)->AddObjectToRemoveList();
        }
        archers.clear();

        for(std::vector<Creature*>::const_iterator it = peasants.begin(); it != peasants.end(); ++it)
        {
            (*it)->ForcedDespawn();
            (*it)->AddObjectToRemoveList();
        }
        peasants.clear();

        DespawnSoldiers();

        gotCleaner.clear();
    }

    void DespawnEris()
    {
        m_creature->ForcedDespawn(0);
        //2hours until respawn (20minutes after nerf)
        m_creature->SetRespawnTime(7200);
    }
        
    void Say(std::stringstream& msg)
    {
        m_creature->MonsterSay(msg.str().c_str(), 0);
    }

    void Say(std::string& msg)
    {
        m_creature->MonsterSay(msg.c_str(), 0);
    }

    void BuffPlr()
    {
        if(running && plr)
            plr->CastSpell(plr, NORDRASSIL, true);
    }

    void Saved()
    {
        std::stringstream ss;
        ss << ++saved;
        ss << " peasants saved!";
        Say(ss);
    }

    void Died()
    {
        std::stringstream ss;
        ss << ++died;
        ss << " peasants died!";
        Say(ss);
    }

    static bool QuestAccepted(Player* plr, Creature* cr, const Quest* quest)
    {
        if(quest->GetQuestId() == QUESTID)
        {
            if(g_pEris && !g_pEris->running)
            {
                g_pEris->plr = plr;
                g_pEris->SpawnArchers();
            }

        }
        return true;
    }

    static CreatureAI* GetAI(Creature* creature)
    {
        if(g_pEris == NULL)
        {
            g_pEris = new Eris(creature);
            return g_pEris;
        }
        else
            return g_pEris;
    }
};

struct Peasant : public npc_escortAI
{
    uint32 endPos;
    bool firstTick;
    bool despawned;
    Player* eventPlayer;

    Peasant(Creature* creature) : npc_escortAI(creature), firstTick(true), despawned(false), eventPlayer(0)
    {
        endPos = urand(0, 2);

        //player are able to heal them
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);

        //slow
        m_creature->SetSpeedRate(MOVE_WALK,0.5);

        //create random peasant stats
        uint8 randomLevel = urand(0,1);
        if (randomLevel == 0)
        {
            m_creature->SetLevel(51);
            m_creature->SetMaxHealth(1700);
        }
        else {
            m_creature->SetLevel(52);
            m_creature->SetMaxHealth(1800);
        }

        //fight will remove invisibilty
        //m_creature->CastSpell(creature, INVISIBILITY, true);

        //get event player
        eventPlayer = g_pEris->plr;

        Start();
    }

    //check if other player have healed the taregt
    void HealBy(Unit* pHealer, uint32 uiAmountHealed)
    {
        if (eventPlayer)
         {
             if (pHealer != eventPlayer)
             {
                 uiAmountHealed = 0;
                 bool gotCleaned = false;
                 for(std::vector<Unit*>::const_iterator it = gotCleaner.begin(); it != gotCleaner.end(); ++it)
                 {
                     if ((*it) == pHealer)
                         gotCleaned = true;
                 }
                 if (!gotCleaned && pHealer->GetTypeId() == TYPEID_PLAYER)
                 {
                     gotCleaner.push_back(pHealer);
                     Creature* pCleaner = m_creature->SummonCreature(CLEANER, pHealer->GetPositionX(), pHealer->GetPositionY(), pHealer->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);
                     pCleaner->AI()->AttackStart(pHealer);
                 }
             }
         }
    }

    //override, so the mob wont run to the attacker
    void AttackStart(Unit* ) {}

    void WaypointReached(uint32 uiPointId)
    {
        despawned = true;
        if(g_pEris)
            g_pEris->Saved();

        Despawn();
    }

    void Despawn()
    {	
        Erase();
        m_creature->ForcedDespawn();
        m_creature->AddObjectToRemoveList();

        if(peasants.empty())
        {
            g_pEris->BuffPlr();
            //footsoldiers spawn after each Blessing of Nordrassil and again after different times after death
            g_pEris->SpawnSoldiers();
        }
    }

    void Erase()
    {
        std::vector<Creature*>::iterator it = std::find(peasants.begin(), peasants.end(), m_creature);
        peasants.erase(it);
    }

    void JustDied(Unit*)
    {
        if(!despawned && g_pEris)
            g_pEris->Died();
        Erase();
    }

    void Reset() {}

    static CreatureAI* GetAI(Creature* creature)
    {
        return new Peasant(creature);
    }
};

struct Archer : public ScriptedAI
{
    uint32 bowShotTime;
    uint32 bowShotTimer;
    Player* eventPlayer;

    Archer(Creature* creature) : ScriptedAI(creature), bowShotTimer(0), eventPlayer(0)
    {
        //fight will remove invisibilty
        //m_creature->CastSpell(creature, INVISIBILITY, true);

        //right stats for archers
        m_creature->SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, 162);
        m_creature->SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, 186);
        m_creature->UpdateDamagePhysical(RANGED_ATTACK);
        m_creature->SetArmor(3791);
        
        //get a bow
        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_UNEQUIP, 6231);

        //get immune
        m_creature->CastSpell(creature, IMMUNITY, true);

        //dont move
        SetCombatMovement(false);

        //shoot timer
        bowShotTime = 2000 + urand(0, 400); 

        //get eventPlayer
        eventPlayer = g_pEris->plr;
    }

    void Reset() {}

    void UpdateAI(const uint32 time)
    {
        bowShotTimer += time;
        if(bowShotTimer >= bowShotTime)
        {
            bowShotTimer = 0;
            size_t rndTarget = urand(0, peasants.size()-1);
            if(rndTarget >= peasants.size())
            {
                if (eventPlayer && eventPlayer->isAlive())
                    DoCastSpellIfCan(eventPlayer, SHOOT);
                else 
                    return;
            }
            else {
                Creature* target = peasants[rndTarget];
                DoCastSpellIfCan(target, SHOOT);
            }
        }
    }

    static CreatureAI* GetAI(Creature* creature)
    {
        return new Archer(creature);
    }
};

struct Soldier : public ScriptedAI
{
    Creature* target;
    Player* eventPlayer;

    Soldier(Creature* creature) : ScriptedAI(creature), target(0), eventPlayer(0) 
    {
        //fight will remove invisibilty
        //m_creature->CastSpell(creature, INVISIBILITY, true);

        //right stats for soldiers
        m_creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 108);
        m_creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 126);
        m_creature->UpdateDamagePhysical(BASE_ATTACK);
        m_creature->SetArmor(3435);

        //get event player
        eventPlayer = g_pEris->plr;
    }

     //check if other player have joined the fight
     void DamageTaken(Unit* pDoneBy, uint32& uiDamage) 
     {
         if (eventPlayer)
         {
             if (pDoneBy != eventPlayer)
             {
                 uiDamage = 0;
                 bool gotCleaned = false;
                 for(std::vector<Unit*>::const_iterator it = gotCleaner.begin(); it != gotCleaner.end(); ++it)
                 {
                     if ((*it) == pDoneBy)
                         gotCleaned = true;
                 }
                 if (!gotCleaned && pDoneBy->GetTypeId() == TYPEID_PLAYER)
                 {
                     gotCleaner.push_back(pDoneBy);
                     Creature* pCleaner = m_creature->SummonCreature(CLEANER, pDoneBy->GetPositionX(), pDoneBy->GetPositionY(), pDoneBy->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000);		 
                     pCleaner->AI()->AttackStart(pDoneBy);
                 }
             }
         }
     }

    void UpdateAI(const uint32 time)
    {
        if(!m_creature->isInCombat() || target == 0)
        {
            //get in combat with all peasants
            for(std::vector<Creature*>::const_iterator it = peasants.begin(); it != peasants.end(); ++it)
                AttackStart((*it));
            //find a random peasant to attack
            uint32 rnd = urand(0, peasants.size()-1);
            if(rnd >= peasants.size())
                return;
            target = peasants[rnd];
            AttackStart(target);
        }

        ScriptedAI::UpdateAI(time);
    }

    void JustDied(Unit*)
    {
        std::vector<Creature*>::iterator i = std::find(soldiers.begin(), soldiers.end(), m_creature);
        soldiers.erase(i);
        m_creature->SetDeathState(JUST_DIED);
        m_creature->SetHealthPercent(0);
    }

    void Reset() 
    { 
        if (eventPlayer && eventPlayer->isAlive())
            AttackStart(eventPlayer);
        else
            JustDied(0); 
    }

    static CreatureAI* GetAI(Creature* creature)
    {
        return new Soldier(creature);
    }
};

void AddSC_Priest_Epic_Quest()
{
    /* Eris */
    Script* pScript = new Script;
    pScript->Name = "PriestQuestEpic";
    pScript->pQuestAcceptNPC = &Eris::QuestAccepted;
    pScript->GetAI = &Eris::GetAI;
    pScript->RegisterSelf();

    /* Peasant */
    pScript = new Script;
    pScript->Name = "PriestQuestPeas";
    pScript->GetAI = &Peasant::GetAI;
    pScript->RegisterSelf();

    /* Archer */
    pScript = new Script;
    pScript->Name = "PriestQuestArch";
    pScript->GetAI = &Archer::GetAI;
    pScript->RegisterSelf();

    /* Soldier */
    pScript = new Script;
    pScript->Name = "PriestQuestSold";
    pScript->GetAI = &Soldier::GetAI;
    pScript->RegisterSelf();
}