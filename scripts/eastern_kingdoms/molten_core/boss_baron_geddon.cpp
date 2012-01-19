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
SDName: Boss_Baron_Geddon
SD%Complete: 100
SDComment:
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

enum eBaronGeddon
{
    EMOTE_SERVICE           = -1409000,

    SPELL_IGNITE_MANA       = 19659,
    SPELL_INFERNO           = 19695,
    SPELL_LIVING_BOMB       = 20475,
    SPELL_ARMAGEDDON        = 20478,
};

struct MANGOS_DLL_DECL boss_baron_geddonAI : public ScriptedAI
{
    boss_baron_geddonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_molten_core*)pCreature->GetInstanceData();
        Reset();
    }

    instance_molten_core* m_pInstance;

    bool m_bArmageddon;

    uint32 m_uiIgniteManaTimer;
    uint32 m_uiInfernoTimer;
    uint32 m_uiLivingBombTimer;
    uint32 m_uiArmageddonTimer;

    void Reset()
    {
        m_bArmageddon = false;

		m_creature->SetBoundingValue(0, 5);
		m_creature->SetBoundingValue(1, 3);
        m_uiIgniteManaTimer = 19000;                              // These times are probably wrong
        m_uiInfernoTimer = 30000;
        m_uiLivingBombTimer = 9000;
        m_uiArmageddonTimer = 0;

        m_creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_FIRE, true);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GEDDON, FAIL);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GEDDON, DONE);
    }

    void GeddonArmageddonDealDamage()
    {
        GUIDVector vGuids;
        m_creature->FillGuidsListFromThreatList(vGuids);
        if (vGuids.empty())
            return;

        for (GUIDVector::const_iterator itr = vGuids.begin(); itr != vGuids.end(); ++itr)
        {
            Unit* pTarget = m_creature->GetMap()->GetUnit(*itr);
			if (pTarget && pTarget->IsWithinDistInMap(m_creature, 20.0f) && pTarget->IsWithinLOSInMap(m_creature))
				m_creature->DealDamage(pTarget, urand(7600, 8400), NULL, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE, NULL, false);
        }

        m_creature->SetHealth(0);
        m_creature->SetDeathState(JUST_DIED);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bArmageddon)
        {
            if (m_uiArmageddonTimer)
            {
                if (m_uiArmageddonTimer <= uiDiff)
                {
                    m_uiArmageddonTimer = 0;
                    GeddonArmageddonDealDamage();
                }
                else
                    m_uiArmageddonTimer -= uiDiff;
            }
            DoMeleeAttackIfReady();
            return;
        }

        // If we are <2% hp cast Armageddon
        //if (HealthBelowPct(2))
		if (m_creature->GetHealthPercent() < 2.0f)
        {
            m_bArmageddon = true;
            m_creature->InterruptNonMeleeSpells(false);

            // prevent remove debuffs for any interuptions
            for(int i = 0; i < TOTAL_AURAS; ++i)
            {
                Unit::AuraList const& Auras = m_creature->GetAurasByType(AuraType(i));
                for(Unit::AuraList::const_iterator itr = Auras.begin(); itr != Auras.end(); ++itr)
                {
                    SpellEntry const* spell = (*itr)->GetSpellProto();
                    if (!(*itr)->IsPositive())
                    {
                        //m_creature->RemoveAura(*itr);
                        m_creature->RemoveAurasDueToSpell(spell->Id);
                        itr = Auras.begin();
                    }
                }
            }

            DoScriptText(EMOTE_SERVICE, m_creature);
            DoCastSpellIfCan(m_creature, SPELL_ARMAGEDDON, CAST_TRIGGERED);
            
            m_uiArmageddonTimer = 5000;
            return;
        }

        // Ignite Mana
        if (m_uiIgniteManaTimer <= uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCastSpellIfCan(pTarget, SPELL_IGNITE_MANA);

            m_uiIgniteManaTimer = 20000;
        }
        else
            m_uiIgniteManaTimer -= uiDiff;

        // Inferno
        if (m_uiInfernoTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_INFERNO);
            m_uiInfernoTimer = 30000;
        }
        else
            m_uiInfernoTimer -= uiDiff;

        // Living Bomb
        if (m_uiLivingBombTimer <= uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCastSpellIfCan(pTarget, SPELL_LIVING_BOMB);

            m_uiLivingBombTimer = 20000;
        }
        else
            m_uiLivingBombTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_baron_geddon(Creature* pCreature)
{
    return new boss_baron_geddonAI(pCreature);
}

void AddSC_boss_baron_geddon()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "boss_baron_geddon";
    pNewscript->GetAI = &GetAI_boss_baron_geddon;
    pNewscript->RegisterSelf();
}
