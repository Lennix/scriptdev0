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
SDName: Ruins of Ahn'Qiraj
SD%Complete: 100
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

/* ContentData
mob_anubisath_guardian
mob_flesh_hunter
EndContentData */

#include "precompiled.h"
#include "ruins_of_ahnqiraj.h"

/*######
## mob_anubisath_guardian
######*/

enum eGuardian
{
    SPELL_METEOR                 = 24340,
    SPELL_PLAGUE                 = 22997,
    SPELL_SHADOW_STORM           = 26546,
    SPELL_THUNDER_CLAP           = 26554,
    SPELL_REFLECT_ARFR           = 13022,
    SPELL_REFLECT_FSSH           = 19595,
    SPELL_ENRAGE                 = 8559,
    SPELL_EXPLODE                = 25698,

    SPELL_SUMMON_ANUB_SWARMGUARD = 17430,
    SPELL_SUMMON_ANUB_WARRIOR    = 17431,

    EMOTE_FRENZY                 = -1000002
};

struct MANGOS_DLL_DECL mob_anubisath_guardianAI : public ScriptedAI
{
    mob_anubisath_guardianAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    bool m_bIsEnraged;

    uint32 m_uiSummonCount;

    uint32 m_uiSpell1;
    uint32 m_uiSpell2;
    uint32 m_uiSpell3;
    uint32 m_uiSpell4;
    uint32 m_uiSpell5;

    uint32 m_uiSpell1Timer;
    uint32 m_uiSpell2Timer;
    uint32 m_uiSpell5Timer;

    void Reset()
    {
        m_bIsEnraged = false;

        m_uiSummonCount = 0;

        m_uiSpell1 = urand(0,1) ? SPELL_METEOR : SPELL_PLAGUE;
        m_uiSpell2 = urand(0,1) ? SPELL_SHADOW_STORM : SPELL_THUNDER_CLAP;
        m_uiSpell3 = urand(0,1) ? SPELL_REFLECT_ARFR : SPELL_REFLECT_FSSH;
        m_uiSpell4 = urand(0,1) ? SPELL_ENRAGE : SPELL_EXPLODE;
        m_uiSpell5 = urand(0,1) ? SPELL_SUMMON_ANUB_SWARMGUARD : SPELL_SUMMON_ANUB_WARRIOR;

        m_uiSpell1Timer = 10000;
        m_uiSpell2Timer = 20000;
        m_uiSpell5Timer = 10000;
    }

    void Aggro(Unit* /*pWho*/)
    {
        // spell reflection
        DoCastSpellIfCan(m_creature, m_uiSpell3);
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(m_creature->getVictim());
        ++m_uiSummonCount;
    }

    void SummonedCreatureDespawn(Creature* pDespawned)
    {
        --m_uiSummonCount;
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        // when we reach 10% of HP explode or enrage
        if (!m_bIsEnraged && HealthBelowPct(10))
        {
            if (m_uiSpell4 == SPELL_ENRAGE)
            {
                DoScriptText(EMOTE_FRENZY, m_creature);
                DoCastSpellIfCan(m_creature, m_uiSpell4);
                m_bIsEnraged = true;
            }
            else
                DoCastSpellIfCan(m_creature->getVictim(), m_uiSpell4);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Meteor or Plague
        if (m_uiSpell1Timer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), m_uiSpell1);
            m_uiSpell1Timer = 15000;
        }
        else
            m_uiSpell1Timer -= uiDiff;

        // Shadow Storm or Thunder Clap
        if (m_uiSpell2Timer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), m_uiSpell2);
            m_uiSpell2Timer = 15000;
        }
        else
            m_uiSpell2Timer -= uiDiff;

        // summon Anubisath Swarmguard or Anubisath Warrior
        if (m_uiSpell5Timer <= uiDiff)
        {
            // change for summon spell
            if (m_uiSummonCount < 4)
                DoCastSpellIfCan(m_creature->getVictim(), m_uiSpell5);

            m_uiSpell5Timer = 15000;
        }
        else
            m_uiSpell5Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_anubisath_guardian(Creature* pCreature)
{
    return new mob_anubisath_guardianAI(pCreature);
}

/*######
## mob_flesh_hunter
######*/

enum eFleshHunter
{
    SPELL_CONSUME       = 25371,
    SPELL_CONSUME_DMG   = 25373,
    SPELL_CONSUME_HEAL  = 25378,
    SPELL_POISON_BOLT   = 25424,
    SPELL_TRASH         = 3391,
};

struct MANGOS_DLL_DECL mob_flesh_hunterAI : public ScriptedAI
{
    mob_flesh_hunterAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    bool m_bPlayerConsumed;

    uint32 m_uiPoisonBoltTimer;
    uint32 m_uiTrashTimer;
    uint32 m_uiConsumeTimer;
    uint32 m_uiConsumeDamageTimer;
    
    ObjectGuid m_uiConsumeVictim;

    void Reset() 
    {
        m_bPlayerConsumed = false;

        m_uiPoisonBoltTimer = 10000;
        m_uiTrashTimer = 15000;
        m_uiConsumeTimer = 30000;
        m_uiConsumeDamageTimer = 1000;

		m_uiConsumeVictim.Clear();
    }

    void KilledUnit(Unit* pWho)
    {
        if (pWho->GetObjectGuid() == m_uiConsumeVictim)
            DoCastSpellIfCan(m_creature, SPELL_CONSUME_HEAL);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiPoisonBoltTimer <= uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            DoCastSpellIfCan(pTarget ? pTarget : m_creature->getVictim(), SPELL_POISON_BOLT);
            m_uiPoisonBoltTimer = urand(5000,10000);
        }
        else
            m_uiPoisonBoltTimer -= uiDiff;

        if (m_uiConsumeTimer <= uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                DoCastSpellIfCan(pTarget, SPELL_CONSUME);
				m_uiConsumeVictim = pTarget->GetObjectGuid();
                DoTeleportPlayer(m_creature->getVictim(), m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0);
                m_bPlayerConsumed = true;
            }
            m_uiConsumeTimer = 30000;
        }
        else
            m_uiConsumeTimer -= uiDiff;

        if (Unit* pConsumeTarget = m_creature->GetMap()->GetUnit(m_uiConsumeVictim))
            if (pConsumeTarget->HasAura(SPELL_CONSUME, EFFECT_INDEX_0))
            {
                if (m_uiConsumeDamageTimer <= uiDiff)
                {
                    DoCastSpellIfCan(pConsumeTarget, SPELL_CONSUME_DMG);
                    m_uiConsumeDamageTimer = 1000;
                }
                else
                    m_uiConsumeDamageTimer -= uiDiff;
            }

        if (m_uiTrashTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_TRASH);
            m_uiTrashTimer = urand(5000,8000);
        }
        else
            m_uiTrashTimer -= uiDiff;

        DoMeleeAttackIfReady(); 
    }
};

CreatureAI* GetAI_mob_flesh_hunter(Creature* pCreature)
{
    return new mob_flesh_hunterAI(pCreature);
}

void AddSC_ruins_of_ahnqiraj()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "mob_anubisath_guardian";
    pNewscript->GetAI = &GetAI_mob_anubisath_guardian;
    pNewscript->RegisterSelf();
    
    pNewscript = new Script;
    pNewscript->Name = "mob_flesh_hunter";
    pNewscript->GetAI = &GetAI_mob_flesh_hunter;
    pNewscript->RegisterSelf();
}
