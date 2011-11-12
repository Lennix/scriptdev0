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
SDName: Boss_Ragnaros
SD%Complete: 80
SDComment: To do: Melt Weapon
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

enum eRanaros
{
    SAY_SUMMON_FIRST            = -1409013,
    SAY_SUMMON                  = -1409014,
    SAY_MIGHT                   = -1409015,
    SAY_WRATH                   = -1409016,
    SAY_KILL                    = -1409017,
    SAY_MAGMA_BLAST             = -1409018,

	SPELL_HAND_OF_RAGNAROS      = 19780,
    SPELL_ELEMENTAL_FIRE        = 20564,    
    SPELL_ERUPTION              = 17731,    //Doesnt work
    SPELL_MIGHT_OF_RAGNAROS     = 21154,    //Doesnt do anything except for summoning a dummy, need 19780 (Hand of Ragnaros) or Hammer of Ragnaros
    SPELL_MAGMA_BLAST           = 20565,    
    SPELL_MELT_WEAPON           = 21387,
    SPELL_WRATH_OF_RAGNAROS     = 20566,    //Melee Knockback
    SPELL_LAVA_BURST            = 21158,    //Deals ~1000 AoE damage, shouldnt even be used by ragnaros
    SPELL_RAGNAROS_EMERGE       = 20568,
    SPELL_RAGNAROS_SUBMERGE     = 21107,
    SPELL_RAGNAROS_SUBMERGE_    = 20567,
    SPELL_SONS_OF_FLAME_DUMMY   = 21108,
    SPELL_FIREBALL              = 22425,    // probably wrong id, currently Fireball Volley (hit all enemy targets, causes 1000-2000 fire dmg)
	TRIGGER                     = 777778,
};

static float Sons[8][4]=
{
    {848.74f, -816.10f, -229.74f, 2.61f},
    {852.56f, -849.86f, -228.56f, 2.83f},
    {808.71f, -852.84f, -227.91f, 0.96f},
    {786.59f, -821.13f, -226.35f, 0.94f},
    {796.21f, -800.94f, -226.01f, 0.56f},
    {821.60f, -782.74f, -226.02f, 6.15f},
    {844.92f, -769.45f, -225.52f, 4.45f},
    {839.82f, -810.86f, -229.68f, 4.69f}
};

struct MANGOS_DLL_DECL boss_ragnarosAI : public ScriptedAI
{
    boss_ragnarosAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_molten_core*)pCreature->GetInstanceData();
        SetCombatMovement(false);
        m_creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_FIRE, true);
        Reset();
    }

    instance_molten_core* m_pInstance;

    bool m_bSubmerged;
    bool m_bSubmergedOnce;

    uint32 m_uiSummonCount;

    uint32 m_uiElementalFireTimer;
    uint32 m_uiEmergeTimer;
    uint32 m_uiEruptionTimer;
    uint32 m_uiMightOfRagnarosTimer;
    uint32 m_uiLavaBurstTimer;
    uint32 m_uiMagmaBlastTimer;
    uint32 m_uiSubmergeTimer;
    uint32 m_uiWrathOfRagnarosTimer;
    uint32 m_uiFireballTimer;
	uint32 m_uiMeltWeaponTimer;

    uint8  m_uiEmergePhase;

	Creature* Trigger;

    void Reset()
    {
        m_bSubmerged = false;
        m_bSubmergedOnce = false;

        m_uiSummonCount = 0;

        m_uiElementalFireTimer = 3000;
        m_uiEmergeTimer = 0;
		m_uiMeltWeaponTimer = 10000;
        m_uiEruptionTimer = 15000;
        m_uiMightOfRagnarosTimer = 20000;
        m_uiLavaBurstTimer = 10000;
        m_uiMagmaBlastTimer = 2000;
        m_uiSubmergeTimer = 180000;
        m_uiWrathOfRagnarosTimer = 30000;
        m_uiFireballTimer = urand(10000,20000);

        m_uiEmergePhase = 0;

		m_creature->RemoveAurasDueToSpell(SPELL_MELT_WEAPON);

		Trigger = 0;
    }
	
    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, FAIL);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, DONE);
    }

    void KilledUnit(Unit* /*pVictim*/)
    {
        DoScriptText(SAY_KILL, m_creature);
    }

    void Aggro(Unit* /*pAttacker*/)
    {
		//Start combat with all players in order to prevent ooc rezzing!
		m_creature->SetInCombatWithZone();
		m_creature->CastSpell(m_creature, SPELL_MELT_WEAPON, false);
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAGNAROS, IN_PROGRESS);
    }

    void JustSummoned(Creature* /*pSummoned*/)
    {
        ++m_uiSummonCount;
    }

    void SummonedCreatureJustDied(Creature* /*pSummoned*/)
    {
        --m_uiSummonCount;

        if (m_uiSummonCount == 0 && m_bSubmerged && m_uiEmergeTimer > 2000)
            m_uiEmergeTimer = 2000;
    }

    Player* DoSelectRandomManaPlayer()
    {
        if (!m_creature->CanHaveThreatList())
            return 0;

        GUIDVector vGuids;
        m_creature->FillGuidsListFromThreatList(vGuids);
        std::vector<Player*> vManaPlayers;

        if (!vGuids.empty())
        {
            for (GUIDVector::const_iterator itr = vGuids.begin(); itr != vGuids.end(); ++itr)
            {
                if (Unit* pTarget = m_creature->GetMap()->GetUnit(*itr))
                {
                    // We need only a player
                    if (!pTarget->IsCharmerOrOwnerPlayerOrPlayerItself())
                        continue;

                    // An enemy is not in the meele range and it is a player with mana power
                    if (!m_creature->CanReachWithMeleeAttack(pTarget) && pTarget->getPowerType() == POWER_MANA)
                        vManaPlayers.push_back((Player*)pTarget);
                }
            }
        }

        if (!vManaPlayers.empty())
        {
            std::vector<Player*>::iterator i = vManaPlayers.begin();
            advance(i, (rand() % vManaPlayers.size()));
            return (*i);
        }
        
        return 0;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bSubmerged)
        {
            // Emerge
            if (m_uiEmergeTimer <= uiDiff)
            {
                if (++m_uiEmergePhase == 1)
                {
                    // Emerge animation
                    m_creature->RemoveAurasDueToSpell(SPELL_RAGNAROS_SUBMERGE);
                    DoCastSpellIfCan(m_creature, SPELL_RAGNAROS_EMERGE);

                    m_uiEmergeTimer = 3000; // maybe 2900 as cast time
                }
                else
                {
                    // Ragnaros is fully emerged
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    m_creature->setFaction(m_creature->GetCreatureInfo()->faction_A);

                    m_bSubmerged = false;
                    m_uiEmergePhase = 0;
                    m_uiSubmergeTimer = 180000; // 180000
                }
            }
            else
                m_uiEmergeTimer -= uiDiff;
        }
        else
        {
            // Elemental Fire
            if (m_uiElementalFireTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ELEMENTAL_FIRE) == CAST_OK)
                    m_uiElementalFireTimer = urand(10000, 14000);
            }
            else
                m_uiElementalFireTimer -= uiDiff;

			// Melt Weapon Recast to prevent  bug
            if (m_uiMeltWeaponTimer <= uiDiff)
            {               
                 m_uiMeltWeaponTimer = 10000;
				 m_creature->RemoveAurasDueToSpell(SPELL_MELT_WEAPON);
				 m_creature->CastSpell(m_creature, SPELL_MELT_WEAPON, false);
            }
            else
                m_uiMeltWeaponTimer -= uiDiff;

            // Eruption
            //if (m_uiEruptionTimer <= uiDiff)
            //{
            //    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ERUPTION) == CAST_OK)
            //        m_uiEruptionTimer = urand(20000, 45000);
            //}
            //else
            //    m_uiEruptionTimer -= uiDiff;

            // Eruption
			if (m_uiEruptionTimer <= uiDiff)
			{
				m_uiEruptionTimer = urand(20000, 45000);
			    GUIDVector tList;
                m_creature->FillGuidsListFromThreatList(tList);
                if (!tList.empty())
                {
                    for (GUIDVector::const_iterator i = tList.begin(); i != tList.end(); ++i)
                    {
						Unit* pPlayer = m_creature->GetMap()->GetUnit(*i);
						if (pPlayer && pPlayer->GetTypeId() == TYPEID_PLAYER)
							if (((((Player*)pPlayer)->m_LavaActive == true) && !(((Player*)pPlayer)->IsInWater())) || ((Player*)pPlayer)->IsInWater())
								((Player*)pPlayer)->EnvironmentalDamage(DAMAGE_LAVA, urand(1300,1600));
						   //Damage
					}
				}
			}
			else
				m_uiEruptionTimer -= uiDiff;

            // Might Of Ragnaros
			if (Trigger) {
             Trigger->CastSpell(Trigger,SPELL_LAVA_BURST,false);
			 Trigger = 0;
			} 

            if (m_uiMightOfRagnarosTimer <= uiDiff)
            {
                if (Player* pManaPlayer = DoSelectRandomManaPlayer())
					if (DoCastSpellIfCan(pManaPlayer, SPELL_MIGHT_OF_RAGNAROS) == CAST_OK)
					{
						//Need a trigger to cast self-targeted aoe spells
						Trigger = m_creature->SummonCreature(TRIGGER, pManaPlayer->GetPositionX(), pManaPlayer->GetPositionY(), pManaPlayer->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 3000);
						if (Trigger)
						{
							Trigger->setFaction(14);
							Trigger->SetVisibility(VISIBILITY_OFF);
							Trigger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
							Trigger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
							Trigger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
					        Trigger->CastSpell(Trigger,SPELL_HAND_OF_RAGNAROS,false);
					        //Trigger->CastSpell(Trigger,SPELL_WRATH_OF_RAGNAROS,false);	
						}
                        DoScriptText(SAY_MIGHT, m_creature);

					}
                m_uiMightOfRagnarosTimer = urand(20000, 30000);
            }
            else
                m_uiMightOfRagnarosTimer -= uiDiff;

            // Wrath Of Ragnaros
            if (m_uiWrathOfRagnarosTimer <= uiDiff)
            {
				//He should not wait until he actually has a target to knock back
                //if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WRATH_OF_RAGNAROS) == CAST_OK)
                //{
				    DoCastSpellIfCan(m_creature, SPELL_WRATH_OF_RAGNAROS);
                    DoScriptText(SAY_WRATH, m_creature);
                    m_uiWrathOfRagnarosTimer = 20000;
                //}
            }
            else
                m_uiWrathOfRagnarosTimer -= uiDiff;

            // Lava Burst //shouldnt be used by ragnaros!
            //if (m_uiLavaBurstTimer <= uiDiff)
            //{
            //    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_LAVA_BURST) == CAST_OK)
            //        m_uiLavaBurstTimer = 10000;
            //}
            //else
            //    m_uiLavaBurstTimer -= uiDiff;

            // Fireball //shouldnt be used by ragnaros!
            //if (m_uiFireballTimer <= uiDiff)
            //{
            //    if (DoCastSpellIfCan(m_creature, SPELL_FIREBALL) == CAST_OK)
            //        m_uiFireballTimer = urand(10000, 20000);
            //}
            //else
            //    m_uiFireballTimer -= uiDiff;

            // Submerge Timer
            if (m_uiSubmergeTimer <= uiDiff)
            {
                m_creature->AttackStop();
                if (m_creature->IsNonMeleeSpellCasted(false))
                    m_creature->InterruptNonMeleeSpells(false);
                //DoCastSpellIfCan(m_creature, 23973);
                m_creature->setFaction(FACTION_FRIENDLY);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                if (!m_bSubmergedOnce)
                {
                    DoScriptText(SAY_SUMMON_FIRST, m_creature);
                    m_bSubmergedOnce = true;
                }
                else
                    DoScriptText(SAY_SUMMON, m_creature);

                for(uint8 i = 0; i < 8; ++i)
				{
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
					{
                        if (Creature* pSon = m_creature->SummonCreature(NPC_SON_OF_FLAME, Sons[i][0], Sons[i][1], Sons[i][2], Sons[i][3], TEMPSUMMON_DEAD_DESPAWN, 10000))
						{
                            pSon->SetMeleeDamageSchool(SPELL_SCHOOL_FIRE);
							pSon->AI()->AttackStart(pTarget);
						}
					}
				}
                //DoCastSpellIfCan(m_creature, SPELL_RAGNAROS_SUBMERGE_);   // Animation of submerge
                DoCastSpellIfCan(m_creature, SPELL_RAGNAROS_SUBMERGE, CAST_INTERRUPT_PREVIOUS);      // Passive

                m_bSubmerged = true;
                m_uiEmergeTimer = 90000;
                return;
            }
            else
                m_uiSubmergeTimer -= uiDiff;

            // If we are within range melee the target
            if (m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
            {
                // Make sure our attack is ready and we arn't currently casting
                if (m_creature->isAttackReady() && !m_creature->IsNonMeleeSpellCasted(false))
                {
                    m_creature->AttackerStateUpdate(m_creature->getVictim());
                    m_creature->resetAttackTimer();
                    //m_creature->CastSpell(m_creature, SPELL_MELT_WEAPON, false);
                }
            }
            else
            {
                // Look-up for a new closest target in melee range
                // If an exist, attack him..
                GUIDVector vGuids;
                m_creature->FillGuidsListFromThreatList(vGuids);
                if (!vGuids.empty())
                {
                    for (GUIDVector::const_iterator itr = vGuids.begin(); itr != vGuids.end(); ++itr)
                    {
                        if (Unit* pTarget = m_creature->GetMap()->GetUnit(*itr))
                        {
							//Set threat of all targets who are not in melee range to a very low value
                            if (!m_creature->CanReachWithMeleeAttack(pTarget))
								m_creature->getThreatManager().modifyThreatPercent(pTarget,-75);
							else
								m_creature->getThreatManager().addThreat(pTarget, 500.0f);
                        }
                    }
					//Proceed to Magma Blast casting if there's no target in melee range
					if (m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
						return;
                }

				 //    // Must to be a new target
                 //if (m_creature->getVictim()->GetObjectGuid() != pTarget->GetObjectGuid())
                 //{
                 //    DoResetThreat();
                 //    //AttackStart(pTarget);
                 //    m_creature->getThreatManager().addThreat(pTarget, 1000.0f);
                 //    return;
                 //}

                // Magma Blast
                if (m_uiMagmaBlastTimer <= uiDiff)
                {   
					Unit* bTarget = vGuids.size() > 1 ? m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO,1) : m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO,0);
                    if (DoCastSpellIfCan(bTarget, SPELL_MAGMA_BLAST) == CAST_OK)
                    {
                        DoScriptText(SAY_MAGMA_BLAST, m_creature);
                        m_uiMagmaBlastTimer = 2500;
                    }
                }
                else
                    m_uiMagmaBlastTimer -= uiDiff;
            }
        }
    }
};

CreatureAI* GetAI_boss_ragnaros(Creature* pCreature)
{
    return new boss_ragnarosAI(pCreature);
}

void AddSC_boss_ragnaros()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "boss_ragnaros";
    pNewScript->GetAI = &GetAI_boss_ragnaros;
    pNewScript->RegisterSelf();
}