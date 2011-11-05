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
SDName: Blackwing_Lair
SD%Complete: 10
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

/* ContentData
mob_demon_portal
EndContentData */

#include "precompiled.h"
#include "blackwing_lair.h"

/*######
## mob_demon_portal
######*/

enum eDemonPortal
{
    NPC_DEMONIC_MINION              = 12422,        // probably wrong creature entry
    SPELL_SUMMON_DEMONIC_MINION     = 19828,        // probably wrong spell entry
};

struct MANGOS_DLL_DECL mob_demon_portalAI : public ScriptedAI
{
    mob_demon_portalAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_creature->addUnitState(UNIT_STAT_ROOT);
        m_creature->addUnitState(UNIT_STAT_CAN_NOT_MOVE);
        m_uiDespawnSelfTimer = 11000;
        Reset();
    }

    uint32 m_uiSpawnDemonTimer;
    uint32 m_uiDespawnSelfTimer;

    void Reset()
    {
        m_uiSpawnDemonTimer = 11000;
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (Unit* pOwner = m_creature->GetMap()->GetUnit(m_creature->GetOwnerGuid()))
        {
            pSummoned->SetOwnerGuid(pOwner->GetObjectGuid());
            pSummoned->SetCharmerGuid(pOwner->GetObjectGuid());
            pSummoned->setFaction(pOwner->getFaction());
            pSummoned->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
            pSummoned->GetMotionMaster()->MoveFollow(pOwner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        }
        else
            pSummoned->setFaction(m_creature->getFaction());
    }

    void UpdateAI(const uint32 uiDiff)
    {
        // Close portal (despawn) after 35 seconds
        if (m_uiDespawnSelfTimer <= uiDiff)
        {
            m_uiDespawnSelfTimer = 0;
            m_creature->ForcedDespawn();
        }

        // Demon Spawn
        if (m_uiSpawnDemonTimer <= uiDiff)
        {
            /*if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_DEMONIC_MINION) == CAST_OK)
                m_uiSpawnDemonTimer = 11000;*/
            Position pos;
            m_creature->GetClosePoint(pos.x, pos.y, pos.z, 0);
            m_creature->SummonCreature(NPC_DEMONIC_MINION, pos.x, pos.y, pos.z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
            m_uiSpawnDemonTimer = 11000;
        }
        else
            m_uiSpawnDemonTimer -= uiDiff;

        // no meele, it is a portal..
    }
};

CreatureAI* GetAI_mob_demon_portal(Creature* pCreature)
{
    return new mob_demon_portalAI(pCreature);
}

/*######
## mob_suppression_trigger
######*/

enum eSuppressionTrigger
{
    SPELL_SUPPRESSION_AURA      = 22247
};

struct MANGOS_DLL_DECL mob_suppression_triggerAI : public Scripted_NoMovementAI
{
    mob_suppression_triggerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_creature->setFaction(103);
        m_creature->SetVisibility(VISIBILITY_OFF);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
        pDevice = NULL;
        Reset();
    }

	GameObject* pDevice;
	uint32 SuppressionTimer;
	uint32 ActivationTimer;

	void Reset()
    {
        SuppressionTimer = urand(1000,2000);
        ActivationTimer = urand(15000, 30000/*DEVICE_RESPAWN*/);
	}

	void UpdateAI(const uint32 uiDiff)
    {
        if (!pDevice)
            pDevice = GetClosestGameObjectWithEntry(m_creature, GO_SUPPRESSION_DEVICE, 5.0f);

        if (pDevice && pDevice->GetGoState() == GO_STATE_ACTIVE)
        {
            if (ActivationTimer < uiDiff)
            {
                ActivationTimer = urand(15000, 30000/*DEVICE_RESPAWN*/);
                pDevice->SetGoState(GO_STATE_READY);
            }
            else
                ActivationTimer -= uiDiff;

            return;
        }

        if (SuppressionTimer < uiDiff)
        {
            SuppressionTimer = urand(1000,2000);
            DoCastSpellIfCan(m_creature, SPELL_SUPPRESSION_AURA);
        }
        else
            SuppressionTimer -= uiDiff;
	}
};

CreatureAI* GetAI_mob_suppression_trigger(Creature* pCreature)
{
    return new mob_suppression_triggerAI(pCreature);
}

/* AddSC */

void AddSC_blackwing_lair()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "mob_demon_portal";
    pNewScript->GetAI = &GetAI_mob_demon_portal;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_suppression_trigger";
    pNewScript->GetAI = &GetAI_mob_suppression_trigger;
    pNewScript->RegisterSelf();
}
