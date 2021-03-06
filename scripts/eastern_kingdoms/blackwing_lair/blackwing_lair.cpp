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
SDName: Blackwing_Lair
SD%Complete: 10
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

/* ContentData
mob_demon_portal
EndContentData */

#include "precompiled.h"
#include "blackwing_lair.h"

/*######
## mob_demon_portal
######*/

enum eDemonPortal
{
    NPC_DEMONIC_MINION              = 12422,        // probably wrong creature entry
    SPELL_SUMMON_DEMONIC_MINION     = 19828,        // probably wrong spell entry
};

struct MANGOS_DLL_DECL mob_demon_portalAI : public ScriptedAI
{
    mob_demon_portalAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_creature->addUnitState(UNIT_STAT_ROOT);
        m_creature->addUnitState(UNIT_STAT_CAN_NOT_MOVE);
        m_uiDespawnSelfTimer = 11000;
        Reset();
    }

    uint32 m_uiSpawnDemonTimer;
    uint32 m_uiDespawnSelfTimer;

    void Reset()
    {
        m_uiSpawnDemonTimer = 11000;
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (Unit* pOwner = m_creature->GetMap()->GetUnit(m_creature->GetOwnerGuid()))
        {
            pSummoned->SetOwnerGuid(pOwner->GetObjectGuid());
            pSummoned->SetCharmerGuid(pOwner->GetObjectGuid());
            pSummoned->setFaction(pOwner->getFaction());
            pSummoned->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
            pSummoned->GetMotionMaster()->MoveFollow(pOwner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        }
        else
            pSummoned->setFaction(m_creature->getFaction());
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Close portal (despawn) after 35 seconds
        if (m_uiDespawnSelfTimer <= uiDiff)
        {
            m_uiDespawnSelfTimer = 0;
            m_creature->ForcedDespawn();
        }

        // Demon Spawn
        if (m_uiSpawnDemonTimer <= uiDiff)
        {
            /*if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_DEMONIC_MINION) == CAST_OK)
                m_uiSpawnDemonTimer = 11000;*/
            Position pos;
            m_creature->GetClosePoint(pos.x, pos.y, pos.z, 0);
            m_creature->SummonCreature(NPC_DEMONIC_MINION, pos.x, pos.y, pos.z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
            m_uiSpawnDemonTimer = 11000;
        }
        else
            m_uiSpawnDemonTimer -= uiDiff;

        // no meele, it is a portal..
    }
};

CreatureAI* GetAI_mob_demon_portal(Creature* pCreature)
{
    return new mob_demon_portalAI(pCreature);
}

/*######
## mob_suppression_trigger
## ToDo: whelp timer should be 30 seconds(hordeguides), dust animation ist missing
######*/

enum eSuppressionTrigger
{
    SPELL_SUPPRESSION_AURA      = 22247
};

struct MANGOS_DLL_DECL mob_suppression_triggerAI : public Scripted_NoMovementAI
{
    mob_suppression_triggerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_creature->setFaction(103);
        m_creature->SetVisibility(VISIBILITY_OFF);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
        pDevice = NULL;
        Reset();
    }

	GameObject* pDevice;
	uint32 SuppressionTimer;
	uint32 ActivationTimer;
    uint32 FogDurationHolder;
    uint32 FogCastTimer;

    bool startSupression;

	void Reset()
    {
        FogDurationHolder = 0;
        FogCastTimer = 0;
        SuppressionTimer = 1000;
        ActivationTimer = urand(15000, 30000/*DEVICE_RESPAWN*/);

        startSupression = false;
	}

	void UpdateAI(const uint32 uiDiff)
    {
        if (!pDevice)
            pDevice = GetClosestGameObjectWithEntry(m_creature, GO_SUPPRESSION_DEVICE, 5.0f);

        //deactivated - dont cast
        if (pDevice && pDevice->GetGoState() == GO_STATE_ACTIVE)
        {
            if (ActivationTimer < uiDiff)
            {
                ActivationTimer = urand(15000, 30000/*DEVICE_RESPAWN*/);
                pDevice->SetGoState(GO_STATE_READY);
                //instant cast possible if the trap is reactivated 
                SuppressionTimer = urand(1000, 8000);
            }
            else
                ActivationTimer -= uiDiff;

            return;
        }
        //active - cast
        else if (pDevice && pDevice->GetGoState() == GO_STATE_READY)
        {
            if (SuppressionTimer < uiDiff && !startSupression)
            {
                //fog animation
                pDevice->SendGameObjectCustomAnim(pDevice->GetGUID());              
                FogCastTimer = 500;
                startSupression = true;
            }
            else
                SuppressionTimer -= uiDiff;
        }

        //fog animation is visible for ~ 4 seconds, therefore we handle the supression aura over this time here
        if (startSupression)
        {
            if (FogDurationHolder > 3500)
            {
                startSupression = false;
                FogDurationHolder = 0;
                /*
                 *minimum delay is equal to the cast time of disarm trap ( 2 seconds)
                 *but we also have to calculate the time to move as close as possible to the trap,
                 *therefore we take a minimum delay of 4 seconds
                 */
                SuppressionTimer = urand(4000, 8000);
            }
            else
            {
                if (FogCastTimer <= uiDiff)
                {
                    DoCastSpellIfCan(m_creature, SPELL_SUPPRESSION_AURA);
                    FogCastTimer = 1000;
                    FogDurationHolder += 1000;
                }
                else
                    FogCastTimer -= uiDiff;
            }
        }
	}
};

CreatureAI* GetAI_mob_suppression_trigger(Creature* pCreature)
{
    return new mob_suppression_triggerAI(pCreature);
}

/*######
## mob_blackwing_taskmaster
## ToDo: need to implement a group pat function for same ids
######*/

enum blackwingTaskmaster
{
    SPELL_HEALING_CIRCLE = 22458,
    SPELL_SHADOW_SHOCK   = 17289,
};

struct MANGOS_DLL_DECL mob_blackwing_taskmasterAI : public ScriptedAI
{
    mob_blackwing_taskmasterAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 shadowShock_Timer;
    uint32 healingCircle_Timer;

    void Reset()
    {
        shadowShock_Timer = urand(1000, 7000);
        healingCircle_Timer = urand(8000, 18000);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        //we have to implement sync for every pat group but if no pat group is possible it doesnt matter
        if (healingCircle_Timer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_HEALING_CIRCLE) == CAST_OK)
                healingCircle_Timer = urand(8000, 18000);
        }
        else
            healingCircle_Timer -= uiDiff;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (shadowShock_Timer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_SHOCK) == CAST_OK)
                shadowShock_Timer = urand(1000, 7000);
        }
        else
            shadowShock_Timer -= uiDiff;
    }
};

CreatureAI* GetAI_mob_blackwing_taskmaster(Creature* pCreature)
{
    return new mob_blackwing_taskmasterAI(pCreature);
}

/// !!! RAZORGORE ADDS !!! \\\
//UPDATE `creature_template` SET `ScriptName` = 'blackwing_mage' WHERE `creature_template`.`entry` =12420 LIMIT 1 ;
//UPDATE `creature_template` SET `ScriptName` = 'blackwing_melee' WHERE `creature_template`.`entry` =12416 LIMIT 1 ;
//UPDATE `creature_template` SET `ScriptName` = 'death_talon_dragonspawn' WHERE `creature_template`.`entry` =12422 LIMIT 1;

struct centerPoints
{
    float fX, fY, fZ;
};

static centerPoints movePoint[]=
{
    {-7595.0f, -1053.0f, 409.0f},
    {-7593.0f, -1041.0f, 408.0f},
    {-7608.0f, -1051.0f, 409.0f},
    {-7598.0f, -1066.0f, 409.0f},
    {-7583.0f, -1056.0f, 408.0f},
};

//NPC_BLACKWING_MAGE
#define SPELL_ARKANE_EXPLOSION	22271
#define SPELL_FIREBALL			17290

struct MANGOS_DLL_DECL blackwing_mageAI : public ScriptedAI
{
    blackwing_mageAI(Creature* pCreature) : ScriptedAI(pCreature)  
    {
        m_creature->SetMaxHealth(7330);
        m_creature->SetMaxPower(POWER_MANA, 12200);
        Reset();
    }

	uint32 arkane_explosion_timer;
	uint32 fireball_timer; 
    uint32 CENTERPOINT;
    bool startMoving;
    bool movedToCenter;

    void Reset()
    {
		arkane_explosion_timer = 2000;
		fireball_timer = 0;
        movedToCenter = false;
        startMoving = false;
        CENTERPOINT = 0;        
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        if (uiMotionType == POINT_MOTION_TYPE && uiPointId == CENTERPOINT)
        {
            movedToCenter = true;
            SetCombatMovement(true);
            m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_FIREBALL);
        }
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage)
    {
        //if creature gets attacked by a other player
        if (!movedToCenter)
        {
            m_creature->GetMotionMaster()->Clear();
            m_creature->GetMotionMaster()->MoveChase(pDoneBy);
            movedToCenter = true;
            SetCombatMovement(true);           
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!startMoving && m_creature->isInCombat())
        {
            m_creature->GetMotionMaster()->Clear();
            m_creature->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
            m_creature->SetSpeedRate(MOVE_RUN, 1.3f);
            SetCombatMovement(false);
            uint8 pointID = urand(0, 4);
            m_creature->GetMotionMaster()->MovePoint(CENTERPOINT, movePoint[pointID].fX, movePoint[pointID].fY, movePoint[pointID].fZ);       
            startMoving = true;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim() || !movedToCenter)
            return;

		if (arkane_explosion_timer <= diff)
		{
			uint8 player_count = 0;
			Map* pInstance = m_creature->GetMap();
			Map::PlayerList const& players = pInstance->GetPlayers();
			if (!players.isEmpty())
			{
				for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
				{
					if (Player* pPlayer = itr->getSource())
					{
						if (m_creature->GetDistance(pPlayer) < 10)
							player_count++;
					}
				}
			}
			if (player_count > 1)
			{
                //Castet nurnoch arkane explosion, wenn zu viele spieler knubbeln
				if (DoCastSpellIfCan(m_creature,SPELL_ARKANE_EXPLOSION) == CAST_OK)
				    fireball_timer = 5000;
			}
			arkane_explosion_timer = 2000;		
		}
		else
			arkane_explosion_timer -= diff;

		if (fireball_timer <= diff)
		{
			if (DoCastSpellIfCan(m_creature->getVictim(),SPELL_FIREBALL)  == CAST_OK)
			    fireball_timer = 3000;
		}
		else
			fireball_timer -= diff;

		DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_blackwing_mage(Creature* pCreature)
{
    return new blackwing_mageAI(pCreature);
}


//NPC_BLACKWING_LEGIONNAIRE / GUARD
#define SPELL_DRACHENFLUCH			23967
#define SPELL_SPALTEN				15284
#define SPELL_STOSS					15580

struct MANGOS_DLL_DECL blackwing_meleeAI : public ScriptedAI
{
    blackwing_meleeAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
        m_creature->SetMaxHealth(9160);
        /*
        UPDATE `creature_template` SET `mindmg` = '800',
        `maxdmg` = '1500',
        `attackpower` = '3000',
        `baseattacktime` = '2000' WHERE `creature_template`.`entry` =12416 LIMIT 1 ;
        */
        Reset();
    }

	uint32 drachenfluch_timer;
	uint32 spalten_timer;
	uint32 stoss_timer;

    void Reset()
    {
		drachenfluch_timer = 20000;
		spalten_timer = 12000;
		stoss_timer = 8000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

		if (drachenfluch_timer < diff)
		{
			if (DoCastSpellIfCan(m_creature,SPELL_DRACHENFLUCH) == CAST_OK)
			    drachenfluch_timer = 20000;
		}
		else
			drachenfluch_timer -= diff;

		if (spalten_timer < diff)
		{
			if (DoCastSpellIfCan(m_creature->getVictim(),SPELL_SPALTEN) == CAST_OK)
			    spalten_timer = 12000;
		}
		else
			spalten_timer -= diff;

		if (stoss_timer < diff)
		{
			if (DoCastSpellIfCan(m_creature->getVictim(),SPELL_STOSS) == CAST_OK)
			    stoss_timer = 8000;
		}
		else
			stoss_timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_blackwing_melee(Creature* pCreature)
{
    return new blackwing_meleeAI(pCreature);
}

//NPC_DEATH_TALON_DRAGONSPAWN
struct MANGOS_DLL_DECL death_talon_dragonspawnAI : public ScriptedAI
{
    death_talon_dragonspawnAI(Creature* pCreature) : ScriptedAI(pCreature) 
	{
		m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
        m_creature->SetMaxHealth(45800);
        /*
        UPDATE `creature_template` SET `mindmg` = '1500',
        `maxdmg` = '2500',
        `attackpower` = '5000',
        `baseattacktime` = '2500' WHERE `creature_template`.`entry` =12422 LIMIT 1 ;
        */
		Reset();
	}

	uint32 drachenfluch_timer;
	uint32 spalten_timer;
	uint32 stoss_timer;

    void Reset()
    {
		drachenfluch_timer = 20000;
		spalten_timer = 12000;
		stoss_timer = 8000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

		if (drachenfluch_timer < diff)
		{
			if (DoCastSpellIfCan(m_creature,SPELL_DRACHENFLUCH) == CAST_OK)
			    drachenfluch_timer = 20000;
		}
		else
			drachenfluch_timer -= diff;

		if (spalten_timer < diff)
		{
			if (DoCastSpellIfCan(m_creature->getVictim(),SPELL_SPALTEN) == CAST_OK)
			    spalten_timer = 12000;
		}
		else
			spalten_timer -= diff;

		if (stoss_timer < diff)
		{
			if (DoCastSpellIfCan(m_creature->getVictim(),SPELL_STOSS) == CAST_OK)
			    stoss_timer = 8000;
		}
		else
			stoss_timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_death_talon_dragonspawn(Creature* pCreature)
{
    return new death_talon_dragonspawnAI(pCreature);
}

// !!! DEATH TALON TRASH GROUPS !!!
enum deathTalonSpells
{
    //Captain
    SPELL_AURA_OF_FLAMES        = 22436,
    SPELL_CLEAVE                = 15496,
    SPELL_INTIMIDATING_ORDERS   = 22440,
    SPELL_MARK_OF_DETONATION    = 22438,

    //Flamescale
    SPELL_BERSERKER_CHARGE      = 16636,
    SPELL_FLAME_SHOCK           = 22423,

    //Wyrmkin
    SPELL_BLAST_WAVE            = 22424,
    SPELL_FIREBALL_VOLLEY       = 22425,

    //Seether
    SPELL_ENRAGE                = 22428,
    SPELL_FLAME_BUFFET          = 22433,
};

//Captain
struct MANGOS_DLL_DECL death_talon_captainAI : public ScriptedAI
{
    death_talon_captainAI(Creature* pCreature) : ScriptedAI(pCreature) 
	{
        Reset();
    }

    uint32 cleave_timer;
    uint32 order_timer;
    uint32 detonation_timer;

    void Reset()
    {
        cleave_timer = urand(3000, 7000);
        order_timer = urand(8000, 15000);
        detonation_timer = urand(3000, 12000);
    }

    void Aggro(Unit* pWho)
    {
        DoCastSpellIfCan(m_creature, SPELL_AURA_OF_FLAMES);
        m_creature->CallForHelp(10.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (cleave_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                cleave_timer = urand(3000, 7000);
        }
        else
            cleave_timer -= diff;

        if (order_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_INTIMIDATING_ORDERS) == CAST_OK)
                order_timer = urand(8000, 15000);
        }
        else
            order_timer -= diff;

        if (detonation_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MARK_OF_DETONATION) == CAST_OK)
                detonation_timer = urand(3000, 12000);
        }
        else
            detonation_timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_death_talon_captain(Creature* pCreature)
{
    return new death_talon_captainAI(pCreature);
}

//Flamescale
struct MANGOS_DLL_DECL death_talon_flamescaleAI : public ScriptedAI
{
    death_talon_flamescaleAI(Creature* pCreature) : ScriptedAI(pCreature) 
	{
        Reset();
    }

    uint32 shock_timer;

    void Reset()
    {
        shock_timer = urand(3000, 8000);
    }

    void Aggro(Unit* pWho)
    {
        DoCastSpellIfCan(pWho, SPELL_BERSERKER_CHARGE);
        m_creature->CallForHelp(10.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_creature->IsWithinDist(m_creature->getVictim(), 10.0f))
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_BERSERKER_CHARGE);

        if (shock_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MARK_OF_DETONATION) == CAST_OK)
                shock_timer = urand(3000, 8000);
        }
        else
            shock_timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_death_talon_flamescale(Creature* pCreature)
{
    return new death_talon_flamescaleAI(pCreature);
}

//Wyrmkin
struct MANGOS_DLL_DECL death_talon_wyrmkinAI : public ScriptedAI
{
    death_talon_wyrmkinAI(Creature* pCreature) : ScriptedAI(pCreature) 
	{
        Reset();
    }

    uint32 wave_timer;
    uint32 volley_timer;

    void Reset()
    {
        wave_timer = urand(5000, 12000);
        volley_timer = urand(8000, 15000);
    }

    void Aggro(Unit* pWho)
    {
        m_creature->CallForHelp(10.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (wave_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BLAST_WAVE) == CAST_OK)
                wave_timer = urand(5000, 12000);
        }
        else
            wave_timer -= diff;

        if (volley_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FIREBALL_VOLLEY) == CAST_OK)
                volley_timer = urand(8000, 15000);
        }
        else
            volley_timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_death_talon_wyrmkin(Creature* pCreature)
{
    return new death_talon_wyrmkinAI(pCreature);
}

//Seether
struct MANGOS_DLL_DECL death_talon_seetherAI : public ScriptedAI
{
    death_talon_seetherAI(Creature* pCreature) : ScriptedAI(pCreature) 
	{
        Reset();
    }

    uint32 enrage_timer;
    uint32 buffet_timer;

    void Reset()
    {
        enrage_timer = urand(12000, 20000);
        buffet_timer = urand(3000, 12000);
    }

    void Aggro(Unit* pWho)
    {
        m_creature->CallForHelp(10.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (enrage_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                enrage_timer = urand(12000, 20000);
        }
        else
            enrage_timer -= diff;

        if (buffet_timer <= diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FIREBALL_VOLLEY) == CAST_OK)
                buffet_timer = urand(3000, 12000);
        }
        else
            buffet_timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_death_talon_seether(Creature* pCreature)
{
    return new death_talon_seetherAI(pCreature);
}


/* AddSC */

void AddSC_blackwing_lair()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "mob_demon_portal";
    pNewScript->GetAI = &GetAI_mob_demon_portal;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_suppression_trigger";
    pNewScript->GetAI = &GetAI_mob_suppression_trigger;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "blackwing_mage";
    pNewScript->GetAI = &GetAI_blackwing_mage;
    pNewScript->RegisterSelf();

	pNewScript = new Script;
    pNewScript->Name = "blackwing_melee";
    pNewScript->GetAI = &GetAI_blackwing_melee;
    pNewScript->RegisterSelf();

	pNewScript = new Script;
    pNewScript->Name = "death_talon_dragonspawn";
    pNewScript->GetAI = &GetAI_death_talon_dragonspawn;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_blackwing_taskmaster";
    pNewScript->GetAI = &GetAI_mob_blackwing_taskmaster;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "death_talon_captain";
    pNewScript->GetAI = &GetAI_death_talon_captain;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "death_talon_flamescale";
    pNewScript->GetAI = &GetAI_death_talon_flamescale;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "death_talon_wyrmkin";
    pNewScript->GetAI = &GetAI_death_talon_wyrmkin;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "death_talon_seether";
    pNewScript->GetAI = &GetAI_death_talon_seether;
    pNewScript->RegisterSelf();
}
