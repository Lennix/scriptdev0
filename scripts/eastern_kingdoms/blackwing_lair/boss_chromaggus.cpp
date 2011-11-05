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
SDName: Boss_Chromaggus
SD%Complete: 95
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "blackwing_lair.h"

enum eChromaggus
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,
    EMOTE_SHIMMER               = -1469003,

    SPELL_BROOD_AFFLICTION      = 23173,                    // handled in core
    SPELL_ENRAGE                = 23128,
    SPELL_FRENZY                = 23537,

    // Breaths
    SPELL_CORROSIVE_ACID        = 23313,                    // Corrosive Acid 23313, 23314
    SPELL_FROST_BURN            = 23187,                    // Frost burn 23187, 23189
    SPELL_IGNITE_FLESH          = 23315,                    // Ignite Flesh 23315,23316
    SPELL_INCINERATE            = 23308,                    // Incinerate 23308,23309
    SPELL_TIME_LAPSE            = 23310,                    // Time lapse 23310, 23311(old threat mod that was removed in 2.01)

    SPELL_FIRE_VURNALBILTY      = 22277,
    SPELL_FROST_VURNALBILTY     = 22278,
    SPELL_SHADOW_VURNALBILTY    = 22279,
    SPELL_NATURE_VURNALBILTY    = 22280,
    SPELL_ARCANE_VURNALBILTY    = 22281,
};

uint32 aBreaths[5] = {SPELL_CORROSIVE_ACID, SPELL_FROST_BURN, SPELL_IGNITE_FLESH, SPELL_INCINERATE, SPELL_TIME_LAPSE};
uint32 aVurn[5] = {SPELL_FIRE_VURNALBILTY, SPELL_FROST_VURNALBILTY, SPELL_SHADOW_VURNALBILTY, SPELL_NATURE_VURNALBILTY, SPELL_ARCANE_VURNALBILTY};

struct MANGOS_DLL_DECL boss_chromaggusAI : public ScriptedAI
{
    boss_chromaggusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_uiBreath1Spell = aBreaths[urand(0,4)];
        m_uiBreath2Spell = aBreaths[urand(0,4)];
        //m_uiBreath1Spell = SPELL_TIME_LAPSE;
        //m_uiBreath2Spell = SPELL_TIME_LAPSE;

        while(m_uiBreath1Spell == m_uiBreath2Spell)
            m_uiBreath2Spell = aBreaths[urand(0,4)];

        //pCreature->setFaction(54);
    }

    uint32 m_uiBreath1Spell;
    uint32 m_uiBreath2Spell;
    uint32 m_uiCurrentVurlnSpell;

    uint32 m_uiAfflictionTimer;
    uint32 m_uiBreath1Timer;
    uint32 m_uiBreath2Timer;
    uint32 m_uiFrenzyTimer;
    uint32 m_uiShimmerTimer;

    void Reset()
    {
        m_uiCurrentVurlnSpell = 0;

        m_uiAfflictionTimer = 10000;
        m_uiBreath1Timer = 30000;
        m_uiBreath2Timer = 60000;
        m_uiFrenzyTimer = 20000;
        m_uiShimmerTimer = 0;
    }

    Player* GetNotTimeLapsedPlayer()
    {
        ThreatList const& tList = m_creature->getThreatManager().getThreatList();
        if (tList.empty())
            return NULL;

        Player* pPlayer = NULL;

        for (ThreatList::const_iterator itr = tList.begin();itr != tList.end(); ++itr)
        {
            Unit* pUnit = m_creature->GetMap()->GetUnit((*itr)->getUnitGuid());

            if (pUnit && pUnit->IsCharmerOrOwnerPlayerOrPlayerItself() && !pUnit->HasAura(SPELL_TIME_LAPSE))
            {
                pPlayer = (Player*)pUnit;
                break;
            }
        }

        return pPlayer;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Enrage
        if (HealthBelowPct(20))
            DoCastSpellIfCan(m_creature, SPELL_ENRAGE, CAST_FORCE_TARGET_SELF + CAST_AURA_NOT_PRESENT);

        // Breath 1
        if (m_uiBreath1Timer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), m_uiBreath1Spell) == CAST_OK)
            {
                if (m_uiBreath1Spell == SPELL_TIME_LAPSE)
                {
                    DoResetThreat();
                    if (Player* pNonStunned = GetNotTimeLapsedPlayer())
                    {
                        AttackStart(pNonStunned);
                    }
                }
                m_uiBreath1Timer = 60000;
            }
        }
        else
            m_uiBreath1Timer -= uiDiff;

        // Breath 2
        if (m_uiBreath2Timer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), m_uiBreath2Spell) == CAST_OK)
            {
                if (m_uiBreath2Spell == SPELL_TIME_LAPSE)
                {
                    DoResetThreat();
                    if (Player* pNonStunned = GetNotTimeLapsedPlayer())
                    {
                        AttackStart(pNonStunned);
                    }
                }
                m_uiBreath2Timer = 60000;
            }
        }
        else
            m_uiBreath2Timer -= uiDiff;

        // Frenzy
        if (m_uiFrenzyTimer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                m_uiFrenzyTimer = urand(10000, 15000);
            }
        }
        else
            m_uiFrenzyTimer -= uiDiff;

        // Affliction
        if (m_uiAfflictionTimer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BROOD_AFFLICTION) == CAST_OK)
                m_uiAfflictionTimer = urand(10000,12000);
        }
        else
            m_uiAfflictionTimer -= uiDiff;

        // Shimmer
        if (m_uiShimmerTimer <= uiDiff)
        {
            if (m_uiCurrentVurlnSpell)
                m_creature->RemoveAurasDueToSpell(m_uiCurrentVurlnSpell);

            uint32 spell = aVurn[urand(0,4)];
            while (spell == m_uiCurrentVurlnSpell)
                spell = aVurn[urand(0,4)];

            if (DoCastSpellIfCan(m_creature, spell) == CAST_OK)
            {
                m_uiCurrentVurlnSpell = spell;
                DoScriptText(EMOTE_SHIMMER, m_creature);
                m_uiShimmerTimer = 45000;
            }
        }
        else
            m_uiShimmerTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_chromaggus(Creature* pCreature)
{
    return new boss_chromaggusAI(pCreature);
}

void AddSC_boss_chromaggus()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "boss_chromaggus";
    pNewScript->GetAI = &GetAI_boss_chromaggus;
    pNewScript->RegisterSelf();
}
