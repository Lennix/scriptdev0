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
SDName: Boss_Tomb_Of_Seven
SD%Complete: 90
SDComment: Learning Smelt Dark Iron if tribute quest rewarded. Basic event implemented. Correct order and timing of event is unknown.
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"
#include "blackrock_depths.h"

enum
{
    FACTION_NEUTRAL             = 734,
    FACTION_HOSTILE             = 754,

    SPELL_CLEAVE                = 15284,
    //SPELL_CLEAVE                = 15496,
    SPELL_HAMSTRING             = 9080,
    SPELL_MORTAL_STRIKE         = 13737,
    SPELL_RECKLESSNESS          = 13847,

    SPELL_SMELT_DARK_IRON       = 14891,
    SPELL_LEARN_SMELT           = 14894,
    QUEST_SPECTRAL_CHALICE      = 4083,
    SKILLPOINT_MIN              = 230
};

struct MANGOS_DLL_DECL boss_gloomrelAI : public ScriptedAI
{
    boss_gloomrelAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData();
        Reset();
    }
    
    instance_blackrock_depths* m_pInstance;

    uint32 m_uiCleave_Timer;
    uint32 m_uiHamstring_Timer;
    uint32 m_uiMortalStrike_Timer;
    
    void Reset()
    {
    	m_uiCleave_Timer = urand(5000, 7000);
    	m_uiHamstring_Timer = urand(2000, 4000);
    	m_uiMortalStrike_Timer = urand(9000, 13000);
    }

  	void JustReachedHome()
  	{
        if (m_pInstance)
            m_pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);
  	}
    
    void UpdateAI(const uint32 uiDiff)
    {
    		//return since we have no target
    		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
    			return;

    		//m_uiCleave_Timer
        if (m_uiCleave_Timer <= uiDiff)
    	{
        DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE);
        m_uiCleave_Timer = urand(7000, 9000);
    	}
    	else
        m_uiCleave_Timer -= uiDiff;

        //m_uiHamstring_Timer
        if (m_uiHamstring_Timer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_HAMSTRING);
            m_uiHamstring_Timer = urand(15000, 18000);
        }
        else
            m_uiHamstring_Timer -= uiDiff;

        //m_uiMortalStrike_Timer
        if (m_uiMortalStrike_Timer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTAL_STRIKE);
            m_uiMortalStrike_Timer = urand(14000, 18000);
        }
        else
            m_uiMortalStrike_Timer -= uiDiff;

        //Cast Recklessness when health is below 30 percent
        if (HealthBelowPct(30))
            DoCastSpellIfCan(m_creature, SPELL_RECKLESSNESS, CAST_AURA_NOT_PRESENT);

        DoMeleeAttackIfReady();
	}
};
CreatureAI* GetAI_boss_gloomrel(Creature* pCreature)
{
    return new boss_gloomrelAI (pCreature);
}

#define GOSSIP_ITEM_TEACH_1 "Teach me the art of smelting dark iron"
#define GOSSIP_ITEM_TEACH_2 "Continue..."
#define GOSSIP_ITEM_TRIBUTE "I want to pay tribute"

bool GossipHello_boss_gloomrel(Player* pPlayer, Creature* pCreature)
{
    if (instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_TOMB_OF_SEVEN) == NOT_STARTED)
        {
            if (pPlayer->GetQuestRewardStatus(QUEST_SPECTRAL_CHALICE) &&
                pPlayer->GetSkillValue(SKILL_MINING) >= SKILLPOINT_MIN &&
                !pPlayer->HasSpell(SPELL_SMELT_DARK_IRON))
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TEACH_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if (!pPlayer->GetQuestRewardStatus(QUEST_SPECTRAL_CHALICE) &&
                pPlayer->GetSkillValue(SKILL_MINING) >= SKILLPOINT_MIN)
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TRIBUTE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        }
    }
	pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_gloomrel(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TEACH_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            pPlayer->SEND_GOSSIP_MENU(2606, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            pPlayer->CLOSE_GOSSIP_MENU();
            pCreature->CastSpell(pPlayer, SPELL_LEARN_SMELT, false);
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "[PH] Continue...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            pPlayer->SEND_GOSSIP_MENU(2604, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            pPlayer->CLOSE_GOSSIP_MENU();
            if (instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData())
            {
                //are 5 minutes expected? go template may have data to despawn when used at quest
				pInstance->DoRespawnGameObject(GO_SPECTRAL_CHALICE, 5*MINUTE);
            }
            break;
    }
    return true;
}

enum
{
    SPELL_SHADOWBOLTVOLLEY              = 15245,
    SPELL_IMMOLATE                      = 12742,
    SPELL_CURSEOFWEAKNESS               = 12493,
    SPELL_DEMONARMOR                    = 13787,
    SPELL_SUMMON_VOIDWALKERS            = 15092,

    SAY_DOOMREL_START_EVENT             = -1230003,

    MAX_DWARF                           = 7
};

#define GOSSIP_ITEM_CHALLENGE   "Your bondage is at an end, Doom'rel. I challenge you!"

struct MANGOS_DLL_DECL boss_doomrelAI : public ScriptedAI
{
    boss_doomrelAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData();
        Reset();
    }

    instance_blackrock_depths* m_pInstance;

    uint32 m_uiShadowVolley_Timer;
    uint32 m_uiImmolate_Timer;
    uint32 m_uiCurseOfWeakness_Timer;
    uint32 m_uiDemonArmor_Timer;
    uint32 m_uiCallToFight_Timer;
    uint8 m_uiDwarfRound;
    bool m_bHasSummoned;

    void Reset()
    {
        m_uiShadowVolley_Timer = 10000;
        m_uiImmolate_Timer = 18000;
        m_uiCurseOfWeakness_Timer = 5000;
        m_uiDemonArmor_Timer = 16000;
        m_uiCallToFight_Timer = 0;
        m_uiDwarfRound = 0;
        m_bHasSummoned = false;
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_TOMB_OF_SEVEN, FAIL);
    }

    void JustDied(Unit *victim)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_TOMB_OF_SEVEN, DONE);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
            pSummoned->AI()->AttackStart(pTarget);
    }

    Creature* GetDwarfForPhase(uint8 uiPhase)
    {
        switch(uiPhase)
        {
            case 0:
				return m_pInstance->GetSingleCreatureFromStorage(NPC_ANGERREL);
            case 1:
                return m_pInstance->GetSingleCreatureFromStorage(NPC_SEETHREL);
            case 2:
                return m_pInstance->GetSingleCreatureFromStorage(NPC_DOPEREL);
            case 3:
                return m_pInstance->GetSingleCreatureFromStorage(NPC_GLOOMREL);
            case 4:
                return m_pInstance->GetSingleCreatureFromStorage(NPC_VILEREL);
            case 5:
                return m_pInstance->GetSingleCreatureFromStorage(NPC_HATEREL);
            case 6:
                return m_creature;
        }
        return NULL;
    }

    void CallToFight(bool bStartFight)
    {
        if (Creature* pDwarf = GetDwarfForPhase(m_uiDwarfRound))
        {
            if (bStartFight && pDwarf->isAlive())
            {
                pDwarf->setFaction(FACTION_HOSTILE);
                pDwarf->SetInCombatWithZone();              // attackstart
            }
            else
            {
                if (!pDwarf->isAlive() || pDwarf->isDead())
                    pDwarf->Respawn();

                pDwarf->setFaction(FACTION_NEUTRAL);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (m_pInstance)
        {
            if (m_pInstance->GetData(TYPE_TOMB_OF_SEVEN) == IN_PROGRESS)
            {
                if (m_uiDwarfRound < MAX_DWARF)
                {
                    if (m_uiCallToFight_Timer < diff)
                    {
                        CallToFight(true);
                        ++m_uiDwarfRound;
                        m_uiCallToFight_Timer = 30000;
                    }
                    else
                        m_uiCallToFight_Timer -= diff;
                }
            }
            else if (m_pInstance->GetData(TYPE_TOMB_OF_SEVEN) == FAIL)
            {
                for (m_uiDwarfRound = 0; m_uiDwarfRound < MAX_DWARF; ++m_uiDwarfRound)
                    CallToFight(false);

                m_uiDwarfRound = 0;
                m_uiCallToFight_Timer = 0;

                if (m_pInstance)
                    m_pInstance->SetData(TYPE_TOMB_OF_SEVEN, NOT_STARTED);
            }
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        //ShadowVolley_Timer
        if (m_uiShadowVolley_Timer < diff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),SPELL_SHADOWBOLTVOLLEY);
            m_uiShadowVolley_Timer = 12000;
        }
        else
            m_uiShadowVolley_Timer -= diff;

        //Immolate_Timer
        if (m_uiImmolate_Timer < diff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                DoCastSpellIfCan(target,SPELL_IMMOLATE);

            m_uiImmolate_Timer = 25000;
        }
        else
            m_uiImmolate_Timer -= diff;

        //CurseOfWeakness_Timer
        if (m_uiCurseOfWeakness_Timer < diff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),SPELL_CURSEOFWEAKNESS);
            m_uiCurseOfWeakness_Timer = 45000;
        }
        else
            m_uiCurseOfWeakness_Timer -= diff;

        //DemonArmor_Timer
        if (m_uiDemonArmor_Timer < diff)
        {
            DoCastSpellIfCan(m_creature,SPELL_DEMONARMOR);
            m_uiDemonArmor_Timer = 300000;
        }
        else
            m_uiDemonArmor_Timer -= diff;

        //Summon Voidwalkers
        if (!m_bHasSummoned && HealthBelowPct(50))
        {
            m_creature->CastSpell(m_creature, SPELL_SUMMON_VOIDWALKERS, true);
            m_bHasSummoned = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_doomrel(Creature* pCreature)
{
    return new boss_doomrelAI(pCreature);
}

bool GossipHello_boss_doomrel(Player* pPlayer, Creature* pCreature)
{
    if (instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_TOMB_OF_SEVEN) == NOT_STARTED)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_CHALLENGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

	pPlayer->SEND_GOSSIP_MENU(2601, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_doomrel(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_DOOMREL_START_EVENT, pCreature, pPlayer);
            // start event
            if (instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData())
                pInstance->SetData(TYPE_TOMB_OF_SEVEN, IN_PROGRESS);

            break;
    }
    return true;
}

void AddSC_boss_tomb_of_seven()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "boss_gloomrel";
    pNewscript->GetAI = &GetAI_boss_gloomrel;
    pNewscript->pGossipHello = &GossipHello_boss_gloomrel;
    pNewscript->pGossipSelect = &GossipSelect_boss_gloomrel;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "boss_doomrel";
    pNewscript->GetAI = &GetAI_boss_doomrel;
    pNewscript->pGossipHello = &GossipHello_boss_doomrel;
    pNewscript->pGossipSelect = &GossipSelect_boss_doomrel;
    pNewscript->RegisterSelf();
}
