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
SDName: Boss_Sulfuron_Harbringer
SD%Complete: 80
SDComment:
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"

enum eSulfuron
{
    // Sulfuron Harbinger
    SPELL_DEMORALIZING_SHOUT    = 19778,
    SPELL_INSPIRE               = 19779,
    SPELL_HAND_OF_RAGNAROS      = 19780,
    SPELL_FLAME_SPEAR           = 19781,

    // Adds Spells
    SPELL_DARKSTRIKE            = 19777,
    SPELL_HEAL                  = 19775,
    SPELL_SHADOW_WORD_PAIN      = 19776,
    SPELL_IMMOLATE              = 20294
};

struct MANGOS_DLL_DECL boss_sulfuronAI : public ScriptedAI
{
    boss_sulfuronAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_molten_core*)pCreature->GetInstanceData();
        Reset();
    }

    instance_molten_core* m_pInstance;

    uint32 m_uiDemoralizingShoutTimer;
    uint32 m_uiInspireTimer;
    uint32 m_uiHandOfRagnarosTimer;
    uint32 m_uiFlameSpearTimer;

    void Reset()
    {
        m_uiDemoralizingShoutTimer = 15000;                 // These times are probably wrong
        m_uiInspireTimer = 13000;
        m_uiHandOfRagnarosTimer = 6000;
        m_uiFlameSpearTimer = 2000;
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SULFURON, FAIL);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SULFURON, DONE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Demoralizing Shout
        if (m_uiDemoralizingShoutTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_DEMORALIZING_SHOUT);
            m_uiDemoralizingShoutTimer = urand(15000, 20000);
        }
        else
            m_uiDemoralizingShoutTimer -= uiDiff;

        // Inspire
        if (m_uiInspireTimer < uiDiff)
        {
            Creature* pTarget = NULL;
            std::list<Creature*> pList = DoFindFriendlyMissingBuff(45.0f, SPELL_INSPIRE);
            if (!pList.empty())
            {
                std::list<Creature*>::iterator i = pList.begin();
                advance(i, (rand()%pList.size()));
                pTarget = (*i);
            }

            if (pTarget)
                DoCastSpellIfCan(pTarget, SPELL_INSPIRE);

            DoCastSpellIfCan(m_creature, SPELL_INSPIRE);

            m_uiInspireTimer = urand(20000, 26000);
        }
        else
            m_uiInspireTimer -= uiDiff;

        // Hand of Ragnaros
        if (m_uiHandOfRagnarosTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_HAND_OF_RAGNAROS);
            m_uiHandOfRagnarosTimer = urand(12000, 15000);
        }
        else
            m_uiHandOfRagnarosTimer -= uiDiff;

        // Flame Spear
        if (m_uiFlameSpearTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCastSpellIfCan(pTarget, SPELL_FLAME_SPEAR);

            m_uiFlameSpearTimer = urand(12000, 16000);
        }
        else
            m_uiFlameSpearTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_sulfuron(Creature* pCreature)
{
    return new boss_sulfuronAI(pCreature);
}

struct MANGOS_DLL_DECL mob_flamewaker_priestAI : public ScriptedAI
{
    mob_flamewaker_priestAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiDarkStrikeTimer;
    uint32 m_uiHealTimer;
    uint32 m_uiShadowWordPainTimer;
    uint32 m_uiImmolateTimer;

    void Reset()
    {
        m_uiDarkStrikeTimer = 10000;
        m_uiHealTimer = urand(15000, 30000);
        m_uiShadowWordPainTimer = 2000;
        m_uiImmolateTimer = 8000;

        m_creature->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK, true);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Dark Strike
        if (m_uiDarkStrikeTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_DARKSTRIKE);
            m_uiDarkStrikeTimer = urand(12000, 16000);
        }
        else
            m_uiDarkStrikeTimer -= uiDiff;

        // Casting Heal to Sulfuron or other Guards.
        if (m_uiHealTimer < uiDiff)
        {
            if (Unit* pUnit = DoSelectLowestHpFriendly(60.0f, 1))
                DoCastSpellIfCan(pUnit, SPELL_HEAL);

            m_uiHealTimer = urand(15000, 20000);
        }
        else
            m_uiHealTimer -= uiDiff;

        // Shadow Word: Pain
        if (m_uiShadowWordPainTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCastSpellIfCan(pTarget, SPELL_SHADOW_WORD_PAIN);

            m_uiShadowWordPainTimer = urand(10000, 20000);
        }
        else
            m_uiShadowWordPainTimer -= uiDiff;

        // Immolate
        if (m_uiImmolateTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                DoCastSpellIfCan(pTarget, SPELL_IMMOLATE);

            m_uiImmolateTimer = urand(10000, 20000);
        }
        else
            m_uiImmolateTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_flamewaker_priest(Creature* pCreature)
{
    return new mob_flamewaker_priestAI(pCreature);
}

void AddSC_boss_sulfuron()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "boss_sulfuron";
    pNewscript->GetAI = &GetAI_boss_sulfuron;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "mob_flamewaker_priest";
    pNewscript->GetAI = &GetAI_mob_flamewaker_priest;
    pNewscript->RegisterSelf();
}
