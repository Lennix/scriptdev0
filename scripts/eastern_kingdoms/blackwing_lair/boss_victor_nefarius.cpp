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
#include "precompiled.h"
#include "blackwing_lair.h"
#include "TemporarySummon.h"

struct DrakonIdInfo
{
    DrakonIdInfo(float _x, float _y, float _z)
        : x(_x), y(_y), z(_z) {}

    float x, y, z;
};

typedef std::vector<DrakonIdInfo> DrakonIdInfoVector;

DrakonIdInfoVector NefarianDragons;
std::vector<Creature*> NefarianBoneConstructs;

  ///////////////////////
 /////  PHASE ONE  /////
///////////////////////

/* ScriptData
SD%Complete: 75
SDComment:		Victor Nefarius:	spawns Nef in wrong place, need more flying points
                Nefarian :			Some issues with class calls effecting more than one class
EndScriptData */

enum
{
    SAY_GAMESBEGIN_1                = -1469004,
    SAY_GAMESBEGIN_2                = -1469005,

    MAX_DRAKES                      = 5,
    MAX_DRAKE_SUMMONS               = 42,
    NPC_BRONZE_DRAKANOID            = 14263,
    NPC_BLUE_DRAKANOID              = 14261,
    NPC_RED_DRAKANOID               = 14264,
    NPC_GREEN_DRAKANOID             = 14262,
    NPC_BLACK_DRAKANOID             = 14265,
    NPC_CHROMATIC_DRAKANOID         = 14302,

    //source old.wowhead.com -> lord victor nefarius
    SPELL_NEFARIUS_BARRIER          = 22663,                // immunity in phase 1
    SPELL_SHADOWBOLT                = 22677,
    SPELL_FEAR                      = 26678,                
    SPELL_SHADOWBOLT_VOLLEY			= 22665,
    SPELL_SILENCE					= 22666,
    SPELL_SHADOW_COMMAND			= 22667,
    SPELL_SHADOWBLINK				= 22664,				// or 22681 -> teleport around the room, possibly random
    SPELL_BONE_CONTRUST				= 23363,

    FACTION_BLACK_DRAGON            = 103,
    FACTION_FRIENDLY                = 35,
    FACTION_HOSTILE					= 14,
};

struct SpawnLocation
{
    float m_fX, m_fY, m_fZ;
};

static const SpawnLocation aNefarianLocs[5] =
{
    {-7599.32f, -1191.72f, 475.545f},                       // opening where red/blue/black darknid spawner appear (ori 3.05433)
    {-7526.27f, -1135.04f, 473.445f},                       // same as above, closest to door (ori 5.75959)
    {-7498.177f, -1273.277f, 481.649f},                     // nefarian spawn location (ori 1.798)
    {-7592.0f, -1264.0f, 481.0f},                           // hide pos (useless; remove this)
    {-7502.002f, -1256.503f, 476.758f},                     // nefarian fly to this position
};

#define GOSSIP_ITEM_NEFARIUS_1           "I've made no mistakes."
#define GOSSIP_ITEM_NEFARIUS_2           "You have lost your mind, Nefarius. You speak in riddles."
#define GOSSIP_ITEM_NEFARIUS_3           "Please do."

static const uint32 aPossibleDrake[MAX_DRAKES] = {NPC_BRONZE_DRAKANOID, NPC_BLUE_DRAKANOID, NPC_RED_DRAKANOID, NPC_GREEN_DRAKANOID, NPC_BLACK_DRAKANOID};

//This script is complicated
//Instead of morphing Victor Nefarius we will have him control phase 1
//And then have him spawn "Nefarian" for phase 2
//When phase 2 starts Victor Nefarius will go invisible and stop attacking
//If Nefarian reched home because nef killed the players then nef will trigger this guy to EnterEvadeMode
//and allow players to start the event over
//If nefarian dies then he will kill himself then he will be despawned in Nefarian script
//To prevent players from doing the event twice

// Dev note: Lord Victor Nefarius should despawn completely, then ~5 seconds later Nefarian should appear.

struct MANGOS_DLL_DECL boss_victor_nefariusAI : public ScriptedAI
{
    boss_victor_nefariusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        //sit down on the throne
        m_creature->SetStandState(UNIT_STAND_STATE_SIT_HIGH_CHAIR);

        // set gossip and friendly flag to begin the event
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        m_creature->setFaction(FACTION_FRIENDLY);

        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

        //get instance id dragons
        m_uiDrakeType[0] = aPossibleDrake[m_pInstance->GetData(VALUE_DRAGON1)];
        m_uiDrakeType[1] = aPossibleDrake[m_pInstance->GetData(VALUE_DRAGON2)];

        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiSpawnedAddDeadCounter;
    uint32 m_uiAddSpawnTimer;
    uint32 m_uiShadowBoltTimer;
    uint32 m_uiShadowBoltVolleyTimer;
    uint32 m_uiSilenceTimer;
    uint32 m_uiFearTimer;
    uint32 m_uiMindControlTimer;
    uint32 m_uiShadowBlinkTimer;
    uint32 m_uiResetTimer;
    uint32 m_uiShadowCommandTimer;
    uint32 m_uiDrakeType[2];
    bool summonedDragonNefarian;
    Unit* ShadowBoltPlayer;


    void Reset()
    {
        m_uiSpawnedAddDeadCounter	= 0;
        m_uiAddSpawnTimer			= 5000;
        m_uiShadowBoltTimer			= 0;
        m_uiShadowBoltVolleyTimer	= urand(8000, 15000);
        m_uiFearTimer				= urand(3000, 8000);
        m_uiSilenceTimer			= urand(5000, 12000);
        m_uiShadowCommandTimer		= urand(8000, 15000);
        m_uiShadowBlinkTimer		= urand(30000, 45000);
        m_uiResetTimer				= 15 * MINUTE * IN_MILLISECONDS;
        summonedDragonNefarian		= false;

        // Make visible if needed
        if (m_creature->GetVisibility() != VISIBILITY_ON)
            m_creature->SetVisibility(VISIBILITY_ON);

        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        m_creature->setFaction(FACTION_FRIENDLY);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void Aggro(Unit* pWho)
    {
        if (isEndbossFight())
        {
            if (m_pInstance)
                m_pInstance->SetData(TYPE_NEFARIAN, IN_PROGRESS);
        }
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_NEFARIAN, FAIL);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_NEFARIAN)
        {
            pSummoned->SetRespawnDelay(7*DAY);
            pSummoned->RemoveSplineFlag(SPLINEFLAG_WALKMODE);

            // see boss_onyxia (also note the removal of this in boss_nefarian)
            pSummoned->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND/* | UNIT_BYTE1_FLAG_UNK_2*/);
            pSummoned->AddSplineFlag(SPLINEFLAG_FLYING);

            // Let Nefarian fly towards combat area
            pSummoned->GetMotionMaster()->MovePoint(1, aNefarianLocs[4].m_fX, aNefarianLocs[4].m_fY, aNefarianLocs[4].m_fZ);
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId)
    {
        // If Nefarian has reached combat area, let him attack
        if (pSummoned->GetEntry() == NPC_NEFARIAN && uiMotionType == POINT_MOTION_TYPE && uiPointId == 1)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        // Despawn self when Nefarian is killed
        if (pSummoned->GetEntry() == NPC_NEFARIAN)
            m_creature->ForcedDespawn();
        else
        {
            m_uiSpawnedAddDeadCounter++;
            pSummoned->CastSpell(pSummoned, SPELL_BONE_CONTRUST, true);

            float fX, fY, fZ;
            pSummoned->GetPosition(fX, fY, fZ);
            NefarianDragons.push_back(DrakonIdInfo(fX, fY, fZ));
        }
    }

    void SpawnDragonWave()
    {
        uint32 uiCreatureId = 0;
        for (uint8 i = 0; i < 2; i++)
        {
            // 1 in 3 chance it will be a chromatic
            uiCreatureId = urand(0, 4) ? m_uiDrakeType[i] : NPC_CHROMATIC_DRAKANOID;
            Creature* cDragon = m_creature->SummonCreature(uiCreatureId, aNefarianLocs[i].m_fX, aNefarianLocs[i].m_fY, aNefarianLocs[i].m_fZ, 5.000f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1500);
            if (cDragon)
            {
                //source hordeguides
                cDragon->SetLevel(60);
                cDragon->setFaction(FACTION_HOSTILE);
                if (uiCreatureId == NPC_CHROMATIC_DRAKANOID)
                    cDragon->SetMaxHealth(24416);
                else
                    cDragon->SetMaxHealth(15260);

                //dragon immunity
                switch(m_uiDrakeType[i])
                {
                    case NPC_BRONZE_DRAKANOID :
                        cDragon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ARCANE, true);
                        break;
                    case NPC_BLUE_DRAKANOID :
                        cDragon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_FROST, true);
                        cDragon->SetResistance(SPELL_SCHOOL_ARCANE, 300);
                        break;
                    case NPC_RED_DRAKANOID :
                        cDragon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_FIRE, true);
                        break;
                    case NPC_GREEN_DRAKANOID :
                        cDragon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_NATURE, true);
                        break;
                    case NPC_BLACK_DRAKANOID :
                        cDragon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_FIRE, true);
                        cDragon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_SHADOW, true);
                        break;
                }	
                cDragon->SetInCombatWithZone();
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    cDragon->AI()->AttackStart(pTarget);
            }
        }
    }

    //checks if lord victor nefarius is at EndbossFight because he is often used for events!
    bool isEndbossFight()
    {
        bool endbossFight = false;
        if (m_creature->HasAura(SPELL_NEFARIUS_BARRIER) && m_creature->getFaction() == FACTION_BLACK_DRAGON)
            endbossFight = true;

        return endbossFight;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (isEndbossFight())
        {
            // Only do this if we haven't spawned nef yet
            if (m_uiSpawnedAddDeadCounter < MAX_DRAKE_SUMMONS)
            {
                // Shadowbolt Volley Timer
                if (m_uiShadowBoltVolleyTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_SHADOWBOLT_VOLLEY) == CAST_OK)
                        m_uiShadowBoltVolleyTimer = urand(8000, 15000);
                }
                else
                    m_uiShadowBoltVolleyTimer -= uiDiff;

                // Silence Timer
                if (m_uiSilenceTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (pTarget->getPowerType() == POWER_MANA && pTarget->GetTypeId() == TYPEID_PLAYER)
                        {
                            m_creature->CastSpell(pTarget, SPELL_SILENCE, true);
                            m_uiSilenceTimer = urand(8000, 15000);
                        }
                    }
                }
                else
                    m_uiSilenceTimer -= uiDiff;

                 // Shadow Command Timer
                if (m_uiShadowCommandTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_SHADOW_COMMAND) == CAST_OK)
                            m_uiShadowCommandTimer = urand(15000, 25000);
                    }
                }
                else
                    m_uiShadowCommandTimer -= uiDiff;

                // Fear Timer
                if (m_uiFearTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_FEAR) == CAST_OK)
                            m_uiFearTimer = urand(10000, 20000);
                    }
                }
                else
                    m_uiFearTimer -= uiDiff;

                // Shadow Blink Timer
                if (m_uiShadowBlinkTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        m_creature->NearTeleportTo(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());
                        m_creature->CastSpell(m_creature, SPELL_SHADOWBLINK, true);
                        m_uiShadowBlinkTimer = urand(30000, 45000);
                        //get new shadowbolt target at new position
                        m_uiShadowBoltTimer = 0;
                    }
                }
                else
                    m_uiShadowBlinkTimer -= uiDiff;


                // spawning mechanism
                if (m_uiAddSpawnTimer < uiDiff)
                {
                    SpawnDragonWave();

                    m_uiAddSpawnTimer = 4000;
                }
                else
                    m_uiAddSpawnTimer -= uiDiff;

                //  cast Shadowbolt if nothing is to do
                if (m_uiShadowBoltTimer != 0)
                {
                    if (ShadowBoltPlayer)
                    {
                        if (DoCastSpellIfCan(ShadowBoltPlayer, SPELL_SHADOWBOLT) == CAST_OK)
                            m_uiShadowBoltTimer--;
                    }
                    else
                        m_uiShadowBoltTimer--;
                }
                else
                {
                    ShadowBoltPlayer = GetPlayerAtMinimumRange(30.0);
                    m_uiShadowBoltTimer = urand(1,3);
                }
            }
            else
            {
                if (!summonedDragonNefarian)
                {
                    //Begin phase 2 by spawning Nefarian
                    if (m_uiSpawnedAddDeadCounter >= MAX_DRAKE_SUMMONS)
                    {
                        //Teleport Victor Nefarius way out of the map
                        //MapManager::Instance().GetMap(m_creature->GetMapId(), m_creature)->CreatureRelocation(m_creature,0,0,-5000,0);

                        //Inturrupt any spell casting
                        m_creature->InterruptNonMeleeSpells(false);

                        //Root self
                        DoCastSpellIfCan(m_creature, 33356, CAST_TRIGGERED);

                        //Make super invis
                        if (m_creature->GetVisibility() != VISIBILITY_OFF)
                            m_creature->SetVisibility(VISIBILITY_OFF);

                        // Do not teleport him away, this is not needed (invisible and rooted)
                        //Teleport self to a hiding spot
                        //m_creature->NearTeleportTo(aNefarianLocs[3].m_fX, aNefarianLocs[3].m_fY, aNefarianLocs[3].m_fZ, 0.0f);

                        // Spawn Nefarian
                        // Summon as active, to be able to work proper!
                        m_creature->SummonCreature(NPC_NEFARIAN, aNefarianLocs[2].m_fX, aNefarianLocs[2].m_fY, aNefarianLocs[2].m_fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0, true);
                        summonedDragonNefarian = true;
                    }
                }
            }
        }
    }
};

bool GossipHello_boss_victor_nefarius(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    pPlayer->SEND_GOSSIP_MENU(7134, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(7198, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            pPlayer->SEND_GOSSIP_MENU(7199, pCreature->GetObjectGuid());
            DoScriptText(SAY_GAMESBEGIN_1, pCreature);
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_GAMESBEGIN_2, pCreature);
            // remove gossip, set hostile and attack
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            pCreature->setFaction(FACTION_BLACK_DRAGON);
            pCreature->CastSpell(pCreature, SPELL_NEFARIUS_BARRIER, false);
            pCreature->AI()->AttackStart(pPlayer);
            pCreature->SetInCombatWithZone();
            break;
    }
    return true;
}

  ///////////////////////
 /////  PHASE TWO  /////
///////////////////////

enum
{
    SAY_AGGRO                   = -1469007,
    SAY_XHEALTH                 = -1469008,             // at 5% hp
    SAY_SHADOWFLAME             = -1469009,
    SAY_RAISE_SKELETONS         = -1469010,
    SAY_SLAY                    = -1469011,
    SAY_DEATH                   = -1469012,

    SAY_MAGE                    = -1469013,
    SAY_WARRIOR                 = -1469014,
    SAY_DRUID                   = -1469015,
    SAY_PRIEST                  = -1469016,
    SAY_PALADIN                 = -1469017,
    SAY_SHAMAN                  = -1469018,
    SAY_WARLOCK                 = -1469019,
    SAY_HUNTER                  = -1469020,
    SAY_ROGUE                   = -1469021,

    SPELL_SHADOWFLAME_INITIAL   = 22992,                // old spell id 22972 -> wrong
    SPELL_SHADOWFLAME           = 22539,
    SPELL_BELLOWING_ROAR        = 22686,
    SPELL_VEIL_OF_SHADOW        = 22687,                // old spell id 7068 -> wrong
    SPELL_CLEAVE                = 20691,
    SPELL_TAIL_LASH             = 23364,                

    SPELL_MAGE                  = 23410,                // wild magic
    SPELL_WARRIOR               = 23397,                // beserk
    SPELL_DRUID                 = 23398,                // cat form
    SPELL_PRIEST                = 23401,                // corrupted healing
    SPELL_PALADIN               = 23418,                // syphon blessing
    SPELL_SHAMAN                = 23425,                // totems
    SPELL_WARLOCK               = 23427,                // infernals    -> should trigger 23426
    SPELL_HUNTER                = 23436,                // bow broke
    SPELL_ROGUE                 = 23414,                // Paralise

    TEAM_ALLIANCE				=     1,
    TEAM_HORDE					=	  2,

    NPC_BONE_CONSTRUCT			= 14605,
};

static int32 Class[9][2] =
{
    {SPELL_MAGE, SAY_MAGE},
    {SPELL_WARRIOR, SAY_WARRIOR},
    {SPELL_DRUID, SAY_DRUID},
    {SPELL_PRIEST, SAY_PRIEST},
    {SPELL_WARLOCK, SAY_WARLOCK},
    {SPELL_HUNTER, SAY_HUNTER},
    {SPELL_ROGUE, SAY_ROGUE},
    {SPELL_PALADIN, SAY_PALADIN},
    {SPELL_SHAMAN, SAY_SHAMAN}
};

struct MANGOS_DLL_DECL boss_nefarianAI : public ScriptedAI
{
    uint32 team;

    boss_nefarianAI(Creature* pCreature) : ScriptedAI(pCreature), team(0)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

        //source hordeguides classic
        m_creature->SetLevel(63);
        m_creature->SetMaxHealth(2165150);
        m_creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_FIRE, true);

        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiShadowFlameTimer;
    uint32 m_uiBellowingRoarTimer;
    uint32 m_uiVeilOfShadowTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiTailLashTimer;
    uint32 m_uiClassCallTimer;
    bool m_bPhase3;
    bool m_bHasEndYell;

    void Reset()
    {
        m_uiShadowFlameTimer    = urand(10000, 15000);                            // These times are probably wrong
        m_uiBellowingRoarTimer  = urand(28000, 32000);
        m_uiVeilOfShadowTimer   = urand(12000, 18000);
        m_uiCleaveTimer         = urand(5000, 9000);
        m_uiTailLashTimer       = 10000;
        m_uiClassCallTimer      = urand(25000, 35000);                            // 35-45 seconds -> wowwiki.com
        m_bPhase3               = false;
        m_bHasEndYell           = false;

        DespawnBoneConstructs();
        m_creature->SetSplineFlags(SplineFlags(SPLINEFLAG_FLYING));
    }

    void KilledUnit(Unit* pVictim)
    {
        if (urand(0, 4) == 0)
            return;

        DoScriptText(SAY_SLAY, m_creature, pVictim);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_NEFARIAN, DONE);
    }

    void JustReachedHome()
    {
        if (m_pInstance)
        {
            m_pInstance->SetData(TYPE_NEFARIAN, FAIL);

            // Cleanup encounter
            if (m_creature->IsTemporarySummon())
            {
                TemporarySummon* pTemporary = (TemporarySummon*)m_creature;

                if (Creature* pNefarius = m_creature->GetMap()->GetCreature(pTemporary->GetSummonerGuid()))
                    pNefarius->AI()->EnterEvadeMode();
            }
            m_creature->ForcedDespawn();
        }
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        // Remove flying in case Nefarian aggroes before his combat point was reached
        if (m_creature->HasSplineFlag(SPLINEFLAG_FLYING))
        {
            m_creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, 0);
            m_creature->RemoveSplineFlag(SPLINEFLAG_FLYING);
        }

        DoCastSpellIfCan(m_creature, SPELL_SHADOWFLAME_INITIAL);
    }

    void SpawnBoneConstructs()
    {
        for(DrakonIdInfoVector::const_iterator it = NefarianDragons.begin(); it != NefarianDragons.end(); ++it)
        {
            Creature* cBoneConstruct = m_creature->SummonCreature(NPC_BONE_CONSTRUCT, it->x, it->y, it->z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);
            if (cBoneConstruct)
            {
                //source hordeguides classic
                cBoneConstruct->SetLevel(60);
                cBoneConstruct->SetMaxHealth(6104);
                cBoneConstruct->setFaction(FACTION_HOSTILE);

                cBoneConstruct->SetInCombatWithZone();
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    cBoneConstruct->AI()->AttackStart(pTarget);

                NefarianBoneConstructs.push_back(cBoneConstruct);
            }
        }

        NefarianDragons.clear();
    }

    void DespawnBoneConstructs()
    {
        for(std::vector<Creature*>::const_iterator it = NefarianBoneConstructs.begin(); it != NefarianBoneConstructs.end(); ++it)
        {
            (*it)->ForcedDespawn();
            (*it)->AddObjectToRemoveList();
        }
        NefarianBoneConstructs.clear();
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        //get team
        if (!team)
        {
            Player* tPlayer = GetPlayerAtMinimumRange(50.0);
            if (tPlayer)
            {
                if (tPlayer->GetTeam() == ALLIANCE)
                    team = TEAM_ALLIANCE;
                else
                {
                    team = TEAM_HORDE;
                    Class[7][0] = Class[8][0];
                    Class[7][1] = Class[8][1];
                }
            }
        }

        // ShadowFlame_Timer
        if (m_uiShadowFlameTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHADOWFLAME) == CAST_OK)
            {
                m_uiShadowFlameTimer = urand(12000, 18000);

                if (urand(0, 2) == 0)
                    DoScriptText(SAY_SHADOWFLAME, m_creature);
            }
        }
        else
            m_uiShadowFlameTimer -= uiDiff;

        // BellowingRoar_Timer
        if (m_uiBellowingRoarTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BELLOWING_ROAR) == CAST_OK)
                m_uiBellowingRoarTimer = urand(28000,32000);
        }
        else
            m_uiBellowingRoarTimer -= uiDiff;

        // VeilOfShadow_Timer
        if (m_uiVeilOfShadowTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO,0);
            if (pTarget)
            {
                if (DoCastSpellIfCan(pTarget, SPELL_VEIL_OF_SHADOW) == CAST_OK)
                    m_uiVeilOfShadowTimer = urand(12000, 18000);
            }
        }
        else
            m_uiVeilOfShadowTimer -= uiDiff;

        // Cleave_Timer
        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(5000, 9000);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        // TailLash_Timer
        if (m_uiTailLashTimer < uiDiff)
        {
            m_creature->CastSpell(m_creature, SPELL_TAIL_LASH, true);
            m_uiTailLashTimer = 10000;
        }
        else
            m_uiTailLashTimer -= uiDiff;

        // ClassCall_Timer
        if (m_uiClassCallTimer < uiDiff)
        {
            //Cast a random class call
            //On official it is based on what classes are currently on the hostil list
            //but we can't do that yet so just randomly call one
             uint32 i = urand(0,7);
             if (DoCastSpellIfCan(m_creature, Class[i][0]) == CAST_OK)
             {
                 //telport rogues in front of nefarian
                 if ( i == 6)
                 {
                    Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO,0);
                    Map::PlayerList const &PlayerList = m_creature->GetMap()->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                    {
                        Player* pPlayer = itr->getSource();
                        if (pTarget && pPlayer && pPlayer->getClass() == CLASS_ROGUE)
                            DoTeleportPlayer(pPlayer, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());
                    }
                 }

                DoScriptText(Class[i][1], m_creature);
                m_uiClassCallTimer = urand(20000, 25000);
             }
        }
        else
            m_uiClassCallTimer -= uiDiff;

          ///////////////////////
         ///// PHASE THREE /////
        ///////////////////////
        if (!m_bPhase3 && m_creature->GetHealthPercent() <= 20.0f)
        {
            SpawnBoneConstructs();

            m_bPhase3 = true;
            DoScriptText(SAY_RAISE_SKELETONS, m_creature);
        }

        // 5% hp yell
        if (!m_bHasEndYell && m_creature->GetHealthPercent() < 5.0f)
        {
            m_bHasEndYell = true;
            DoScriptText(SAY_XHEALTH, m_creature);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_victor_nefarius(Creature* pCreature)
{
    return new boss_victor_nefariusAI(pCreature);
}

CreatureAI* GetAI_boss_nefarian(Creature* pCreature)
{
    return new boss_nefarianAI(pCreature);
}

void AddSC_boss_nefarian()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_victor_nefarius";
    pNewScript->GetAI = &GetAI_boss_victor_nefarius;
    pNewScript->pGossipHello = &GossipHello_boss_victor_nefarius;
    pNewScript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_nefarian";
    pNewScript->GetAI = &GetAI_boss_nefarian;
    pNewScript->RegisterSelf();
}
