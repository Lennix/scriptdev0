#include "precompiled.h"
#include "escort_ai.h"

enum
{
	/* Spawn IDs */
	PEASANT = 14485,
	SOLDIER = 14486,
	ARCHER = 14489,

	/* Spells */
	SEETHINGPLAGUE = 23072,
	SHOOT = 23073,
	ENTERTHELIGHT = 23107,
	NORDRASSIL = 27527,
	DEATHSDOOR = 23127,
	INVISIBILITY = 23196,

	/* Generic */
	QUESTID = 7622,

	NUM_PEASANTS = 12,
	NUM_ARCHERS = 10,

	EVENT_RANGE = 50,

	EYE_OF_SHADOW = 18665,
};

const float archerPosis[NUM_ARCHERS][4] =
{
	{ 3380.255f, -3060.907f, 182.504f, 2.359f },
	{ 3382.333f, -3057.745f, 181.919f, 2.371f },
	{ 3371.479f, -3070.499f, 175.159f, 2.025f },
	{ 3358.776f, -3074.729f, 174.090f, 1.350f },
	{ 3347.300f, -3070.288f, 177.841f, 1.279f },
	{ 3366.956f, -3023.204f, 171.213f, 3.382f },
	{ 3334.764f, -3052.669f, 175.158f, 1.357f },
	{ 3317.438f, -3037.754f, 165.531f, 0.265f },
	{ 3325.897f, -3022.678f, 170.103f, 6.144f },
	{ 3362.131f, -3010.514f, 183.945f, 3.602f }
};

const float peasantPosis[NUM_PEASANTS][3] =
{
	{ 3364.0f, -3054.0f, 165.5f },
	{ 3363.0f, -3054.4f, 165.5f },
	{ 3362.0f, -3054.5f, 165.5f },
	{ 3361.0f, -3053.3f, 165.5f },
	{ 3360.0f, -3055.0f, 165.5f },
	{ 3365.0f, -3052.0f, 165.5f },
	{ 3364.0f, -3054.0f, 165.5f },
	{ 3363.0f, -3053.0f, 165.5f },
	{ 3362.0f, -3053.6f, 165.5f },
	{ 3361.4f, -3052.0f, 165.5f },
	{ 3361.0f, -3054.0f, 165.5f },
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

struct Eris;
Eris* g_pEris;

struct Eris : public ScriptedAI
{
	bool abuse;
	bool running;
	uint32 wave;
	uint32 maxPlagued;
	uint32 saved;
	uint32 died;
	uint32 waveTimer;
	uint32 soldierTimer;
	Player* plr;

	Eris(Creature* creature) : ScriptedAI(creature), abuse(false), running(false), wave(1), maxPlagued(1), saved(0), died(0), waveTimer(40000), soldierTimer(0), plr(0)
	{
		//m_creature->CastSpell(m_creature, ENTERTHELIGHT, true);
		m_creature->CastSpell(m_creature, INVISIBILITY, true);
		m_creature->setFaction(35);
	}

	void Reset() { DespawnArchers(); }

	void UpdateAI(const uint32 time)
	{
		waveTimer += time;
		soldierTimer += time;
		//quest fails
		if(abuse || died >= 15 || (plr && plr->isDead()))
		{
			running = false;
			DespawnArchers();
			if (abuse)
			{
                m_creature->MonsterSay("Another player joined the EpicQuest area!", LANG_UNIVERSAL);
				abuse = false;
			}
			if (plr)
				plr->FailQuest(QUESTID);
		}
		//quest complete
		if(saved >= 50)
		{
			running = false;
			DespawnArchers();
			if (plr)
				plr->CompleteQuest(QUESTID);
		}
		if(running && waveTimer >= 38000)
		{
			waveTimer = 0;
			SpawnWave();
		}
		if(wave > 2 && soldierTimer >= 15000)
		{
			soldierTimer = 0;
			abuse = SpawnSoldiers();
		}
	}

	bool SpawnSoldiers()
	{
		bool abuse = false;
		uint32 count = 3 + urand(0, 4);

		for(uint32 i = 0, n = 0; i < count; ++i)
		{
			Creature* cr = m_creature->SummonCreature(SOLDIER, soldierPosis[n][0], soldierPosis[n][1], soldierPosis[n][2], soldierPosis[n][3], TEMPSUMMON_MANUAL_DESPAWN, DAY*IN_MILLISECONDS);
			if(cr)
			{
				soldiers.push_back(cr);
				abuse = isEventAbused(cr);
				if(++n > 2)
					n = 0;
			}
		}
		return abuse;
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
			Creature* peas = m_creature->SummonCreature(PEASANT, peasantPosis[i][0], peasantPosis[i][1], peasantPosis[i][2], 1.505f, TEMPSUMMON_CORPSE_DESPAWN, DAY*IN_MILLISECONDS);
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

	bool isEventAbused(Creature *cCheck)
	{
		bool fail = false;
		Map::PlayerList const &PlayerList = m_creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
			Player* m_pPlayer = itr->getSource();
			if (m_pPlayer != plr && !fail)
			{
				if (m_creature->IsWithinDistInMap(m_pPlayer,EVENT_RANGE))
					fail = true;
				else if (plr->IsWithinDistInMap(m_pPlayer,EVENT_RANGE))
					fail = true;
				else if (cCheck->IsWithinDistInMap(m_pPlayer,EVENT_RANGE))
					fail = true;
			}
		}
		return fail;
	}

	void SpawnArchers()
	{
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
			DoCastSpellIfCan(m_creature, NORDRASSIL);
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
			if (plr->HasItemCount(EYE_OF_SHADOW ,1))
			{
				if(g_pEris && !g_pEris->running)
				{
					g_pEris->plr = plr;
					g_pEris->SpawnArchers();
				}
			}
			else
				plr->FailQuest(QUESTID);
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

	Peasant(Creature* creature) : npc_escortAI(creature), firstTick(true), despawned(false)
	{
		endPos = urand(0, 2);
		Start();
	}

	//override, so the mob wont run to the attacker
	void AttackStart(Unit* ) {}

	//override, so the mob wont attack the attacker
	void UpdateEscortAI(const uint32 time) { m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT|UNIT_FLAG_PVP); }

	void WaypointReached(uint32 uiPointId)
	{
		despawned = true;
		if(g_pEris)
			g_pEris->Saved();

		Despawn();
	}

	void Despawn()
	{
		std::vector<Creature*>::iterator it = std::find(peasants.begin(), peasants.end(), m_creature);
		peasants.erase(it);
		m_creature->ForcedDespawn();
		m_creature->AddObjectToRemoveList();

		if(peasants.empty())
			g_pEris->BuffPlr();
	}

	void JustDied(Unit*)
	{
		if(!despawned && g_pEris)
			g_pEris->Died();
		Despawn();
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

	Archer(Creature* creature) : ScriptedAI(creature), bowShotTimer(0)
	{
		bowShotTime = 2000 + urand(0, 400); 
	}

	//override, so the mob wont run to the attacker
	void AttackStart(Unit*) {}
	void Reset() {}

	void UpdateAI(const uint32 time)
	{
		bowShotTimer += time;
		if(bowShotTimer >= bowShotTime)
		{
			bowShotTimer = 0;
			size_t rndTarget = urand(0, peasants.size()-1);
			if(rndTarget >= peasants.size())
				return;

			Creature* target = peasants[rndTarget];
			DoCastSpellIfCan(target, SHOOT);
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
	Soldier(Creature* creature) : ScriptedAI(creature), target(0) {}

	void UpdateAI(const uint32 time)
	{
		if(!m_creature->isInCombat() || target == 0)
		{
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
		m_creature->ForcedDespawn();
		m_creature->AddObjectToRemoveList();
	}

	void Reset() { JustDied(0); }

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