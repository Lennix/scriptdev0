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
SDName: boss_cannon_master_willey
SD%Complete: 100
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "stratholme.h"

static Locations Spawn[]=
{
    {3553.85f,-2945.88f,125.00f},
    {3559.20f,-2952.92f,125.00f},
    {3552.41f,-2948.66f,125.00f},
    {3555.65f,-2953.51f,125.00f},
    {3547.92f,-2950.97f,125.00f},
    {3553.09f,-2952.12f,125.00f},
    {3552.72f,-2957.77f,125.00f},
    {3547.15f,-2953.16f,125.00f},
    {3550.20f,-2957.43f,125.00f}
};

enum Spells
{
    SPELL_KNOCK_AWAY              = 10101,
    SPELL_PUMMEL                  = 15615,
    SPELL_SHOOT                   = 20463,
    SPELL_SUMMON_CRIMSON_RIFLEMAN = 17279
};

struct MANGOS_DLL_DECL boss_cannon_master_willeyAI : public ScriptedAI
{
    boss_cannon_master_willeyAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiKnockAwayTimer;
    uint32 m_uiPummelTimer;
    uint32 m_uiShootTimer;
    uint32 m_uiSummonRiflemanTimer;

    void Reset()
    {
        m_uiKnockAwayTimer = urand(8000,10000);
        m_uiPummelTimer = urand(3000,6000);
        m_uiShootTimer = 0;
        m_uiSummonRiflemanTimer = urand(12000,14000);
    }

    void JustDied(Unit* pKiller)
    {
        for(uint32 i = 0; i < 7; ++i)
        {
            Creature* pRifleman = m_creature->SummonCreature(NPC_CRIMSON_RIFLEMAN, Spawn[i].x, Spawn[i].y, Spawn[i].z, 0.59f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Pummel spell
        if (m_uiPummelTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_PUMMEL);
            m_uiPummelTimer = urand(6000,8000);
        }
        else
            m_uiPummelTimer -= uiDiff;

        // Knock Away spell
        if (m_uiKnockAwayTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCK_AWAY);
            m_uiKnockAwayTimer = urand(8000,10000);
        }
        else
            m_uiKnockAwayTimer -= uiDiff;

        // Shoot
        if (m_uiShootTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHOOT);
            m_uiShootTimer = urand(1000,3000);
        }
        else
            m_uiShootTimer -= uiDiff;

        // Summon Rifleman
        if (m_uiSummonRiflemanTimer <= uiDiff)
        {
            for(uint32 i = 0; i < 4; ++i)
            {
                uint32 j = urand(0,8);
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                Creature* pRifleman = m_creature->SummonCreature(NPC_CRIMSON_RIFLEMAN, Spawn[j].x, Spawn[j].y, Spawn[j].z, 0.59f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                if (pTarget && pRifleman)
                    pRifleman->AI()->AttackStart(pTarget);
            }
            m_uiSummonRiflemanTimer = urand(15000,25000);
        }
        else
            m_uiSummonRiflemanTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_cannon_master_willey(Creature* pCreature)
{
    return new boss_cannon_master_willeyAI(pCreature);
}

/*######
## NPC_CRIMSON_RIFLEMAN
######*/

enum
{
    SPELL_RIFLEMAN_SHOOT = 17353
};

struct MANGOS_DLL_DECL npc_crimson_riflemanAI : public ScriptedAI
{
    npc_crimson_riflemanAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_stratholme*)pCreature->GetInstanceData();
		SetCombatMovement(false);
        Reset();
    }

    instance_stratholme* m_pInstance;

    uint32 m_uiShootTimer;

    void Reset()
    {
        m_uiShootTimer = urand(0,1000);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

        if (m_uiShootTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_RIFLEMAN_SHOOT);
            m_uiShootTimer = urand(1000,2000);
        }
        else
            m_uiShootTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_crimson_riflemanAI(Creature* pCreature)
{
    return new npc_crimson_riflemanAI(pCreature);
}

void AddSC_boss_cannon_master_willey()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "boss_cannon_master_willey";
    pNewscript->GetAI = &GetAI_boss_cannon_master_willey;
    pNewscript->RegisterSelf();

	pNewscript = new Script;
    pNewscript->Name = "npc_crimson_rifleman";
    pNewscript->GetAI = &GetAI_npc_crimson_riflemanAI;
    pNewscript->RegisterSelf();
}
