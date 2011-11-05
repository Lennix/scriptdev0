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
SDName: Boss_Jin'do the Hexxer
SD%Complete: 85
SDComment: Mind Control not working because of core bug. Shades visible for all.
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "zulgurub.h"

enum eJindo
{
    SAY_AGGRO                       = -1309021,

    SPELL_BANISH                    = 24466,
    SPELL_BRAINWASH_TOTEM           = 24262,
    SPELL_HEX                       = 17172,
    SPELL_DELUSIONS_OF_JINDO        = 24306,
    SPELL_SHADE_OF_JINDO_PASSIVE    = 24307,
    SPELL_SHADE_OF_JINDO            = 24308,    // not work by creature, but as player yes :?
    SPELL_POWERFULL_HEALING_WARD    = 24309,

    // Healing Ward
    SPELL_HEAL                      = 24311,

    // Brain Wash Totem
    SPELL_BRAIN_WASH                = 24261,

    // Shade of Jindo
    SPELL_SHADOW_SHOCK              = 24458
};

struct MANGOS_DLL_DECL boss_jindoAI : public ScriptedAI
{
    boss_jindoAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_zulgurub*)pCreature->GetInstanceData();
        Reset();
    }

    instance_zulgurub* m_pInstance;

    uint32 m_uiBrainWashTotemTimer;
    uint32 m_uiHealingWardTimer;
    uint32 m_uiHexTimer;
    uint32 m_uiDelusionsTimer;
    uint32 m_uiTeleportTimer;

    void Reset()
    {
        m_uiBrainWashTotemTimer = 20000;
        m_uiHealingWardTimer = 16000;
        m_uiHexTimer = 8000;
        m_uiDelusionsTimer = 10000;
        m_uiTeleportTimer = 5000;
    }

    void Aggro(Unit* /*pWho*/)
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Brain Wash Totem
        if (m_uiBrainWashTotemTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_BRAINWASH_TOTEM);
            m_uiBrainWashTotemTimer = urand(18000,26000);
        }
        else
            m_uiBrainWashTotemTimer -= uiDiff;

        // Healing Ward
        if (m_uiHealingWardTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_POWERFULL_HEALING_WARD);
            //DoSpawnCreature(NPC_POWERFUL_HEALING_WARD, irand(-3,3), irand(-3,3), 0, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
            m_uiHealingWardTimer = urand(14000,20000);
        }
        else
            m_uiHealingWardTimer -= uiDiff;

        // Hex
        if (m_uiHexTimer <= uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                DoCastSpellIfCan(pTarget, SPELL_HEX);

                if (m_creature->getThreatManager().getThreat(pTarget))
                    m_creature->getThreatManager().modifyThreatPercent(pTarget,-80);
            }

            m_uiHexTimer = urand(12000,16000); // 12000, 20000
        }
        else
            m_uiHexTimer -= uiDiff;

        // Casting the delusion curse with a shade. So shade will attack the same target with the curse.
        if (m_uiDelusionsTimer <= uiDiff)
        {
            // Summon 2 illusions
            for(uint8 i = 0; i < 2; ++i)
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    DoCastSpellIfCan(pTarget, SPELL_DELUSIONS_OF_JINDO);
                    //DoCastSpellIfCan(m_creature, SPELL_SHADE_OF_JINDO);     // not work
                    if (Creature* pShade = m_creature->SummonCreature(NPC_SHADE_OF_JINDO, pTarget->GetPositionX()-5, pTarget->GetPositionY()+5, pTarget->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000))
                        pShade->AI()->AttackStart(pTarget);
                }

            m_uiDelusionsTimer = urand(4000,12000);
        }
        else
            m_uiDelusionsTimer -= uiDiff;

        // Teleporting a random gamer and spawning 9 skeletons that will attack this player
        if (m_uiTeleportTimer <= uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (pTarget && pTarget->GetTypeId() == TYPEID_PLAYER)
            {
                //DoCastSpellIfCan(pTarget, SPELL_BANISH);
                DoTeleportPlayer(pTarget, -11583.7783f, -1249.4278f, 77.5471f, 4.745f);

                if (m_creature->getThreatManager().getThreat(pTarget))
                    m_creature->getThreatManager().modifyThreatPercent(pTarget,-100);

                float fX, fY, fZ;
                pTarget->GetPosition(fX, fY, fZ);
                for(uint32 i = 0; i < 9; ++i)
                {
                    if (Creature* Skeleton = m_creature->SummonCreature(NPC_SACRIFICED_TROLL, fX+irand(-4,4), fY+irand(-4,4), fZ, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000))
                        Skeleton->AI()->AttackStart(pTarget);
                }
            }
            m_uiTeleportTimer = urand(15000,23000);
        }
        else
            m_uiTeleportTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_jindo(Creature* pCreature)
{
    return new boss_jindoAI(pCreature);
}

struct MANGOS_DLL_DECL mob_healing_wardAI : public ScriptedAI
{
    mob_healing_wardAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_zulgurub*)pCreature->GetInstanceData();
        Reset();
    }

    instance_zulgurub* m_pInstance;

    uint32 m_uiHealTimer;

    void Reset()
    {
        m_uiHealTimer = 2000;
        m_creature->addUnitState(UNIT_STAT_CAN_NOT_MOVE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Heal spell
        if (m_uiHealTimer < uiDiff)
        {
            if (m_pInstance)
            {
                Unit* pJindo = m_pInstance->GetSingleCreatureFromStorage(NPC_JINDO);
                if (pJindo && pJindo->isAlive())
                    DoCastSpellIfCan(pJindo, SPELL_HEAL);
            }

            m_uiHealTimer = 3000;
        }
        else
            m_uiHealTimer -= uiDiff;
    }
};

CreatureAI* GetAI_mob_healing_ward(Creature* pCreature)
{
    return new mob_healing_wardAI(pCreature);
}

struct MANGOS_DLL_DECL mob_brain_wash_totemAI : public ScriptedAI
{
    mob_brain_wash_totemAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_zulgurub*)pCreature->GetInstanceData();
        pHoldedTarget = 0;
        bSpellCasted = false;
        Reset();
    }

    instance_zulgurub* m_pInstance;
    Unit* pHoldedTarget;
    bool bSpellCasted;

    void Reset()
    {
        m_creature->addUnitState(UNIT_STAT_CAN_NOT_MOVE);
    }

    void DamageTaken(Unit *, uint32)
    {
        // Totem will always looking at charmed target
        if (pHoldedTarget)
            m_creature->SetTargetGuid(pHoldedTarget->GetObjectGuid());
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Brain Wash spell
        if (!bSpellCasted && m_pInstance)
        {
            if (Creature* pJindo = m_pInstance->GetSingleCreatureFromStorage(NPC_JINDO))
                if (Unit* pHoldedTarget = pJindo->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                {
                    DoCastSpellIfCan(pHoldedTarget, SPELL_BRAIN_WASH, CAST_AURA_NOT_PRESENT);
                    bSpellCasted = true;
                }
        }
    }
};

CreatureAI* GetAI_mob_brain_wash_totem(Creature* pCreature)
{
    return new mob_brain_wash_totemAI(pCreature);
}

struct MANGOS_DLL_DECL mob_shade_of_jindoAI : public ScriptedAI
{
    mob_shade_of_jindoAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiShadowShockTimer;
    uint32 m_uiInvisibilityTimer;

    void Reset()
    {
        m_uiShadowShockTimer = 2000;
        m_uiInvisibilityTimer = 0;
        DoCastSpellIfCan(m_creature, SPELL_SHADE_OF_JINDO_PASSIVE, CAST_FORCE_CAST + CAST_FORCE_TARGET_SELF + CAST_TRIGGERED);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Invisibility (custom name)
        if (m_uiInvisibilityTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_SHADE_OF_JINDO_PASSIVE, CAST_TRIGGERED + CAST_AURA_NOT_PRESENT);
            m_uiInvisibilityTimer = 200;
        }
        else
            m_uiInvisibilityTimer -= uiDiff;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Shadow Shock
        if (m_uiShadowShockTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_SHOCK);
            m_uiShadowShockTimer = 2000;
        }
        else
            m_uiShadowShockTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_shade_of_jindo(Creature* pCreature)
{
    return new mob_shade_of_jindoAI(pCreature);
}

void AddSC_boss_jindo()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_jindo";
    pNewScript->GetAI = &GetAI_boss_jindo;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_healing_ward";
    pNewScript->GetAI = &GetAI_mob_healing_ward;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_brain_wash_totem";
    pNewScript->GetAI = &GetAI_mob_brain_wash_totem;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_shade_of_jindo";
    pNewScript->GetAI = &GetAI_mob_shade_of_jindo;
    pNewScript->RegisterSelf();
}
