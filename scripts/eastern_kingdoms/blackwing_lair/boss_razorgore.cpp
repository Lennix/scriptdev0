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
SDName: Boss_Razorgore
SD%Complete: 75
SDComment: NOTE THAT THESE SCRIPT ISNT USED WHILE WE CONTROLLING HIM !!!    
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "blackwing_lair.h"

enum eRazorgore
{
    SAY_EGGS_BROKEN1        = -1469022,
    SAY_EGGS_BROKEN2        = -1469023,
    SAY_EGGS_BROKEN3        = -1469024,
    SAY_DEATH               = -1469025,

    // Razorgore spells
    SPELL_CLEAVE            = 19632,
    SPELL_CONFLAGRATION     = 23023,
    SPELL_FIREBALL_VOLLEY   = 22425,
    SPELL_WAR_STOMP         = 24375,
    SPELL_WARMING_FLAMES    = 23040,

    // Grethok
    SPELL_ARCANE_MISSILES   = 22273,
    SPELL_DOMINATE_MIND     = 14515,
    SPELL_GREATER_POLYMORPH = 22274,
    SPELL_SLOW              = 13747,

    // Orb of Domination spells
    SPELL_DRAGON_ORB        = 19869,
    SPELL_MIND_EXHAUSTION   = 23958,
};

struct MANGOS_DLL_DECL boss_razorgoreAI : public ScriptedAI
{
    boss_razorgoreAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackwing_lair*)pCreature->GetInstanceData();
        Reset();
    }

    instance_blackwing_lair* m_pInstance;

    bool PHASE_TWO;

    uint32 m_uiCheckControllerAggroTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiConflagrationTimer;
    uint32 m_uiFireballVolleyTimer;
    uint32 m_uiWarStompTimer;

    ObjectGuid m_uiControllerGUID;

    void Reset()
    {
        PHASE_TWO = false;

        m_uiCheckControllerAggroTimer = 3000;
        m_uiCleaveTimer = 15000;
        m_uiConflagrationTimer = 12000;
        m_uiFireballVolleyTimer = 7000;
        m_uiWarStompTimer = 35000;

		m_uiControllerGUID.Clear();
    }

    /* DATA RAZORGORE
     * ==============
     * FAIL         = Wipe
     * IN_PROGRESS  = Phase 1
     * SPECIAL      = Phase 2
     * DONE         = Razorgore is defeated
     */

    void Aggro(Unit* /*pWho*/)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZORGORE, IN_PROGRESS);

        //taunt immune
        m_creature->CastSpell(m_creature, 20027, true);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZORGORE, FAIL);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (PHASE_TWO)
        {
            DoScriptText(SAY_DEATH, m_creature);
            if (m_pInstance)
                m_pInstance->SetData(TYPE_RAZORGORE, DONE);
        }
        else
        {
            // TODO: we should kill whole raid -> find proper spell for that
            m_creature->Respawn();

            Map* pMap = m_creature->GetMap();
            if (!pMap || !pMap->IsRaid())
                return;

            Map::PlayerList const& PlayerList = pMap->GetPlayers();

            if (PlayerList.isEmpty())
                return;

            for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
            {
                Player* pPlayer = itr->getSource();

                if (pPlayer && pPlayer->isAlive())
                    m_creature->DealDamage(pPlayer, pPlayer->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

		if (((Player*)pVictim)->GetObjectGuid() == m_uiControllerGUID)
			m_uiControllerGUID.Clear();
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (pSpell->Id != SPELL_USE_DRAGON_ORB || pCaster->GetTypeId() != TYPEID_PLAYER)
            return;

        // Drop threat for previous controller
        if (Player* pPrevController = m_creature->GetMap()->GetPlayer(m_uiControllerGUID))
            if (m_creature->getThreatManager().getThreat(pPrevController))
                m_creature->getThreatManager().modifyThreatPercent(pPrevController,-100);

        // Add huge amount of threat for new controller
        m_creature->AddThreat(pCaster, 10000.0f);
        m_uiControllerGUID = ((Player*)pCaster)->GetObjectGuid();
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (PHASE_TWO)
            return;

        if (m_pInstance && m_pInstance->GetData(TYPE_RAZORGORE) == SPECIAL)
        {
            PHASE_TWO = true;
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_WARMING_FLAMES, CAST_TRIGGERED);
            return;
        }
    }

    void CheckControllerAggro(Unit* pVictim)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

        Player* pPlayer = (Player*)pVictim;

        if (pPlayer->GetObjectGuid() == m_uiControllerGUID)
            return;

        if (Player* pController = m_creature->GetMap()->GetPlayer(m_uiControllerGUID))
        {
            float m_fThreat = m_creature->getThreatManager().getThreat(pPlayer);
            m_creature->getThreatManager().addThreat(pController, m_fThreat*10);
            m_creature->TauntApply(pController);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // If last controller is alive he should always have aggro until he dies
        if (m_uiControllerGUID)
        {
            if (m_uiCheckControllerAggroTimer <= uiDiff)
            {
                CheckControllerAggro(m_creature->getVictim());
                m_uiCheckControllerAggroTimer = 3000;
            }
            else
                m_uiCheckControllerAggroTimer -= uiDiff;
        }

        // Cleave
        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(7000, 10000);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        // Conflagration
        if (m_uiConflagrationTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1);
            if (pTarget && DoCastSpellIfCan(m_creature->getVictim(), SPELL_CONFLAGRATION) == CAST_OK)
            {
                if (m_creature->getThreatManager().getThreat(m_creature->getVictim()))
                    m_creature->getThreatManager().modifyThreatPercent(m_creature->getVictim(),-50);

                m_creature->TauntApply(pTarget);
                m_uiConflagrationTimer = 12000;
            }
        }
        else
            m_uiConflagrationTimer -= uiDiff;

        // Fireball Volley
        if (m_uiFireballVolleyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FIREBALL_VOLLEY) == CAST_OK)
                m_uiFireballVolleyTimer = urand(12000, 15000);
        }
        else
            m_uiFireballVolleyTimer -= uiDiff;

        // War Stomp
        if (m_uiWarStompTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WAR_STOMP) == CAST_OK)
                m_uiWarStompTimer = urand(15000, 25000);
        }
        else
            m_uiWarStompTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_razorgore(Creature* pCreature)
{
    return new boss_razorgoreAI(pCreature);
}

struct MANGOS_DLL_DECL mob_grethok_the_controllerAI : public ScriptedAI
{
    mob_grethok_the_controllerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackwing_lair*)pCreature->GetInstanceData();
        Reset();
    }

    instance_blackwing_lair* m_pInstance;

    uint32 m_uiArcaneMisslesTimer;
    uint32 m_uiDominateMindTimer;
    uint32 m_uiGreaterPolymorphTimer;
    uint32 m_uiSlowTimer;

    void Reset()
    {
        m_uiArcaneMisslesTimer = 2000;
        m_uiDominateMindTimer = 8000;
        m_uiGreaterPolymorphTimer = 18000;
        m_uiSlowTimer = 13000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Arcane Missiles
        if (m_uiArcaneMisslesTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ARCANE_MISSILES) == CAST_OK)
                m_uiArcaneMisslesTimer = urand(7000, 10000);
        }
        else
            m_uiArcaneMisslesTimer -= uiDiff;

        // Dominate Mind
        if (m_uiDominateMindTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (DoCastSpellIfCan(pTarget ? pTarget : m_creature->getVictim(), SPELL_DOMINATE_MIND) == CAST_OK)
                m_uiDominateMindTimer = urand(18000, 30000);
        }
        else
            m_uiDominateMindTimer -= uiDiff;

        // Greater Polymorph
        if (m_uiGreaterPolymorphTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (DoCastSpellIfCan(pTarget ? pTarget : m_creature->getVictim(), SPELL_GREATER_POLYMORPH) == CAST_OK)
                m_uiGreaterPolymorphTimer = urand(15000, 25000);
        }
        else
            m_uiGreaterPolymorphTimer -= uiDiff;

        // Slow
        if (m_uiSlowTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SLOW) == CAST_OK)
                m_uiSlowTimer = urand(10000, 15000);
        }
        else
            m_uiSlowTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_grethok_the_controller(Creature* pCreature)
{
    return new mob_grethok_the_controllerAI(pCreature);
}

/*####
## go_orb_of_domination
####*/

bool GOUse_go_orb_of_domination(Player* pPlayer, GameObject* pGo)
{
    instance_blackwing_lair* m_pInstance = (instance_blackwing_lair*)pGo->GetInstanceData();

    if (m_pInstance)
    {
        if (m_pInstance->GetData(TYPE_RAZORGORE) != IN_PROGRESS)
            return true;

        if (Creature* pRazorgore = m_pInstance->GetSingleCreatureFromStorage(NPC_RAZORGORE))
        {
            if (pRazorgore->IsCharmerOrOwnerPlayerOrPlayerItself())
                return true;
        }
    }
       
    if (!pPlayer->HasAura(SPELL_MIND_EXHAUSTION, EFFECT_INDEX_0))
        pPlayer->CastSpell(pPlayer, SPELL_USE_DRAGON_ORB, true);

    return true;
}

void AddSC_boss_razorgore()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_razorgore";
    pNewScript->GetAI = &GetAI_boss_razorgore;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_grethok_the_controller";
    pNewScript->GetAI = &GetAI_mob_grethok_the_controller;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_orb_of_domination";
    pNewScript->pGOUse = &GOUse_go_orb_of_domination;
    pNewScript->RegisterSelf();
}
