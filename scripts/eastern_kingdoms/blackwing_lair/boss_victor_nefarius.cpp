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
SDName: Boss_Victor_Nefarius
SD%Complete: 75
SDComment: unfinished
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "blackwing_lair.h"

enum eNefarian
{
    // Event texts
    SAY_GAMES_BEGIN_1               = -1469004,
    SAY_GAMES_BEGIN_2               = -1469005,
    SAY_VAEL_INTRO                  = -1469006,                    // when he corrupts Vaelastrasz

    // Nefarian phase two texts
    SAY_AGGRO                       = -1469007,
    SAY_XHEALTH                     = -1469008,
    SAY_SHADOWFLAME                 = -1469009,
    SAY_RAISE_SKELETONS             = -1469010,
    SAY_SLAY                        = -1469011,
    SAY_DEATH                       = -1469012,
    SAY_MAGE                        = -1469013,
    SAY_WARRIOR                     = -1469014,
    SAY_DRUID                       = -1469015,
    SAY_PRIEST                      = -1469016,
    SAY_PALADIN                     = -1469017,
    SAY_SHAMAN                      = -1469018,
    SAY_WARLOCK                     = -1469019,
    SAY_HUNTER                      = -1469020,
    SAY_ROGUE                       = -1469021,

    // Victor's spells
    SPELL_NEFARIANS_BARRIER         = 22663,
    SPELL_FEAR                      = 22678,    // 26070,
    SPELL_SHADOW_BOLT               = 22677,    // 21077,
    SPELL_SHADOW_BOLT_VOLLEY        = 22665,
    SPELL_SHADOW_COMMAND            = 22667,
    SPELL_SHADOWBLINK_1             = 22664,    // dummy effects
    SPELL_SHADOWBLINK_2             = 22681,    // dummy effects

    // Nefarian's spells
    SPELL_BELLOWING_ROAR            = 22686,
    SPELL_CLEAVE                    = 19983,    // 20691,
    SPELL_SHADOW_FLAME              = 22539,
    SPELL_SHADOW_FLAME_INITIAL      = 22972,
    SPELL_TAIL_LASH                 = 23364,
    SPELL_VEIL_OF_SHADOW            = 22687,    // 7068,

    SPELL_RAISE_UNDEAD_DRAKONID     = 23361,
    SPELL_RAISE_DRAKONID            = 23362,
    SPELL_SUMMON_DRAKONID_CORPSE    = 23363,

    // Class-call outs
    SPELL_WILD_POLYMORPH            = 23603,    // MAGE
    SPELL_BERSERK                   = 23397,    // WARRIOR
    SPELL_INVOLUNTARY_TRANSFORM     = 23398,    // DRUID
    SPELL_CORRUPTED_HEALING         = 23401,    // PRIEST
    SPELL_SIPHON_BLESSING           = 23418,    // PALADIN
    SPELL_CORRUPTED_TOTEMS          = 23425,    // SHAMAN - triggers spell 23424 - dummy - need to script
    SPELL_SUMMON_INFERNALS          = 23427,    // WARLOCK
    SPELL_CORRUPT_WEAPON            = 23436,    // HUNTER
    SPELL_PARALYZE                  = 23414,    // ROGUE

    MAX_DRAKONID                    = 42
};

uint32 aDrakonids[5] = {NPC_BLACK_DRAKONID, NPC_BLUE_DRAKONID, NPC_BRONZE_DRAKONID, NPC_GREEN_DRAKONID, NPC_RED_DRAKONID};

static Location Drake[]=
{
    {-7591.15f, -1204.05f, 476.80f, 0.0f},
    {-7514.59f, -1150.44f, 476.80f, 0.0f}
};

struct ClassInfo
{
    Classes classId;
    uint32 spellId;
    int32 yellId;
};

static ClassInfo CallOut[]=
{
    {CLASS_MAGE, SPELL_WILD_POLYMORPH, SAY_MAGE},
    {CLASS_WARRIOR, SPELL_BERSERK, SAY_WARRIOR},
    {CLASS_DRUID, SPELL_INVOLUNTARY_TRANSFORM, SAY_DRUID},
    {CLASS_PRIEST, SPELL_CORRUPTED_HEALING, SAY_PRIEST},
    {CLASS_PALADIN, SPELL_SIPHON_BLESSING, SAY_PALADIN},
    {CLASS_SHAMAN, SPELL_CORRUPTED_TOTEMS, SAY_SHAMAN},
    {CLASS_WARLOCK, SPELL_SUMMON_INFERNALS, SAY_WARLOCK},
    {CLASS_HUNTER, SPELL_CORRUPT_WEAPON, SAY_HUNTER},
    {CLASS_ROGUE, SPELL_PARALYZE, SAY_ROGUE}
};

#define GOSSIP_ITEM_1           "I've made no mistakes."
#define GOSSIP_ITEM_2           "You have lost your mind, Nefarius. You speak in riddles."
#define GOSSIP_ITEM_3           "Please do."

struct MANGOS_DLL_DECL boss_victor_nefariusAI : public ScriptedAI
{
    boss_victor_nefariusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();

        m_auiDrakonidType[0] = aDrakonids[urand(0,4)];
        m_auiDrakonidType[1] = aDrakonids[urand(0,4)];

        while(m_auiDrakonidType[0] == m_auiDrakonidType[1])
            m_auiDrakonidType[1] = aDrakonids[urand(0,4)];
    }

    bool m_bPhaseOne;
    bool m_bPhaseThree;

    uint32 m_auiDrakonidType[2];
    uint32 m_uiDrakonidSummoned;
    uint32 m_uiDrakonidDead;
    uint32 m_uiDrakonidSpawnTimer;
    uint32 m_uiShadowBoltTimer;
    uint32 m_uiFearTimer;
    uint32 m_uiResetTimer;

    uint32 m_uiBellowingRoarTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiShadowFlameTimer;
    uint32 m_uiTailLashTimer;
    uint32 m_uiVeilOfShadowTimer;
    uint32 m_uiClassCallTimer;

    DrakonidInfoList m_lDrakonidInfo;

    void Reset()
    {
        m_bPhaseOne = true;
        m_bPhaseThree = false;

        m_uiDrakonidSummoned = 0;
        m_uiDrakonidDead = 0;
        m_uiDrakonidSpawnTimer = 10000;
        m_uiShadowBoltTimer = 5000;
        m_uiFearTimer = 8000;
        m_uiResetTimer = 900000;                                // On official it takes him 15 minutes(900 seconds) to reset. We are only doing 1 minute to make testing easier

        m_uiBellowingRoarTimer = 30000;
        m_uiCleaveTimer = 7000;
        m_uiShadowFlameTimer = 12000;
        m_uiTailLashTimer = 10000;
        m_uiVeilOfShadowTimer = 15000;
        m_uiClassCallTimer = 35000;

        m_lDrakonidInfo.clear();
        m_creature->RemoveAurasDueToSpell(SPELL_NEFARIANS_BARRIER);

        if (m_creature->GetEntry() != NPC_VICTOR_NEFARIUS)
            m_creature->UpdateEntry(NPC_VICTOR_NEFARIUS);

        m_creature->setFaction(FACTION_FRIENDLY);
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);
    }

    void KilledUnit(Unit* pVictim)
    {
        if (urand(0,4))
            return;

        DoScriptText(SAY_SLAY, m_creature, pVictim);
    }

    void JustSummoned(Creature* pSummoned)
    {
        // Victor summons Rend in UBRS -> we don't any action here
        if (pSummoned->GetEntry() == 10429)
            return;

        if (pSummoned->GetEntry() != NPC_BONE_CONSTRUCT)
            ++m_uiDrakonidSummoned;

        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(pTarget);
        else
            pSummoned->SetInCombatWithZone();
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        if (!m_bPhaseOne)
            return;

        if (pSummoned->GetEntry() != NPC_BONE_CONSTRUCT)
        {
            pSummoned->CastSpell(pSummoned, SPELL_SUMMON_DRAKONID_CORPSE, true);
            float fX, fY, fZ;
            pSummoned->GetPosition(fX, fY, fZ);
            m_lDrakonidInfo.push_back(DrakonidInfo(fX, fY, fZ));

            ++m_uiDrakonidDead;

            // Start phase two
            if (m_uiDrakonidDead >= MAX_DRAKONID)
            {
                m_bPhaseOne = false;
                m_creature->InterruptNonMeleeSpells(false);
                m_creature->RemoveAurasDueToSpell(SPELL_NEFARIANS_BARRIER);
                m_creature->UpdateEntry(NPC_NEFARIAN);
                DoResetThreat();
                DoScriptText(SAY_AGGRO, m_creature);
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    DoCastSpellIfCan(pTarget, SPELL_SHADOW_FLAME_INITIAL);
                    m_creature->AI()->AttackStart(pTarget);
                }
            }
        }
    }

    void BeginEvent(Player* pTarget)
    {
        DoScriptText(SAY_GAMES_BEGIN_2, m_creature);

        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        DoCastSpellIfCan(m_creature, SPELL_NEFARIANS_BARRIER);
        m_creature->setFaction(FACTION_BLACKWING);
        m_creature->AI()->AttackStart(pTarget);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bPhaseOne)
        {
            // Shadow Bolt
            if (m_uiShadowBoltTimer < uiDiff)
            {
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                if (pTarget && DoCastSpellIfCan(pTarget, SPELL_SHADOW_BOLT) == CAST_OK)
                    m_uiShadowBoltTimer = urand(3000, 10000);
            }
            else
                m_uiShadowBoltTimer -= uiDiff;

            // Fear
            if (m_uiFearTimer < uiDiff)
            {
                Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
                if (pTarget && DoCastSpellIfCan(pTarget, SPELL_FEAR) == CAST_OK)
                    m_uiFearTimer = urand(10000, 20000);
            }
            else
                m_uiFearTimer -= uiDiff;

            // Drakonid spawn
            if (m_uiDrakonidSpawnTimer < uiDiff)
            {
                for (uint8 i = 0; i < 2; ++i)
                {
                    if (m_uiDrakonidSummoned < MAX_DRAKONID)
                        m_creature->SummonCreature(m_auiDrakonidType[i], Drake[i].x, Drake[i].y, Drake[i].z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30*IN_MILLISECONDS);
                }

                if (!urand(0,3) && m_uiDrakonidSummoned < MAX_DRAKONID)
                    m_creature->SummonCreature(NPC_CHROMATIC_DRAKONID, Drake[0].x, Drake[0].y, Drake[0].z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30*IN_MILLISECONDS);

                m_uiDrakonidSpawnTimer = 4000;
            }
            else
                m_uiDrakonidSpawnTimer -= uiDiff;
        }
        else
        {
            // When we are below X health
            if (!m_bPhaseThree && HealthBelowPct(20))
            {
                m_bPhaseThree = true;
                DoScriptText(SAY_RAISE_SKELETONS, m_creature);

                for(DrakonidInfoList::iterator itr = m_lDrakonidInfo.begin(); itr != m_lDrakonidInfo.end(); ++itr)
                    m_creature->SummonCreature(NPC_BONE_CONSTRUCT, itr->x, itr->y, itr->z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            }

            // Bellowing Roar
            if (m_uiBellowingRoarTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BELLOWING_ROAR) == CAST_OK)
                    m_uiBellowingRoarTimer = 30000;
            }
            else
                m_uiBellowingRoarTimer -= uiDiff;

            // Cleave
            if (m_uiCleaveTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                    m_uiCleaveTimer = 7000;
            }
            else
                m_uiCleaveTimer -= uiDiff;

            // Shadow Flame
            if (m_uiShadowFlameTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_FLAME) == CAST_OK)
                    m_uiShadowFlameTimer = 12000;
            }
            else
                m_uiShadowFlameTimer -= uiDiff;

            // Veil Of Shadow
            if (m_uiVeilOfShadowTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_VEIL_OF_SHADOW) == CAST_OK)
                    m_uiVeilOfShadowTimer = 15000;
            }
            else
                m_uiVeilOfShadowTimer -= uiDiff;

            // Tail Lash
            if (m_uiTailLashTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_TAIL_LASH) == CAST_OK)
                    m_uiTailLashTimer = 10000;
            }
            else
                m_uiTailLashTimer -= uiDiff;

            // Class Call
            if (m_uiClassCallTimer < uiDiff)
            {
                // On official it is based on what classes are currently on the hostil list
                // but we can't do that yet so just randomly call one
                uint32 i = urand(0,8);
                if (DoCastSpellIfCan(m_creature, CallOut[i].spellId) == CAST_OK)
                {
                    DoScriptText(CallOut[i].yellId, m_creature);
                    m_uiClassCallTimer = urand(35000, 40000);
                }
            }
            else
                m_uiClassCallTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_boss_victor_nefarius(Creature* pCreature)
{
    return new boss_victor_nefariusAI(pCreature);
}

bool GossipHello_boss_victor_nefarius(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->GetMapId() == 469)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_1 , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(7134, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(7198, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(7199, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            pPlayer->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_GAMES_BEGIN_1, pCreature);
            if (boss_victor_nefariusAI* pNefAI = dynamic_cast<boss_victor_nefariusAI*>(pCreature->AI()))
                pNefAI->BeginEvent(pPlayer);
            break;
    }
    return true;
}

void AddSC_boss_victor_nefarius()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_victor_nefarius";
    pNewScript->GetAI = &GetAI_boss_victor_nefarius;
    pNewScript->pGossipHello = &GossipHello_boss_victor_nefarius;
    pNewScript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    pNewScript->RegisterSelf();
}
