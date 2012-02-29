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
SDName: Boss_Majordomo_Executus
SD%Complete: 70
SDComment:
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"
#include "molten_core.h"

enum eMajordomoExecutus
{
    SAY_AGGRO                   = -1409003,
    SAY_SPAWN                   = -1409004,
    SAY_SLAY                    = -1409005,
    SAY_SPECIAL                 = -1409006,
    SAY_DEFEAT                  = -1409007,

    SAY_SUMMON_MAJ              = -1409008,
    SAY_ARRIVAL1_RAG            = -1409009,
    SAY_ARRIVAL2_MAJ            = -1409010,
    SAY_ARRIVAL3_RAG            = -1409011,
    SAY_ARRIVAL5_RAG            = -1409012,

    SPELL_MAGIC_REFLECTION      = 20619,
    SPELL_DAMAGE_SHIELD         = 21075,
    SPELL_BLAST_WAVE            = 20229,
    SPELL_AEGIS                 = 20620,                // This is self casted whenever we are below 50%
    SPELL_TELEPORT              = 20618,
    SPELL_TELEPORT_VISUAL       = 19484,
    SPELL_SUMMON_RAGNAROS       = 19774,

    // Flamewaker Elite
    SPELL_FIRE_BLAST            = 20623,

    // Flamewaker Healer
    SPELL_SHADOW_BOLT           = 21077,
    SPELL_GREATER_HEAL          = 22883,                // need correct pretbc spell id

    // Ragnaros's "underwater" spells
    SPELL_RAGNAROS_EMERGE       = 20568,
    SPELL_RAGNAROS_SUBMERGE_FADE = 21107,
    SPELL_RAGNAROS_SUBMERGE_VISUAL = 20567,
};

#define SPAWN_RAG_X             838.51f
#define SPAWN_RAG_Y             -829.84f
#define SPAWN_RAG_Z             -232.00f
#define SPAWN_RAG_O             1.70f

#define GOSSIP_LET_ME_SPEAK     "Let me speak to your master, servant."

struct MANGOS_DLL_DECL boss_majordomoAI : public npc_escortAI
{
    boss_majordomoAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_pInstance = (instance_molten_core*)pCreature->GetInstanceData();
        m_uiEventPhase = 0;
        m_uiEventTimer = 0;
        m_uiRagnarosSummonPlayerGUID = ObjectGuid();
        m_bCanWalk = false;
        Reset();
    }

    instance_molten_core* m_pInstance;

    uint32 m_uiMagicReflectionTimer;
    uint32 m_uiDamageShieldTimer;
    uint32 m_uiBlastwaveTimer;
    uint32 m_uiTeleportTimer;

    uint8 m_uiEventPhase;
    uint32 m_uiEventTimer;

    ObjectGuid m_uiRagnarosSummonPlayerGUID;

    bool m_bCanWalk;

    void Reset()
    {
        m_uiMagicReflectionTimer = 30000;                     // Damage reflection first so we alternate
        m_uiDamageShieldTimer = 15000;
        m_uiBlastwaveTimer = 10000;
        m_uiTeleportTimer = 30000;  //60000

        if ((m_pInstance && m_pInstance->GetData(TYPE_MAJORDOMO) == DONE) || HasEscortState(STATE_ESCORT_ESCORTING) || HasEscortState(STATE_ESCORT_PAUSED))
            return;

        m_uiEventPhase = 0;
        m_uiEventTimer = 0;

		m_uiRagnarosSummonPlayerGUID.Clear();

        m_bCanWalk = false;
    }

    void JustReachedHome()
    {
        if (m_pInstance && m_pInstance->GetData(TYPE_MAJORDOMO) != DONE)
            m_pInstance->SetData(TYPE_MAJORDOMO, FAIL);
    }

    void JustStartedEscort()
    {
        m_bCanWalk = true;

        // Respawn dead Ragnaros
        if (Creature* pRag = GetRagnaros())
            if (pRag->isDead())
                pRag->Respawn();
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 2:
                m_bCanWalk = false;
                m_uiEventPhase = 20;
                m_uiEventTimer = 2000;
                break;
        }
    }

    void KilledUnit(Unit* /*pVictim*/)
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_SLAY, m_creature);
    }

    void Aggro(Unit* /*pWho*/)
    {
        m_creature->CallForHelp(40.0f);
        DoScriptText(SAY_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAJORDOMO, IN_PROGRESS);
    }

    Creature* GetRagnaros()
    {
        if (!m_pInstance)
            return NULL;

		Creature* pRagnaros = m_pInstance->GetSingleCreatureFromStorage(NPC_RAGNAROS);

        return pRagnaros ? pRagnaros : NULL;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventPhase >= 20)
            {
                Player* pPlayer = m_creature->GetMap()->GetPlayer(m_uiRagnarosSummonPlayerGUID);
                if (!pPlayer || (pPlayer && pPlayer->isDead()))
                {
                    // cancel summoning event
                    m_creature->SetHealth(0);
                    m_creature->SetDeathState(JUST_DIED);
                    return;
                }
            }

            if (m_uiEventTimer <= uiDiff)
            {
                m_uiEventTimer = 0;

                switch(m_uiEventPhase)
                {
                    // Event #1: Teleport
                    case 1:
                        m_uiEventTimer = 3000; // ~35000
                        break;
                    case 2:
                        DoCastSpellIfCan(m_creature, SPELL_TELEPORT_VISUAL);
                        m_uiEventTimer = 1300;
                        break;
                    case 3:
                        //TODO: SetHomePosition
                        m_creature->NearTeleportTo(852.95f, -819.74f, -229.0f, 3.70f);
                        m_uiEventTimer = 1000;
                        break;
                    case 4:
                        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        m_uiEventPhase = 0;
                        return;
                        //break;

                    // Event #2: To talk with Ragnaros
                    case 20:
                        if (Creature* pRagnaros = GetRagnaros())
                        {
                            //m_creature->SetFacingToObject(pRagnaros);
                            pRagnaros->setFaction(FACTION_FRIENDLY);
                            pRagnaros->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            m_creature->SetFacingTo(5.25f);
                            pRagnaros->CastSpell(pRagnaros, SPELL_RAGNAROS_SUBMERGE_VISUAL, true);
                            pRagnaros->CastSpell(pRagnaros, SPELL_RAGNAROS_SUBMERGE_FADE,  true);
                        }
                        m_uiEventTimer = 3000;
                        break;
                    case 21:
                        if (Creature* pRagnaros = GetRagnaros())
                            pRagnaros->SetVisibility(VISIBILITY_ON);
                        DoScriptText(SAY_SUMMON_MAJ, m_creature);
                        m_uiEventTimer = 10000;
                        break;
                    case 22:
                        if (Creature* pRagnaros = GetRagnaros())
                        {
                            pRagnaros->RemoveAurasDueToSpell(SPELL_RAGNAROS_SUBMERGE_FADE);
                            pRagnaros->CastSpell(pRagnaros, SPELL_RAGNAROS_EMERGE, false);
                        }
                        m_uiEventTimer = 5000;  // maybe 7000 will be better
                        break;
                    case 23:
                        if (Creature* pRagnaros = GetRagnaros())
                            pRagnaros->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                        m_uiEventTimer = 4000;
                        break;
                    case 24:
                        if (Creature* pRagnaros = GetRagnaros())
                            DoScriptText(SAY_ARRIVAL1_RAG, pRagnaros);
                        m_uiEventTimer = 13000;
                        break;
                    case 25:
                        DoScriptText(SAY_ARRIVAL2_MAJ, m_creature);
                        m_uiEventTimer = 10000;
                        break;
                    case 26:
                        if (Creature* pRagnaros = GetRagnaros())
                            DoScriptText(SAY_ARRIVAL3_RAG, pRagnaros);
                        m_uiEventTimer = 10000;
                        break;
                    case 27:
                        if (Creature* pRagnaros = GetRagnaros())
                            pRagnaros->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                        m_uiEventTimer = 7500;
                        break;
                    case 28:
                        // Majordomo died (fake corpse)
                        m_creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
                        m_uiEventTimer = 6000;
                        break;
                    case 29:
                        if (Creature* pRagnaros = GetRagnaros())
                        {
                            DoScriptText(SAY_ARRIVAL5_RAG, pRagnaros);

                            // Ragnaros look at his awakener (maybe not to be)
                            if (m_uiRagnarosSummonPlayerGUID)
                                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_uiRagnarosSummonPlayerGUID))
                                    pRagnaros->SetFacingToObject(pPlayer);
                        }
                        m_uiEventTimer = 12000; // ~15000
                        break;
                    case 30:
                        if (Creature* pRagnaros = GetRagnaros())
                        {
                            pRagnaros->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            pRagnaros->setFaction(pRagnaros->GetCreatureInfo()->faction_A);
                            if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_uiRagnarosSummonPlayerGUID))
                                pRagnaros->AI()->AttackStart(pPlayer);
                        }
                        m_uiEventTimer = 5000;
                        break;
                    case 31:
                        // To complete escort.
                        m_uiEventPhase = 0;
						m_uiRagnarosSummonPlayerGUID.Clear();
                        m_bCanWalk = true;
                        return;
                        //break;
                }
                ++m_uiEventPhase;
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        if (m_bCanWalk)
            npc_escortAI::UpdateAI(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;
            
        // When all adds of Majordomo are dead, evade and teleport to the Ragnaros's cave
        if (m_pInstance)
        {
            if (m_pInstance->GetData(TYPE_MAJORDOMO) == SPECIAL)
            {
                m_pInstance->SetData(TYPE_MAJORDOMO, DONE);
                m_pInstance->SetData(TYPE_MAJORDOMO_FIRST_SPAWN, DONE); // Chest will be spawned only one time
                DoScriptText(SAY_DEFEAT, m_creature);
    
                EnterEvadeMode();
                m_creature->setFaction(FACTION_FRIENDLY);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    
                m_uiEventPhase = 1;
                m_uiEventTimer = 100;
                return;
            }
        }
        //else
        //    error_log("SD0: Molten Core: m_pinstance is null, majordomo event will never end.");

        // Cast Aegis if less than 50% hp
        if (HealthBelowPct(50))
            DoCastSpellIfCan(m_creature, SPELL_AEGIS);

        // Magic Reflection
        if (m_uiMagicReflectionTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_MAGIC_REFLECTION);
            m_uiMagicReflectionTimer = 30000;
        }
        else
            m_uiMagicReflectionTimer -= uiDiff;

        // Damage Shield
        if (m_uiDamageShieldTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_DAMAGE_SHIELD);
            m_uiDamageShieldTimer = 30000;
        }
        else
            m_uiDamageShieldTimer -= uiDiff;

        // Blast Wave
        if (m_uiBlastwaveTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_BLAST_WAVE);
            m_uiBlastwaveTimer = 10000;
        }
        else
            m_uiBlastwaveTimer -= uiDiff;

        // Teleport (player at burning coals)
        if (m_uiTeleportTimer <= uiDiff)
        {
            uint32 randomnumber = urand(0,1);
            if (randomnumber == 0)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    m_creature->getThreatManager().modifyThreatPercent(m_creature->getVictim(), -100);
                    DoCastSpellIfCan(pTarget, SPELL_TELEPORT);
                }
            }
            else
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))
                {
                    m_creature->getThreatManager().modifyThreatPercent(m_creature->getVictim(), -100);
                    DoCastSpellIfCan(pTarget, SPELL_TELEPORT);
                }   
            }
            m_uiTeleportTimer = urand(20000, 30000);  //60000
        }
        else
            m_uiTeleportTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_majordomo(Creature* pCreature)
{
    return new boss_majordomoAI(pCreature);
}

bool GossipHello_boss_majordomo(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LET_ME_SPEAK, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
	pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_majordomo(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        if (boss_majordomoAI* pDomoAI = dynamic_cast<boss_majordomoAI*>(pCreature->AI()))
        {
            pPlayer->CLOSE_GOSSIP_MENU();
            pDomoAI->m_uiRagnarosSummonPlayerGUID = pPlayer->GetObjectGuid();
            pDomoAI->Start(false, 0, 0);
        }
    }
    return true;
}

/*######
## mob_flamewaker_healer
######*/

struct MANGOS_DLL_DECL mob_flamewaker_healerAI : public ScriptedAI
{
    mob_flamewaker_healerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiShadowBoltTimer;
    uint32 m_uiGreaterHealTimer;

    void Reset()
    {
        m_uiShadowBoltTimer = urand(3000, 7000);
        m_uiGreaterHealTimer = 10000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Shadow Bolt
        if (m_uiShadowBoltTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_BOLT);
            m_uiShadowBoltTimer = urand(3500,6000);
        }
        else
            m_uiShadowBoltTimer -= uiDiff;

        // Greater Heal
        if (m_uiGreaterHealTimer <= uiDiff)
        {
            if (Unit* pTarget = DoSelectLowestHpFriendly(40.0f, 1000))
                DoCastSpellIfCan(pTarget, SPELL_GREATER_HEAL);

            m_uiGreaterHealTimer = urand(8000,12000);
        }
        else
            m_uiGreaterHealTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_flamewaker_healer(Creature* pCreature)
{
    return new mob_flamewaker_healerAI(pCreature);
}

/*######
## mob_flamewaker_elite
######*/

struct MANGOS_DLL_DECL mob_flamewaker_eliteAI : public ScriptedAI
{
    mob_flamewaker_eliteAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiBlastWaveTimer;
    uint32 m_uiFireBlastTimer;

    void Reset()
    {
        m_uiBlastWaveTimer = 10000;
        m_uiFireBlastTimer = 20000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Fire Blast
        if (m_uiBlastWaveTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_FIRE_BLAST);
            m_uiBlastWaveTimer = urand(8000,12000);
        }
        else
            m_uiBlastWaveTimer -= uiDiff;

        // Blast Wave
        if (m_uiFireBlastTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_BLAST_WAVE);
            m_uiFireBlastTimer = 10000;
        }
        else
            m_uiFireBlastTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_flamewaker_elite(Creature* pCreature)
{
    return new mob_flamewaker_eliteAI(pCreature);
}

void AddSC_boss_majordomo()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_majordomo";
    pNewScript->GetAI = &GetAI_boss_majordomo;
    pNewScript->pGossipHello = &GossipHello_boss_majordomo;
    pNewScript->pGossipSelect = &GossipSelect_boss_majordomo;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_flamewaker_healer";
    pNewScript->GetAI = &GetAI_mob_flamewaker_healer;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_flamewaker_elite";
    pNewScript->GetAI = &GetAI_mob_flamewaker_elite;
    pNewScript->RegisterSelf();
}
