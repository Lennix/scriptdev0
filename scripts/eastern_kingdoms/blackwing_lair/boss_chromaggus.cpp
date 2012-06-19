/*
 * Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
 * Copyright (C) 2010-2011 ScriptDev0 <http://github.com/mangos-zero/scriptdev0>
 *
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
            - chromatic mutation needs test

SDCategory: Blackwing Lair
EndScriptData 

Button Sql:
INSERT INTO `gameobject` VALUES ('632891', '179148', '469', '-7510.41', '-1094.64', '476.555', '2.0929', '0', '0', '0.865652', '0.500646', '25', '100', '1');
*/



#include "precompiled.h"
#include "blackwing_lair.h"

enum
{
    EMOTE_GENERIC_FRENZY_KILL   = -1000001,
    EMOTE_SHIMMER               = -1469003,

    // These spells are actually called elemental shield
    // What they do is decrease all damage by 75% then they increase
    // One school of damage by 1100%
    SPELL_FIRE_VULNERABILITY    = 22277,
    SPELL_FROST_VULNERABILITY   = 22278,
    SPELL_SHADOW_VULNERABILITY  = 22279,
    SPELL_NATURE_VULNERABILITY  = 22280,
    SPELL_ARCANE_VULNERABILITY  = 22281,

    MAX_BREATHS                 = 5,
    MAX_VULNERABILITY			= 5,
    MAX_AFFLICTION				= 5,
    SPELL_INCINERATE            = 23308,                    // Incinerate 23308,23309
    SPELL_TIME_LAPSE            = 23310,                    // Time lapse 23310, 23311(old threat mod that was removed in 2.01)
    SPELL_CORROSIVE_ACID        = 23313,                    // Corrosive Acid 23313, 23314
    SPELL_IGNITE_FLESH          = 23315,                    // Ignite Flesh 23315,23316
    SPELL_FROST_BURN            = 23187,                    // Frost burn 23187, 23189

    // Brood Affliction 23173 - Scripted Spell that cycles through all targets within 100 yards and has a chance to cast one of the afflictions on them
    // Since Scripted spells arn't coded I'll just write a function that does the same thing
    SPELL_BROODAF_BLUE          = 23153,                    // Blue affliction 23153
    SPELL_BROODAF_BLACK         = 23154,                    // Black affliction 23154
    SPELL_BROODAF_RED           = 23155,                    // Red affliction 23155 (23168 on death)
    SPELL_BROODAF_BRONZE        = 23170,                    // Bronze Affliction  23170
    SPELL_BROODAF_GREEN         = 23169,                    // Brood Affliction Green 23169

    SPELL_CHROMATIC_MUT_1       = 23174,                    // Spell cast on player if they get 4 debuffs

    SPELL_FRENZY                = 28371,                    // The frenzy spell may be wrong
    SPELL_ENRAGE                = 28747
};

static const uint32 aPossibleBreaths[MAX_BREATHS] = {SPELL_INCINERATE, SPELL_TIME_LAPSE, SPELL_CORROSIVE_ACID, SPELL_IGNITE_FLESH, SPELL_FROST_BURN};
static const uint32 aPossibleVulnerabilities[MAX_VULNERABILITY] = {SPELL_FIRE_VULNERABILITY, SPELL_FROST_VULNERABILITY, SPELL_SHADOW_VULNERABILITY, SPELL_NATURE_VULNERABILITY, SPELL_ARCANE_VULNERABILITY};
static const uint32 aPossibleAfflictions[MAX_AFFLICTION] = {SPELL_BROODAF_BLUE, SPELL_BROODAF_BLACK, SPELL_BROODAF_RED, SPELL_BROODAF_BRONZE, SPELL_BROODAF_GREEN};

struct MANGOS_DLL_DECL boss_chromaggusAI : public ScriptedAI
{
    boss_chromaggusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        // Select the 2 different breaths that we are going to use until despawned
        // 5 possiblities for the first breath, 4 for the second, 20 total possiblites

        // select two different numbers between 0..MAX_BREATHS-1
        uint8 uiPos1 = urand(0, MAX_BREATHS - 1);
        uint8 uiPos2;
        do uiPos2 = urand(0, MAX_BREATHS - 1); while (uiPos1 == uiPos2);

        m_uiBreathOneSpell = aPossibleBreaths[uiPos1];
        m_uiBreathTwoSpell = aPossibleBreaths[uiPos2];

        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiBreathOneSpell;
    uint32 m_uiBreathTwoSpell;
    uint32 m_uiCurrentVulnerabilitySpell;

    uint32 m_uiShimmerTimer;
    uint32 m_uiBreathTimer;
    uint32 m_uiAfflictionTimer;
    uint32 m_uiFrenzyTimer;
    bool m_switchBreath;
    bool m_bEnraged;

    void Reset()
    {
        m_uiCurrentVulnerabilitySpell = 0;                  // We use this to store our last vulnerability spell so we can remove it later

        m_uiShimmerTimer    = 0;                            // Time till we change vurlnerabilites
        m_uiBreathTimer		= 30000;                        // First breath is 30 seconds
        m_uiAfflictionTimer = 3000;                        // This is special - 5 seconds means that we cast this on 1 pPlayer every 5 sconds
        m_uiFrenzyTimer     = urand(5000, 12000);

        m_switchBreath		= false;
        m_bEnraged          = false;
    }

    void Aggro(Unit* pWho)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CHROMAGGUS, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CHROMAGGUS, DONE);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CHROMAGGUS, FAIL);
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

        // --- SHIMMER ---
        if (m_uiShimmerTimer < uiDiff)
        {
            // Remove old vulnerability spell
            if (m_uiCurrentVulnerabilitySpell)
                m_creature->RemoveAurasDueToSpell(m_uiCurrentVulnerabilitySpell);
            // Cast new random vurlnabilty on self
            uint32 uiSpell;
            do uiSpell = aPossibleVulnerabilities[urand(0,4)]; while(uiSpell == m_uiCurrentVulnerabilitySpell);
            if (DoCastSpellIfCan(m_creature, uiSpell) == CAST_OK)
            {
                m_uiCurrentVulnerabilitySpell = uiSpell;
                DoScriptText(EMOTE_SHIMMER, m_creature);
                m_uiShimmerTimer = 45000;
            }
        }
        else
            m_uiShimmerTimer -= uiDiff;
        
        // --- BREATH ---
        if (m_uiBreathTimer < uiDiff)
        {
            uint32 breathSpell;
            if (!m_switchBreath)
                breathSpell = m_uiBreathOneSpell;
            else
                breathSpell = m_uiBreathTwoSpell;
            //needs LOS
            if (DoCastSpellIfCan(m_creature, breathSpell) == CAST_OK)
            {
                if (breathSpell == SPELL_TIME_LAPSE)
                {
                    DoResetThreat();
                    if (Player* pNonStunned = GetNotTimeLapsedPlayer())
                    {
                        AttackStart(pNonStunned);
                    }
                }
                m_uiBreathTimer = 30000;
                m_switchBreath = !m_switchBreath;
            }
        }
        else
            m_uiBreathTimer -= uiDiff;
        
        // --- AFFLICTION ---
        if (m_uiAfflictionTimer < uiDiff)
        {
            uint32 m_uiSpellAfflict = aPossibleAfflictions[urand(0,4)];
            std::vector<ObjectGuid> vGuids;
            m_creature->FillGuidsListFromThreatList(vGuids);
            for (std::vector<ObjectGuid>::const_iterator i = vGuids.begin();i != vGuids.end(); ++i)
            {
                Unit* pUnit = m_creature->GetMap()->GetUnit(*i);
                if (pUnit)
                {
                    // Cast affliction, needs no LOS
                    m_creature->CastSpell(pUnit, m_uiSpellAfflict, true);
                    // Chromatic mutation if target is effected by 4 afflictions
                    uint8 afflictionCounter = 0;
                    for (uint8 i = 0; i < 5; i++)
                    {
                        if (pUnit->HasAura(aPossibleAfflictions[i], EFFECT_INDEX_0))
                            afflictionCounter++;
                    }
                    if (afflictionCounter > 3)
                    {
                        pUnit->RemoveAllAuras();
                        m_creature->CastSpell(pUnit,SPELL_CHROMATIC_MUT_1,true);
                    }
                }
            }
            m_uiAfflictionTimer = 10000;
        }
        else
            m_uiAfflictionTimer -= uiDiff;
        
        // --- FRENZY ---
        if (m_uiFrenzyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_FRENZY_KILL, m_creature);
                m_uiFrenzyTimer = urand(10000, 15000);
            }
        }
        else
            m_uiFrenzyTimer -= uiDiff;

        // --- ENRAGE ---
        if (!m_bEnraged && m_creature->GetHealthPercent() < 20.0f)
        {
            DoCastSpellIfCan(m_creature, SPELL_ENRAGE);
            m_bEnraged = true;
        }

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