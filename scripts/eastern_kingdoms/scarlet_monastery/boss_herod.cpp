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
SDName: Boss_Herod
SD%Complete: 95
SDComment: Should in addition spawn Myrmidons in the hallway outside
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"

enum eHerod
{
    SAY_AGGRO                   = -1189000,
    SAY_WHIRLWIND               = -1189001,
    SAY_ENRAGE                  = -1189002,
    SAY_KILL                    = -1189003,
    EMOTE_ENRAGE                = -1189004,

    SPELL_RUSHING_CHARGE        = 8260,
    SPELL_CLEAVE                = 15496,
    SPELL_WHIRLWIND             = 8989,
    SPELL_FRENZY                = 8269,

    NPC_SCARLET_TRAINEE       = 6575,
    NPC_SCARLET_MYRMIDON      = 4295,
};

struct MANGOS_DLL_DECL boss_herodAI : public ScriptedAI
{
    boss_herodAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    bool m_bEnrage;
    uint32 m_uiCleaveTimer;
    uint32 m_uiWhirlwindTimer;

    void Reset()
    {
        m_bEnrage = false;
        m_uiCleaveTimer = 12000;
        m_uiWhirlwindTimer = 45000;
    }

    void Aggro(Unit* /*pWho*/)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_RUSHING_CHARGE);
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        // TODO: Play sound "/script PlaySoundFile("Sound\\Creature\\ScarletTrainee\\ScarletTraineeSpawn01.wav")"
        for(uint8 i = 0; i < 20; ++i)
        {
            float x,y,z;
            m_creature->GetPosition(x,y,z);
            if (Creature* pSummon = m_creature->SummonCreature(NPC_SCARLET_TRAINEE, 1939.18f, -431.58f, 17.09f, 6.22f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000))
            {
                CreatureCreatePos pos(pSummon->GetMap(), x, y, z, 0.0f);
                pSummon->SetSummonPoint(pos);
            }
        }
    }

    void JustSummoned(Unit* pSummoned)
    {
        m_creature->MonsterSay("JustSummoned for %t", LANG_UNIVERSAL, pSummoned);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // If we are <30% hp goes enraged
        if (!m_bEnrage && HealthBelowPct(30) && !m_creature->IsNonMeleeSpellCasted(false))
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_ENRAGE, m_creature);
                DoScriptText(SAY_ENRAGE, m_creature);
                m_bEnrage = true;
            }
        }

        // Cleave
        if (m_uiCleaveTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE);
            m_uiCleaveTimer = 12000;
        }
        else 
            m_uiCleaveTimer -= uiDiff;

        // Whirlwind
        if (m_uiWhirlwindTimer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WHIRLWIND) == CAST_OK)
            {
                DoScriptText(SAY_WHIRLWIND, m_creature);
                m_uiWhirlwindTimer = 30000;
            }
        }
        else 
            m_uiWhirlwindTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_herod(Creature* pCreature)
{
    return new boss_herodAI(pCreature);
}

struct MANGOS_DLL_DECL mob_scarlet_traineeAI : public npc_escortAI
{
    mob_scarlet_traineeAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiStartTimer = urand(1000,6000);
        m_uiRandomMoveTimer = urand(1000,2000);
        m_bEscortCompleted = false;
        Reset();
    }

    uint32 m_uiStartTimer;
    uint32 m_uiRandomMoveTimer;
    bool m_bEscortCompleted;

    void Reset() {}
    void WaypointReached(uint32 uiPoint)
    {
        if (uiPoint == 11)
        {
            m_bEscortCompleted = true;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (m_uiStartTimer)
        {
            if (m_uiStartTimer <= uiDiff)
            {
                Start(true);
                m_uiStartTimer = 0;
            }
            else 
                m_uiStartTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            if (m_bEscortCompleted)
            {
                if (m_uiRandomMoveTimer <= uiDiff)
                {
                    m_creature->GetMotionMaster()->MoveRandom();
                    m_uiRandomMoveTimer = urand(2500,4500);
                }
                else
                    m_uiRandomMoveTimer -= uiDiff;
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_scarlet_trainee(Creature* pCreature)
{
    return new mob_scarlet_traineeAI(pCreature);
}

void AddSC_boss_herod()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_herod";
    pNewScript->GetAI = &GetAI_boss_herod;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_scarlet_trainee";
    pNewScript->GetAI = &GetAI_mob_scarlet_trainee;
    pNewScript->RegisterSelf();
}
