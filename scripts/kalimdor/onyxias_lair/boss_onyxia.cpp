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

/* ScriptData
SDName: Boss_Onyxia
SD%Complete: 70
SDComment: ToDo:
                    - ony shouldnt move if the topaggro target is in her meleerange
                    - ony shouldnt reset the aggro of player who arent reachable
                    - problem with hover animation after reaching a new movepoint
                    - special melee hits should hit her in phase breath

SDCategory: Onyxia's Lair
EndScriptData */

#include "precompiled.h"
#include "onyxias_lair.h"

enum
{
    SAY_AGGRO                   = -1249000,
    SAY_KILL                    = -1249001,
    SAY_PHASE_2_TRANS           = -1249002,
    SAY_PHASE_3_TRANS           = -1249003,
    EMOTE_BREATH                = -1249004,

    SPELL_WINGBUFFET            = 18500,
    SPELL_FLAMEBREATH           = 18435,
    SPELL_CLEAVE                = 19983,
    SPELL_TAILSWEEP             = 15847,
    SPELL_ERUPTION              = 17731,
    SPELL_KNOCK_AWAY            = 19633,
    SPELL_FIREBALL              = 18392,

    SPELL_BREATH_NORTH_TO_SOUTH = 17086,                    // 20x in "array" 
    SPELL_BREATH_SOUTH_TO_NORTH = 18351,                    // 11x in "array"

    SPELL_BREATH_EAST_TO_WEST   = 18576,                    // 7x in "array"
    SPELL_BREATH_WEST_TO_EAST   = 18609,                    // 7x in "array"

    SPELL_BREATH_SE_TO_NW       = 18564,                    // 12x in "array"
    SPELL_BREATH_NW_TO_SE       = 18584,                    // 12x in "array"
    SPELL_BREATH_SW_TO_NE       = 18596,                    // 12x in "array"
    SPELL_BREATH_NE_TO_SW       = 18617,                    // 12x in "array"

    SPELL_VISUAL_BREATH_A       = 4880,                     // Only and all of the above Breath spells (and their triggered spells) have these visuals
    SPELL_VISUAL_BREATH_B       = 4919,

    SPELL_BELLOWINGROAR         = 18431,
    SPELL_HEATED_GROUND         = 22191,                    // TODO

    SPELL_SUMMONWHELP           = 17646,                    // TODO this spell is only a summon spell, but would need a spell to activate the eggs

    MAX_WHELPS_PER_PACK         = 20,

    PHASE_START                 = 1,
    PHASE_BREATH                = 2,
    PHASE_END                   = 3,
    PHASE_BREATH_PRE            = 4,
    PHASE_BREATH_POST           = 5,

    liftOff                     = 0,
    landing                     = 1,

    x                           = 0,
    y                           = 1,
    z                           = 2
};

struct OnyxiaMove
{
    uint32 uiLocId;
    uint32 uiLocIdEnd;
    uint32 uiSpellId;
    float fX, fY, fZ;
};

static OnyxiaMove aMoveData[]=
{
    {0, 4, SPELL_BREATH_NORTH_TO_SOUTH,  22.8763f, -217.152f, -60.0548f},   // north
    {1, 5, SPELL_BREATH_NE_TO_SW,        10.2191f, -247.912f, -60.896f},    // north-east
    {2, 6, SPELL_BREATH_EAST_TO_WEST,   -31.4963f, -250.123f, -60.1278f},   // east
    {3, 7, SPELL_BREATH_SE_TO_NW,       -63.5156f, -240.096f, -60.477f},    // south-east
    {4, 0, SPELL_BREATH_SOUTH_TO_NORTH, -65.8444f, -213.809f, -60.2985f},   // south
    {5, 1, SPELL_BREATH_SW_TO_NE,       -58.2509f, -189.020f, -60.790f},    // south-west
    {6, 2, SPELL_BREATH_WEST_TO_EAST,   -33.5561f, -182.682f, -60.9457f},   // west
    {7, 3, SPELL_BREATH_NW_TO_SE,         6.8951f, -180.246f, -60.896f},    // north-west
};

static const float afSpawnLocations[2][3]=
{
    {-30.127f, -254.463f, -89.440f},                       // whelps
    {-30.817f, -177.106f, -89.258f}                        // whelps
};

static const float trans[2][3]=
{
   {-71.36f, -214.745f, -83.782f},                            // liftOff
   {-37.491f, -214.39f, -86.712f}                             // landing
};

struct MANGOS_DLL_DECL boss_onyxiaAI : public ScriptedAI
{
    boss_onyxiaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_onyxias_lair*)pCreature->GetInstanceData();
        m_uiMaxBreathPositions = sizeof(aMoveData)/sizeof(OnyxiaMove);
        Reset();
    }

    instance_onyxias_lair* m_pInstance;

    uint8 m_uiPhase;
    uint8 m_uiMaxBreathPositions;

    uint32 m_uiFlameBreathTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiTailSweepTimer;
    uint32 m_uiWingBuffetTimer;
    uint32 m_uiKnockAwayTimer;

    uint32 m_uiMovePoint;
    uint32 m_uiMovementTimer;
    uint32 m_uiMovementTicker;
    OnyxiaMove* m_pPointData;

    uint32 m_uiFireballTimer;
    uint32 m_uiSummonWhelpsTimer;
    uint32 m_uiBellowingRoarTimer;
    uint32 m_uiWhelpTimer;

    uint32 m_uiCheckLairTimer;

    uint32 m_uiLiftOffTimer;
    uint32 liftOffData;

    uint32 spellDeepBreath;
    uint32 m_uiToastTimer;
    uint32 m_uiToastAgainTimer;

    uint8 m_uiSummonCount;

    bool m_bIsSummoningWhelps;
    bool touchGround;
    bool fearMode;
    bool hasTank;
    bool stopMeleeAttacking;
    bool toastUnits;

    Unit* mTank;

    void Reset()
    {
        if (!IsCombatMovement())
            SetCombatMovement(true);

        m_creature->SetBoundingValue(0, 16);
        m_creature->SetBoundingValue(1, 16);

        m_uiPhase = PHASE_START;

        m_uiFlameBreathTimer    = urand(10000, 20000);
        m_uiTailSweepTimer      = urand(15000, 20000);
        m_uiCleaveTimer         = urand(2000, 5000);
        m_uiWingBuffetTimer     = urand(10000, 20000);
        m_uiKnockAwayTimer      = urand(20000, 30000);

        m_uiMovePoint           = urand(0, m_uiMaxBreathPositions - 1);
        m_uiMovementTimer       = 20000;
        m_pPointData            = GetMoveData();

        m_uiFireballTimer       = 5000;
        m_uiSummonWhelpsTimer   = 5000;
        m_uiBellowingRoarTimer  = 2000;                      // Immediately after landing
        m_uiWhelpTimer          = 1000;
        m_uiMovementTicker      = 1000;

        m_uiCheckLairTimer      = 0;

        m_uiLiftOffTimer		= 0;
        liftOffData				= 0;

        spellDeepBreath         = 0;
        m_uiToastTimer          = 0;
        m_uiToastAgainTimer     = 0;

        m_uiSummonCount         = 0;

        m_bIsSummoningWhelps    = false;
        touchGround             = false;
        fearMode                = false;
        hasTank                 = false;
        stopMeleeAttacking      = false;
        toastUnits              = false;

        mTank                   = 0;

        m_creature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
    }

    void Aggro(Unit* pWho)
    {
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        DoScriptText(SAY_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_ONYXIA, IN_PROGRESS);
    }

    void JustReachedHome()
    {
        m_creature->SetStandState(UNIT_STAND_STATE_SLEEP);
        m_creature->RemoveSplineFlag(SPLINEFLAG_FLYING);
        m_creature->SetSplineFlags(SPLINEFLAG_WALKMODE);
        m_creature->SetHover(false);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_ONYXIA, FAIL);
    }

    void JustDied(Unit* pKiller)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ONYXIA, DONE);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (!m_pInstance)
            return;

        pSummoned->SetInCombatWithZone();

        if (pSummoned->GetEntry() == NPC_ONYXIA_WHELP)
            ++m_uiSummonCount;
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(SAY_KILL, m_creature);
    }

    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell)
    {
        if (pSpell->Id == SPELL_WINGBUFFET)
        {
            uint8 uiThreatReduce = urand(5, 15);
            m_creature->getThreatManager().modifyThreatPercent(pTarget, -uiThreatReduce);
            return;
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        switch(pSpell->Id)
        {
            case SPELL_BREATH_EAST_TO_WEST:
            case SPELL_BREATH_WEST_TO_EAST:
            case SPELL_BREATH_SE_TO_NW:
            case SPELL_BREATH_NW_TO_SE:
            case SPELL_BREATH_SW_TO_NE:
            case SPELL_BREATH_NE_TO_SW:
            case SPELL_BREATH_SOUTH_TO_NORTH:
            case SPELL_BREATH_NORTH_TO_SOUTH:
            {

                if (m_pPointData = GetMoveData())
                {
                    if (!m_pInstance)
                        return;

                    if (Creature* pTrigger = m_pInstance->GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER))
                    {
                        m_creature->GetMap()->CreatureRelocation(m_creature, m_pPointData->fX, m_pPointData->fY, m_pPointData->fZ, m_creature->GetAngle(pTrigger));
                        m_creature->SendMonsterMove(m_pPointData->fX, m_pPointData->fY, m_pPointData->fZ, SPLINETYPE_FACINGTARGET, m_creature->GetSplineFlags(), 1, NULL, pTrigger->GetObjectGuid().GetRawValue());
                    }
                }
                break;
            }
        }
    }

    OnyxiaMove* GetMoveData()
    {
        for (uint32 i = 0; i < m_uiMaxBreathPositions; ++i)
        {
            if (aMoveData[i].uiLocId == m_uiMovePoint)
                return &aMoveData[i];
        }

        return NULL;
    }

    //function will teleport all players if they are to far away to the center of the fight
    void checkOnyxiasLair()
    {
        if (m_pInstance)
        {
            m_creature->SetInCombatWithZone();
            Map::PlayerList const &PlayerList = m_pInstance->instance->GetPlayers();
            for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
            {
                if (Creature* pCenter = m_pInstance->GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER))
                {
                    Player* pPlayer = itr->getSource();
                    if (pPlayer && pPlayer->isAlive() && !pCenter->IsWithinDistInMap(pPlayer, 100))
                        DoTeleportPlayer(pPlayer, pCenter->GetPositionX(), pCenter->GetPositionY(), pCenter->GetPositionZ(), 0); 
                }
            }
        }
    }

    Player* getRandomPlayerPhaseTwo()	
    {
        Player* randomPlayer = 0;
        if (m_pInstance)
        {
            std::list<Player*> possiblePlayer;
            Map::PlayerList const &PlayerList = m_pInstance->instance->GetPlayers();
            for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
            {
                Player* pPlayer = itr->getSource();
                if (pPlayer && !pPlayer->isGameMaster() && pPlayer->isAlive())
                    possiblePlayer.push_back(pPlayer);                   
            }

            if (!possiblePlayer.empty())
            {
                std::list<Player*>::iterator i = possiblePlayer.begin();
                advance(i, (rand() % possiblePlayer.size()));
                randomPlayer = (*i);
            }
        }

        return randomPlayer;
    }


    bool isAnyPlayerStillAlive()
    {
        bool alivePlayer = false;
        if (m_pInstance)
        {
            Map::PlayerList const &PlayerList = m_pInstance->instance->GetPlayers();
            for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
            {
                Player* pPlayer = itr->getSource();
                if (pPlayer && !pPlayer->isGameMaster() && pPlayer->isAlive())
                {
                    alivePlayer = true;
                    m_creature->SetInCombatWithZone();
                }
            }
        }

        return alivePlayer;
    }

    void MovementInform(uint32 uiMoveType, uint32 uiPointId)
    {
        if (uiMoveType != POINT_MOTION_TYPE || !m_pInstance)
            return;

        if (m_uiPhase == PHASE_BREATH_PRE || m_uiPhase == PHASE_BREATH)
        {
            if (Creature* pTrigger = m_pInstance->GetSingleCreatureFromStorage(NPC_ONYXIA_TRIGGER))
                m_creature->SetFacingToObject(pTrigger);

            m_creature->SendMeleeAttackStop(m_creature->getVictim());

            if (m_uiPhase == PHASE_BREATH_PRE)
            {
                stopMeleeAttacking = true;
                m_uiLiftOffTimer = 1000;
            }
        }

        if (m_uiPhase == PHASE_BREATH_POST)
            touchGround = true;
    }

    void AttackStart(Unit* pWho)
    {
        if (!stopMeleeAttacking)
        {
            if (pWho && m_creature->Attack(pWho, true))
            {
                m_creature->AddThreat(pWho);
                m_creature->SetInCombatWith(pWho);
                pWho->SetInCombatWith(m_creature);

            if (IsCombatMovement())
                m_creature->GetMotionMaster()->MoveChase(pWho);
            }

            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);
        }
        else
            m_creature->SendMeleeAttackStop(pWho);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        //enter evade mode is handled here
        if  (m_uiPhase == PHASE_START || m_uiPhase == PHASE_END)
        {
            //do this if she attacks -> she is at the ground
            if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;
        }
        else
        {
            //do this if she doesnt attack -> she is flying or is going to fly / touch ground
            if (!isAnyPlayerStillAlive())
            {
                if (!m_creature->IsInEvadeMode())
                    m_creature->AI()->EnterEvadeMode();

                return;
            }
        }

        //check onyias lair every 10 seconds
        if (m_uiCheckLairTimer < uiDiff)
        {
            checkOnyxiasLair();
            m_uiCheckLairTimer = 10000;
        }
        else
            m_uiCheckLairTimer -= uiDiff;

        switch (m_uiPhase)
        {
              //////////////////////
             ///////PHASE 3////////
            //////////////////////
            case PHASE_END:
            {
                // Fear Calculation
				if (fearMode && mTank)
				{
				    if (!(mTank->HasAura(SPELL_BELLOWINGROAR)))
					{
					    m_creature->getThreatManager().modifyThreatPercent(mTank,11000);
						fearMode = false;					
					}
				}
				else if (hasTank)
				{
					if (mTank && mTank->HasAura(SPELL_BELLOWINGROAR))
					{
						fearMode = true;
						m_creature->getThreatManager().modifyThreatPercent(mTank,-99);
					}
					hasTank = false;
				}

                if (m_uiBellowingRoarTimer < uiDiff)
                {
                    if (!fearMode)
					{
                        //get MainTank
						if (mTank = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))
						    hasTank = true;
					}

                    if (DoCastSpellIfCan(m_creature, SPELL_BELLOWINGROAR) == CAST_OK)
                        m_uiBellowingRoarTimer = 30000;
                }
                else
                    m_uiBellowingRoarTimer -= uiDiff;
                // no break, phase 3 will use same abilities as in 1
            }
              //////////////////////
             ///////PHASE 1////////
            //////////////////////
            case PHASE_START:
            {
                if (m_uiPhase == PHASE_START && m_creature->GetHealthPercent() < 65.0f)
                {
                    m_uiPhase = PHASE_BREATH_PRE;

                    DoScriptText(SAY_PHASE_2_TRANS, m_creature);

                    if (m_creature->isInRoots())
                        m_creature->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);

                    SetCombatMovement(false);
                    m_creature->GetMotionMaster()->MoveIdle();
                    m_creature->GetMotionMaster()->MovePoint(0, trans[liftOff][x], trans[liftOff][y], trans[liftOff][z]);

                    return;
                }

                //improved melee movement
                if (m_uiMovementTicker < uiDiff)
                {
                    if (m_creature->CanReachWithMeleeAttack(m_creature->getVictim()))
                    {
                        m_creature->SetFacingToObject(m_creature->getVictim());
                        if (!m_creature->isInRoots())
                            m_creature->CastSpell(m_creature, 23973, true);
                    }
                    else
                    {
                        if (m_creature->isInRoots())
                            m_creature->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);
                    }

                    m_uiMovementTicker = 1000;
                }
                else
                    m_uiMovementTicker -= uiDiff;

                if (m_uiFlameBreathTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FLAMEBREATH) == CAST_OK)
                        m_uiFlameBreathTimer = urand(10000, 20000);
                }
                else
                    m_uiFlameBreathTimer -= uiDiff;

                if (m_uiTailSweepTimer < uiDiff)
                {
                    m_creature->CastSpell(m_creature, SPELL_TAILSWEEP, true);
                    m_uiTailSweepTimer = urand(15000, 20000);
                }
                else
                    m_uiTailSweepTimer -= uiDiff;

                if (m_uiCleaveTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                        m_uiCleaveTimer = urand(2000, 5000);
                }
                else
                    m_uiCleaveTimer -= uiDiff;

                /*if (m_uiKnockAwayTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCK_AWAY) == CAST_OK)
                    {
                        m_creature->getThreatManager().modifyThreatPercent(m_creature->getVictim(), -25);
                        m_uiKnockAwayTimer = urand(15000, 30000);
                    }
                }
                else
                    m_uiKnockAwayTimer -= uiDiff;
		*/
                if (m_uiWingBuffetTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_WINGBUFFET) == CAST_OK)
                        m_uiWingBuffetTimer = urand(15000, 30000);
                }
                else
                    m_uiWingBuffetTimer -= uiDiff;

                DoMeleeAttackIfReady();
                break;
            }
              ////////////////////////////////////////////
             ///////PHASE 1 TO PHASE 2 (lift off)////////
            ////////////////////////////////////////////
            case PHASE_BREATH_PRE:
            {
                if (m_uiPhase == PHASE_BREATH_PRE && m_pInstance->GetData(TYPE_ONYXIA) == DATA_LIFTOFF)
                {
                    m_uiPhase = PHASE_BREATH;

                    if (m_pPointData)
                        m_creature->GetMotionMaster()->MovePoint(m_pPointData->uiLocId, m_pPointData->fX, m_pPointData->fY, m_pPointData->fZ,false);

                    return;
                }

                if (m_uiLiftOffTimer)
                {
                     if (m_uiLiftOffTimer <= uiDiff)
                     {
                        switch(liftOffData)
                        {
                            case 0:
                            {
                                m_creature->SetHover(true);
                                m_uiLiftOffTimer = 1000;
                                break;
                            }
                            case 1:
                            {
                                m_creature->AddSplineFlag(SPLINEFLAG_FLYING);

                                if (m_pInstance)
                                    m_pInstance->SetData(TYPE_ONYXIA, DATA_LIFTOFF);
                                break;
                            }
                        }
                        liftOffData++;
                     }
                     else
                        m_uiLiftOffTimer -= uiDiff;
                }
                break;
            }
              //////////////////////
             ///////PHASE 2////////
            //////////////////////
            case PHASE_BREATH:
            {
                if (m_creature->GetHealthPercent() < 40.0f)
                {
                    m_uiPhase = PHASE_BREATH_POST;
                    DoScriptText(SAY_PHASE_3_TRANS, m_creature);

                    m_creature->GetMotionMaster()->MovePoint(0, trans[landing][x], trans[landing][y], trans[landing][z], false);
                 
                    return;
                }

                //handle deep breath damage
                if (toastUnits)
                {
                    //time how long the lair is toasted
                    if (m_uiToastTimer > uiDiff)
                    {
                        //how often the player got hitted
                        if (m_uiToastAgainTimer <= uiDiff)
                        {
                            m_creature->CastSpell(m_creature->getVictim(), spellDeepBreath, true);
                            m_uiToastAgainTimer = 500;
                        }
                        else
                            m_uiToastAgainTimer -= uiDiff;

                        m_uiToastTimer -= uiDiff;
                    }
                    else
                        toastUnits = false;
                }

                if (m_uiMovementTimer < uiDiff)
                {
                    m_uiMovementTimer = urand(20000, 30000);

                    // 3 possible actions
                    switch(urand(0, 4))
                    {
                        // breath, maybe we need some other cases for deep breath, for example knubbel grp or to less dmg
                        case 0:                             
                            if (m_pPointData = GetMoveData())
                            {
                                //handle deep breath animation
                                DoScriptText(EMOTE_BREATH, m_creature);
                                //emote dont work therfore we actually do this
                                m_creature->MonsterYell("takes in a deep Breath...", LANG_UNIVERSAL);
                                spellDeepBreath = m_pPointData->uiSpellId;
                                DoCastSpellIfCan(m_creature, spellDeepBreath, CAST_INTERRUPT_PREVIOUS);
                                m_uiMovePoint = m_pPointData->uiLocIdEnd;

                                m_uiToastTimer = 1000;
                                toastUnits = true;
                            }
                            return;
                        // a point on the left side
                        case 1:
                        case 2:
                        {
                            m_uiMovePoint += m_uiMaxBreathPositions - 1;
                            m_uiMovePoint %= m_uiMaxBreathPositions;
                            break;
                        }
                        // a point on the right side
                        case 3:
                        case 4:
                            ++m_uiMovePoint %= m_uiMaxBreathPositions;
                            break;
                    }

                    if (m_pPointData = GetMoveData())
                        m_creature->GetMotionMaster()->MovePoint(m_pPointData->uiLocId, m_pPointData->fX, m_pPointData->fY, m_pPointData->fZ,false);
                }
                else
                    m_uiMovementTimer -= uiDiff;

                if (m_uiFireballTimer < uiDiff)
                {
                    //fireball completly random watch real classic onyxia movie
                    Unit* pRandom = getRandomPlayerPhaseTwo();
                    if (pRandom && pRandom->isAlive())
                    {
                        if (DoCastSpellIfCan(pRandom, SPELL_FIREBALL) == CAST_OK)
                            m_uiFireballTimer = 3000;
                    }
                }
                else
                    m_uiFireballTimer -= uiDiff;

                if (m_bIsSummoningWhelps)
                {
                    if (m_uiSummonCount < MAX_WHELPS_PER_PACK)
                    {
                        if (m_uiWhelpTimer < uiDiff)
                        {
                            float x1,y1,z1;
                            m_creature->GetRandomPoint(afSpawnLocations[0][x], afSpawnLocations[0][y], afSpawnLocations[0][z], 5.0f, x1, y1, z1);
                            m_creature->SummonCreature(NPC_ONYXIA_WHELP, afSpawnLocations[0][x], afSpawnLocations[0][y], afSpawnLocations[0][z], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                            m_creature->SummonCreature(NPC_ONYXIA_WHELP, x1, y1, z1, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                            m_creature->GetRandomPoint(afSpawnLocations[1][x], afSpawnLocations[1][y], afSpawnLocations[1][z], 5.0f, x1, y1, z1);
                            m_creature->SummonCreature(NPC_ONYXIA_WHELP, afSpawnLocations[1][x], afSpawnLocations[1][y], afSpawnLocations[1][z], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                            m_creature->SummonCreature(NPC_ONYXIA_WHELP, x1, y1, z1, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                            m_uiWhelpTimer = 2000;
                        }
                        else
                            m_uiWhelpTimer -= uiDiff;
                    }
                    else
                    {
                        m_bIsSummoningWhelps = false;
                        m_uiSummonCount = 0;
                        m_uiSummonWhelpsTimer = 80000;      // 90s - 10s for summoning
                    }
                }
                else
                {
                    if (m_uiSummonWhelpsTimer < uiDiff)
                        m_bIsSummoningWhelps = true;
                    else
                        m_uiSummonWhelpsTimer -= uiDiff;
                }

                break;
            }
              ///////////////////////////////////////////
             ///////PHASE 2 TO PHASE 3 (landing)////////
            ///////////////////////////////////////////
            case PHASE_BREATH_POST:
            {
                if (touchGround)
                {
                    m_uiPhase = PHASE_END;
               
                    m_creature->RemoveSplineFlag(SPLINEFLAG_FLYING);
                    m_creature->SetSplineFlags(SPLINEFLAG_WALKMODE);
                    m_creature->SetHover(false);

                    stopMeleeAttacking = false;
                    SetCombatMovement(true);
                    m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
                }
            }
        }
    }
};

CreatureAI* GetAI_boss_onyxia(Creature* pCreature)
{
    return new boss_onyxiaAI(pCreature);
}

void AddSC_boss_onyxia()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_onyxia";
    pNewScript->GetAI = &GetAI_boss_onyxia;
    pNewScript->RegisterSelf();
}
