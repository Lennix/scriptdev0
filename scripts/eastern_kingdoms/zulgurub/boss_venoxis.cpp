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
SDName: Boss_Venoxis
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "zulgurub.h"

enum eVenoxis
{
    SAY_TRANSFORM           = -1309003,
    SAY_DEATH               = -1309004,

    SPELL_DISPELL           = 23859,
    SPELL_FRENZY            = 8269,
    SPELL_HOLY_FIRE         = 23860,
    SPELL_HOLY_NOVA         = 23858,
    SPELL_HOLY_WRATH        = 23979,
    SPELL_PARASITIC_SERPENT = 23865,
    SPELL_POISON_CLOUD      = 23861,
    SPELL_RENEW             = 23895,
    SPELL_TRASH             = 3391,
    SPELL_VENOM_SPIT        = 23862,
    SPELL_VENOXIS_TRANSFORM = 23849,
};

struct MANGOS_DLL_DECL boss_venoxisAI : public ScriptedAI
{
    boss_venoxisAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_zulgurub*)pCreature->GetInstanceData();
        Reset();
    }

    instance_zulgurub* m_pInstance;

    bool m_bFrenzied;
    bool m_bPhaseTwo;

    uint32 m_uiHolyFireTimer;
    uint32 m_uiHolyWrathTimer;
    uint32 m_uiVenomSpitTimer;
    uint32 m_uiRenewTimer;
    uint32 m_uiPoisonCloudTimer;
    uint32 m_uiHolyNovaTimer;
    uint32 m_uiDispellTimer;
    uint32 m_uiParasiticTimer;
    uint32 m_uiTrashTimer;

    uint8 m_uiTargetsInRangeCount;

    void Reset()
    {
        m_bFrenzied = false;
        m_bPhaseTwo = false;

        m_uiHolyFireTimer = 10000;
        m_uiHolyWrathTimer = 60500;
        m_uiVenomSpitTimer = 5500;
        m_uiRenewTimer = 30500;
        m_uiPoisonCloudTimer = 2000;
        m_uiHolyNovaTimer = 5000;
        m_uiDispellTimer = 35000;
        m_uiParasiticTimer = 10000;
        m_uiTrashTimer = 5000;

        m_uiTargetsInRangeCount = 0;

        m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_VENOXIS, NOT_STARTED);

        std::list<Creature*> m_lCobras;
        GetCreatureListWithEntryInGrid(m_lCobras, m_creature, NPC_RAZZASHI_COBRA, DEFAULT_VISIBILITY_INSTANCE);

        if (m_lCobras.empty())
            debug_log("SD2: ZG: boss_venoxis, no Cobras with the entry %u were found", NPC_RAZZASHI_COBRA);
        else
        {
            for(std::list<Creature*>::iterator itr = m_lCobras.begin(); itr != m_lCobras.end(); ++itr)
            {
                if ((*itr) && !(*itr)->isAlive())
                    (*itr)->Respawn();
            }
        }
    }

    void Aggro(Unit* /*pWho*/)
    {
        m_creature->CallForHelp(25.f);
        if (m_pInstance)
            m_pInstance->SetData(TYPE_VENOXIS, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_VENOXIS, DONE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Start Phase Two
        if (!m_bPhaseTwo && HealthBelowPct(50))
        {
            DoScriptText(SAY_TRANSFORM, m_creature);
            DoCastSpellIfCan(m_creature, SPELL_VENOXIS_TRANSFORM, CAST_INTERRUPT_PREVIOUS + CAST_FORCE_TARGET_SELF);

            m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
            const CreatureInfo* cinfo = m_creature->GetCreatureInfo();
            m_creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 25)));
            m_creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 25)));
            m_creature->UpdateDamagePhysical(BASE_ATTACK);
            DoResetThreat();
            m_bPhaseTwo = true;
        }

        // Frenzy if health or mana below 15%
        if (m_bPhaseTwo && !m_bFrenzied && (HealthBelowPct(15) || (m_creature->GetPower(POWER_MANA)*100/m_creature->GetMaxPower(POWER_MANA) <= 15)))
        {
            DoCastSpellIfCan(m_creature, SPELL_FRENZY, CAST_INTERRUPT_PREVIOUS + CAST_FORCE_TARGET_SELF);
            m_bFrenzied = true;
        }

        if (!m_bPhaseTwo)
        {
            if (m_uiDispellTimer <= uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_DISPELL);
                m_uiDispellTimer = urand(15000, 30000);
            }
            else
                m_uiDispellTimer -= uiDiff;

            if (m_uiRenewTimer <= uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_RENEW);
                m_uiRenewTimer = urand(15000, 30000);
            }
            else
                m_uiRenewTimer -= uiDiff;

            if (m_uiHolyWrathTimer <= uiDiff)
            {
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                DoCastSpellIfCan(pTarget ? pTarget : m_creature->getVictim(), SPELL_HOLY_WRATH);
                m_uiHolyWrathTimer = urand(8000, 10000);       // 15000, 25000
            }
            else
                m_uiHolyWrathTimer -= uiDiff;

            if (m_uiHolyNovaTimer <= uiDiff)
            {
                m_uiTargetsInRangeCount = 0;
                for(uint8 i = 0; i < 10; ++i)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO,i))
                        if (m_creature->CanReachWithMeleeAttack(pTarget))
                            ++m_uiTargetsInRangeCount;
                }

                if (m_uiTargetsInRangeCount > 1)
                {
                    DoCastSpellIfCan(m_creature->getVictim(), SPELL_HOLY_NOVA);
                    m_uiHolyNovaTimer = 1000;
                }
                else
                    m_uiHolyNovaTimer = 2000;
            }
            else
                m_uiHolyNovaTimer -= uiDiff;

            if (m_uiHolyFireTimer <= uiDiff && m_uiTargetsInRangeCount < 3)
            {
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0);
                DoCastSpellIfCan(pTarget ? pTarget : m_creature->getVictim(), SPELL_HOLY_FIRE);

                m_uiHolyFireTimer = 3000;       // 8000
            }
            else
                m_uiHolyFireTimer -= uiDiff;
        }
        else
        {
            // Poison Cloud
            if (m_uiPoisonCloudTimer <= uiDiff)
            {
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_POISON_CLOUD);
                m_uiPoisonCloudTimer = urand(5000,8000);        // 10000
            }
            else
                m_uiPoisonCloudTimer -= uiDiff;

            // Venom Spit
            if (m_uiVenomSpitTimer <= uiDiff)
            {
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                DoCastSpellIfCan(pTarget ? pTarget : m_creature->getVictim(), SPELL_VENOM_SPIT);

                m_uiVenomSpitTimer = urand(5000,6000);       // 15000, 20000
            }
            else
                m_uiVenomSpitTimer -= uiDiff;

            // Parasitic Serpent
            if (m_uiParasiticTimer <= uiDiff)
            {
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0);
                DoCastSpellIfCan(pTarget ? pTarget : m_creature->getVictim(), SPELL_PARASITIC_SERPENT);

                m_uiParasiticTimer = 10000;
            }
            else
                m_uiParasiticTimer -= uiDiff;

            if (m_uiTrashTimer <= uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_TRASH);
                m_uiTrashTimer = urand(8000, 12000);
                
            }
            else
                m_uiTrashTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_venoxis(Creature* pCreature)
{
    return new boss_venoxisAI(pCreature);
}

/*######
## mob_razzashi_cobra
######*/

enum eRazzashiCobra
{
    SPELL_POISON = 24097,
    SPELL_SPIT   = 27919
};

struct MANGOS_DLL_DECL mob_razzashi_cobraAI : public ScriptedAI
{
    mob_razzashi_cobraAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_zulgurub*)pCreature->GetInstanceData();
        Reset();
    }

    instance_zulgurub* m_pInstance;

    uint32 m_uiPoisonTimer;
    uint32 m_uiSpitTimer;

    void Reset()
    {
        m_uiPoisonTimer = urand(2500,5000);
        m_uiSpitTimer = urand(0,2000);
    }

    void Aggro(Unit* pVictim)
    {
        m_creature->CallForHelp(25.0f);
    }

    void JustDied(Unit *)
    {
        // Increases Venoxis's damage by 15 percent
        if (m_pInstance)
        {
            if (Creature* pVenoxis = m_pInstance->GetSingleCreatureFromStorage(NPC_VENOXIS))
            {
                if (pVenoxis->isAlive() && m_pInstance->GetData(TYPE_VENOXIS) == IN_PROGRESS)
                {
                    uint8 m_uiRazzashiCobrasDeadCount = 0;
                    std::list<Creature*> m_lCobras;
                    GetCreatureListWithEntryInGrid(m_lCobras, m_creature, NPC_RAZZASHI_COBRA, DEFAULT_VISIBILITY_INSTANCE);

                    if (m_lCobras.empty())
                        debug_log("SD2: ZG: boss_venoxis, no Cobras with the entry %u were found", NPC_RAZZASHI_COBRA);
                    else
                    {
                        for(std::list<Creature*>::iterator itr = m_lCobras.begin(); itr != m_lCobras.end(); ++itr)
                        {
                            if ((*itr) && !(*itr)->isAlive())
                                ++m_uiRazzashiCobrasDeadCount;
                        }
                    }

                    const CreatureInfo* cinfo = pVenoxis->GetCreatureInfo();
                    pVenoxis->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg + (((cinfo->mindmg/100) * ((15 * m_uiRazzashiCobrasDeadCount)))));
                    pVenoxis->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg + (((cinfo->maxdmg/100) * ((15 * m_uiRazzashiCobrasDeadCount)))));
                    pVenoxis->UpdateDamagePhysical(BASE_ATTACK);
                }
            }
        }
        else
            debug_log("SD0: Zul'Gurub - Razzashi Cobra near boss Venoxis: m_pInstance NULL ! Unable increas Venoxis's damage.");
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Return if we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Poison
        if (m_uiPoisonTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_POISON);
            m_uiPoisonTimer = urand(7000,10000);
        }
        else
            m_uiPoisonTimer -= uiDiff;

        // Spit
        if (m_uiSpitTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_SPIT);
            m_uiSpitTimer = urand(7000,10000);
        }
        else
            m_uiSpitTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_razzashi_cobra(Creature* pCreature)
{
    return new mob_razzashi_cobraAI(pCreature);
}

void AddSC_boss_venoxis()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_venoxis";
    pNewScript->GetAI = &GetAI_boss_venoxis;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_razzashi_cobra";
    pNewScript->GetAI = &GetAI_mob_razzashi_cobra;
    pNewScript->RegisterSelf();
}
