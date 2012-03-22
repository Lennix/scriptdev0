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

/*
-- AV_Immunity_Settings --
update creature_template set mechanic_immune_mask = 617299803 where 
entry = 11946 or entry = 12121 or entry = 12122 or entry = 14770 or entry = 14771 or entry = 14772 or entry = 14773 or entry = 14774 or entry = 14775 or entry = 14776 or entry = 14777 
or entry = 11948 or entry = 14762 or entry = 14763 or entry = 14764 or entry = 14765 or entry = 14766 or entry = 14767 or entry = 14768 or entry = 14769
or entry = 11949 or entry = 11947
or entry = 13419 or entry = 13256;
*/

#include "precompiled.h"
#include "BattleGround.h"
#include "escort_ai.h"

enum Creatures
{
    NPC_NORTH_MARSHAL                             = 14762,
    NPC_SOUTH_MARSHAL                             = 14763,
    NPC_ICEWING_MARSHAL                           = 14764,
    NPC_STONEHEARTH_MARSHAL                       = 14765,
    NPC_EAST_FROSTWOLF_WARMASTER                  = 14772,
    NPC_ICEBLOOD_WARMASTER                        = 14773,
    NPC_TOWER_POINT_WARMASTER                     = 14776,
    NPC_WEST_FROSTWOLF_WARMASTER                  = 14777,
};

enum Faction
{
    FACTION_FROSTWOLF       = 729,
    FACTION_STORMPIKE       = 730,
};

enum Buffs
{
    BG_AV_WARCRY_BUFF_1     = 28418,
    BG_AV_WARCRY_BUFF_2     = 28419,
    BG_AV_WARCRY_BUFF_3     = 28420
};


const float centerX[2] = {722, -1372};
const float centerY[2] = {-11, -216};

enum Spells
{
    SPELL_CHARGE                                  = 22911,
    SPELL_CLEAVE                                  = 40504,
    SPELL_DEMORALIZING_SHOUT                      = 23511,
    SPELL_ENRAGE                                  = 8599,
    SPELL_WHIRLWIND1                              = 15589,
    SPELL_WHIRLWIND2                              = 13736,
    
    // Are there some preTBC equivalents ?
    SPELL_NORTH_MARSHAL                           = 45828,
    SPELL_SOUTH_MARSHAL                           = 45829,
    SPELL_STONEHEARTH_MARSHAL                     = 45830,
    SPELL_ICEWING_MARSHAL                         = 45831,
    SPELL_ICEBLOOD_WARMASTER                      = 45822,
    SPELL_TOWER_POINT_WARMASTER                   = 45823,
    SPELL_WEST_FROSTWOLF_WARMASTER                = 45824,
    SPELL_EAST_FROSTWOLF_WARMASTER                = 45826,
};

struct MANGOS_DLL_DECL mob_av_marshal_or_warmasterAI : public ScriptedAI
{
    mob_av_marshal_or_warmasterAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    bool m_bHasAura;

    uint32 m_uiChargeTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiDemoralizingShoutTimer;
    uint32 m_uiWhirlwind1Timer;
    uint32 m_uiWhirlwind2Timer;
    uint32 m_uiEvadeTimer;

    void Reset()
    {
        m_bHasAura = false;

        m_uiChargeTimer = urand(2*IN_MILLISECONDS,12*IN_MILLISECONDS);
        m_uiCleaveTimer = urand(1*IN_MILLISECONDS,11*IN_MILLISECONDS);
        m_uiDemoralizingShoutTimer = urand(2*IN_MILLISECONDS,2*IN_MILLISECONDS);
        m_uiWhirlwind1Timer = urand(1*IN_MILLISECONDS,12*IN_MILLISECONDS);
        m_uiWhirlwind2Timer = urand(5*IN_MILLISECONDS,20*IN_MILLISECONDS);
        m_uiEvadeTimer = 5*IN_MILLISECONDS;
    }

    void Aggro(Unit*)
    {
        m_creature->CallForHelp(30.0f);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        /*if (!m_bHasAura)
        {
            switch(m_creature->GetEntry())
            {
                case NPC_NORTH_MARSHAL:
                    DoCastSpellIfCan(m_creature, SPELL_NORTH_MARSHAL);
                    break;
                case NPC_SOUTH_MARSHAL:
                    DoCastSpellIfCan(m_creature, SPELL_SOUTH_MARSHAL);
                    break;
                case NPC_STONEHEARTH_MARSHAL:
                    DoCastSpellIfCan(m_creature, SPELL_STONEHEARTH_MARSHAL);
                    break;
                case NPC_ICEWING_MARSHAL:
                    DoCastSpellIfCan(m_creature, SPELL_ICEWING_MARSHAL);
                    break;
                case NPC_EAST_FROSTWOLF_WARMASTER:
                    DoCastSpellIfCan(m_creature, SPELL_EAST_FROSTWOLF_WARMASTER);
                    break;
                case NPC_WEST_FROSTWOLF_WARMASTER:
                    DoCastSpellIfCan(m_creature, SPELL_WEST_FROSTWOLF_WARMASTER);
                    break;
                case NPC_ICEBLOOD_WARMASTER:
                    DoCastSpellIfCan(m_creature, SPELL_ICEBLOOD_WARMASTER);
                    break;
                case NPC_TOWER_POINT_WARMASTER:
                    DoCastSpellIfCan(m_creature, SPELL_TOWER_POINT_WARMASTER);
                    break;
            }

            m_bHasAura = true;
        }*/

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Enrage
        if (HealthBelowPct(40))
            DoCastSpellIfCan(m_creature, SPELL_ENRAGE, CAST_AURA_NOT_PRESENT);

        // Charge
        if (m_uiChargeTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_CHARGE);
            m_uiChargeTimer = urand(10*IN_MILLISECONDS,25*IN_MILLISECONDS);
        }
        else
            m_uiChargeTimer -= uiDiff;

        // Cleave
        if (m_uiCleaveTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE);
            m_uiCleaveTimer =  urand(10*IN_MILLISECONDS,16*IN_MILLISECONDS);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        // Demoralizing Shout
        if (m_uiDemoralizingShoutTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_DEMORALIZING_SHOUT);
            m_uiDemoralizingShoutTimer = urand(10*IN_MILLISECONDS,15*IN_MILLISECONDS);
        }
        else
            m_uiDemoralizingShoutTimer -= uiDiff;

        // Whirlwind 1
        if (m_uiWhirlwind1Timer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_WHIRLWIND1);
            m_uiWhirlwind1Timer = urand(6*IN_MILLISECONDS,20*IN_MILLISECONDS);
        }
        else
            m_uiWhirlwind1Timer -= uiDiff;

        // Whirlwind 1
        if (m_uiWhirlwind2Timer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_WHIRLWIND2);
            m_uiWhirlwind2Timer = urand(10*IN_MILLISECONDS,25*IN_MILLISECONDS);
        }
        else
            m_uiWhirlwind2Timer -= uiDiff;

        // Check if creature is not outside of building
        if (m_uiEvadeTimer <= uiDiff)
        {
            uint8 team;
            if (m_creature->getFaction() == FACTION_STORMPIKE)
                team = 0;
            else
                team = 1;

            if (m_creature->GetDistance2d(centerX[team], centerY[team]) > 30.0f)
                EnterEvadeMode();
            m_uiEvadeTimer = 5*IN_MILLISECONDS;
        }
        else
            m_uiEvadeTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_av_marshal_or_warmaster(Creature* pCreature)
{
    return new mob_av_marshal_or_warmasterAI(pCreature);
}

//TEAM SMITHS !!!
/*DATABASE:
 delete from creature_battleground where guid = 150102;
 INSERT INTO `creature_battleground` VALUES ('150102', '66', '0');
 delete from battleground_events where event1 = 65;
 INSERT INTO `battleground_events` VALUES ('30', '65', '0', 'Alliance Smith');
 delete from battleground_events where event1 = 66;^^
 INSERT INTO `battleground_events` VALUES ('30', '66', '0', 'Horde Smith');
 delete from creature_battleground where guid = 150107;
 INSERT INTO `creature_battleground` VALUES ('150107', '65', '0');
 UPDATE creature_template SET ScriptName = 'BG_AV_TeamSmith' WHERE entry = '13176';
 UPDATE creature_template SET ScriptName = 'BG_AV_TeamSmith' WHERE entry = '13257';
 */

struct MANGOS_DLL_DECL BG_AV_TeamSmith : public ScriptedAI
{
    BG_AV_TeamSmith(Creature* pCreature) : ScriptedAI(pCreature)
    {   
        //remove gossip until the core will handle it
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        Reset();
    }

    //maybe we will need additional code here!

    void Reset() {}
};

bool GossipHello_BG_AV_TeamSmith(Player* pPlayer, Creature* pCreature)
{
    //u need enough reputation to speak with him, this reputation increased with each buff level
    bool canTalkWithSmith = false;
    uint8 reputationUNeed = 0;
    GossipIndex Channel;
    if (pPlayer->GetTeam() == ALLIANCE)
        Channel = GOSSIP_CHANNEL1;
    else
        Channel = GOSSIP_CHANNEL2;

    if (BattleGround *bg = pPlayer->GetBattleGround())
    {
        switch(bg->GetGossipStatus(Channel))
        {
            case STATUS_WAIT_ACTION: reputationUNeed = REP_HONORED; break;
            case STATUS_ACTION1: reputationUNeed = REP_REVERED; break;
            case STATUS_ACTION2: reputationUNeed = REP_EXALTED; break;
        }
    }

    if (pPlayer->GetReputationRank(FACTION_STORMPIKE) >= reputationUNeed || pPlayer->GetReputationRank(FACTION_FROSTWOLF) >= reputationUNeed)
        canTalkWithSmith = true;

    if (canTalkWithSmith && pCreature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
    {
        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, "Upgrade now !!!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        //wrong textid
        pPlayer->SEND_GOSSIP_MENU(6066, pCreature->GetObjectGuid());

        return true;
    }
    else
        return false;
}

bool GossipSelect_BG_AV_TeamSmith(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            pPlayer->CLOSE_GOSSIP_MENU();
            if (BattleGround *bg = pPlayer->GetBattleGround())
            {
                GossipIndex Channel;
                GossipStatus Status;
                if (pPlayer->GetTeam() == ALLIANCE)
                    Channel = GOSSIP_CHANNEL1;
                else
                    Channel = GOSSIP_CHANNEL2;

                switch(bg->GetGossipStatus(Channel))
                {
                    case STATUS_WAIT_ACTION: Status = STATUS_ACTION1; break;
                    case STATUS_ACTION1: Status = STATUS_ACTION2; break;
                    case STATUS_ACTION2: Status = STATUS_ACTION3; break;
                }
                pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                bg->SetGossipStatus(Channel, Status, BATTLEGROUND_AV); 
            }
            break;
        }
    }
    return true;
}

CreatureAI* GetAI_BG_AV_TeamSmith(Creature* pCreature)
{
    return new BG_AV_TeamSmith(pCreature);
}

/*
DATABASE:
-- HORDE WAYPOINTS --
delete from script_waypoint where entry = 13236;
INSERT INTO `script_waypoint` VALUES (13236, '0', -1319, -342, 60, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '1', -1315, -346, 60, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '2', -1280, -352, 61, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '3', -1248, -342, 59, '0', 'aufmounten!');
INSERT INTO `script_waypoint` VALUES (13236, '4', -1240, -368, 59, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '5', -1198, -366, 53, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '6', -1139, -349, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '7', -1092, -368, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '8', -1062, -363, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '9', -1043, -385, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '10', -1003, -399, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '11', -898, -382, 48, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '12', -844, -393, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '13', -812, -447, 54, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '14', -760, -429, 64, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '15', -718, -408, 67, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '16', -711, -364, 66, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '17', -669, -381, 64, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '18', -625, -396, 58, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '19', -616, -346, 55, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '20', -579, -313, 46, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '21', -566, -285, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '22', -542, -251, 55, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '23', -537, -232, 56, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '24', -494, -205, 57, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '25', -480, -198, 56, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '26', -426, -199, 26, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '27', -418, -165, 23, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '28', -421, -125, 23, '0', '');
INSERT INTO `script_waypoint` VALUES (13236, '29', -370, -127, 26, '0', 'abmounten!');

-- ALLIANZ WAYPOINTS --
delete from script_waypoint where entry = 13442;
INSERT INTO `script_waypoint` VALUES (13442, '0', 725, -80, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '1', 721, -87, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '2', 704, -101, 52, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '3', 667, -106, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '4', 652, -99, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '5', 644, -91, 48, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '6', 629, -101, 40, '0', 'aufmounten!');
INSERT INTO `script_waypoint` VALUES (13442, '7', 618, -138, 33, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '8', 624, -190, 38, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '9', 630, -229, 37, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '10', 635, -267, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '11', 635, -300, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '12', 623, -323, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '13', 608, -335, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '14', 582, -332, 29, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '15', 543, -321, 6, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '16', 518, -324, -1, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '17', 459, -372, -1, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '18', 377, -394, -1, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '19', 309, -381, 0, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '20', 289, -384, 5, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '21', 272, -394, 14, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '22', 257, -408, 28, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '23', 249, -413, 33, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '24', 227, -419, 39, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '25', 200, -409, 42, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '26', 153, -397, 42, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '27', 120, -377, 43, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '28', 93, -391, 45, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '29', 28, -392, 45, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '30', -5, -418, 45, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '31', -16, -414, 42, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '32', -35, -411, 32, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '33', -44, -396, 20, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '34', -55, -370, 12, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '35', -91, -389, 14, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '36', -116, -378, 12, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '37', -183, -362, 6, '0', '');
INSERT INTO `script_waypoint` VALUES (13442, '38', -195, -347, 6, '0', 'abmounten!');

-- SET SCRIPTS --
update creature_template set ScriptName = "mob_AV_BossSummonerMaster" where entry = 13236;
update creature_template set ScriptName = "mob_AV_BossSummonerAdd" where entry = 13284;
update creature_template set ScriptName = "mob_AV_BossSummonerMaster" where entry = 13442;
update creature_template set ScriptName = "mob_AV_BossSummonerAdd" where entry = 13443;

-- SET OBJECTS --
delete from gameobject where guid = 632777 and id = 178465;
INSERT INTO `gameobject` VALUES ('632777', '178465', '30', '-366.143', '-130.525', '26.4224', '5.50564', '0', '0', '0.379054', '-0.925375', '-25', '100', '1');
delete from gameobject where guid = 632888 and id = 178670;
INSERT INTO `gameobject` VALUES ('632888', '178670', '30', '-199.538', '-343.331', '6.79235', '1.95407', '0', '0', '0.828843', '0.559481', '-25', '100', '1');
*/
const uint32 masterModellId[2] = {14578, 14331};
const uint32 masterModellSpell[2] = {23249, 23221};
const uint32 addModellId[2] = {10278, 6444};
const uint32 addModellSpell[2] = {6746, 16056};
const uint32 masterAddId[2] = {13284, 13443};
const uint32 masterId[2] = {13236, 13442};
const uint32 object[2] = {178465, 178670};
const uint32 mountPosition[2] = {3, 6};
const uint32 dismountPosition[2] = {29, 38};

struct SummonLocation
{
    float x, y, z;
};

const SummonLocation spawnPoint[2] =
{
    {-370.0f, -128.0f, 26.0f},
    {-196.0f, -346.0f, 6.0f}   
};

const uint32 masterAttackSpell[2] = {15234, 22206};
const uint32 masterHealSpell[2] = {15982, 15981};
const uint32 masterSupportSpell[2] = {15786,22127};

enum master0_additional_spells
{
    Kettenblitz     = 16006,
    Flammenschock   = 15616,
    Zaubersperre    = 19647
};

struct MANGOS_DLL_DECL mob_AV_BossSummonerMaster : public npc_escortAI
{
    mob_AV_BossSummonerMaster(Creature* creature) : npc_escortAI(creature) 
    { 
        m_creature->SetMaxPower(POWER_MANA, 42189);
        m_creature->SetPower(POWER_MANA, m_creature->GetMaxPower(POWER_MANA));
        Reset(); 
    }

    uint32 m_uiAttackTimer;
    uint32 m_uiHealTimer;
    uint32 m_uiSupportTimer;
    uint32 updateMovementTimer;
    uint32 m_uiBlitzTimer;
    uint32 m_uiSchockTimer;
    uint32 m_uiSperreTimer;

    int id;

    std::list<Creature*> masterAdds;

	void Reset() 
    {
        m_uiSupportTimer = 3000;
        m_uiAttackTimer = 5000;
        m_uiSupportTimer = 8000;
        updateMovementTimer = 1000;
        m_uiSchockTimer = 8000;
        m_uiBlitzTimer = 5000;
        m_uiSperreTimer = 1000;

        if (m_creature->GetEntry() == masterId[0])
            id = 0;
        else
            id = 1;
    }

    void Aggro(Unit*) 
    { 
        m_creature->CallForHelp(20.0f);
        if (HasEscortState(STATE_ESCORT_ESCORTING) && !HasEscortState(STATE_ESCORT_PAUSED))
            Dismount(); 
    }

    void EnterEvadeMode()
    {
        npc_escortAI::EnterEvadeMode();
        GetCreatureListWithEntryInGrid(masterAdds, m_creature, masterAddId[id], 100.0f);
        if (HasEscortState(STATE_ESCORT_ESCORTING) && !HasEscortState(STATE_ESCORT_PAUSED))
            Mount();
    }

    void Mount()
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_creature->Mount(masterModellId[id], masterModellSpell[id]);
            mountAdds(true);
        }
    }

    void mountAdds(bool mount)
    {
        float pos = 0.0f;
        for(std::list<Creature*>::iterator itr = masterAdds.begin(); itr != masterAdds.end(); ++itr)
		{   
            pos += 1.0f;
            if ((*itr) && (*itr)->isAlive())
            {
                (*itr)->GetMotionMaster()->Clear();
                (*itr)->GetMotionMaster()->MoveFollow(m_creature, 3.0f, (1.0f + pos));
                (*itr)->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
                (*itr)->SetSpeedRate(MOVE_RUN, 1.4f);

                if (mount)
                    (*itr)->Mount(addModellId[id], addModellSpell[id]);
            }
        }
    }

    void Dismount()
    {
        m_creature->Unmount();
        m_creature->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
        for(std::list<Creature*>::iterator itr = masterAdds.begin(); itr != masterAdds.end(); ++itr)
		{
            if ((*itr) && (*itr)->isAlive())
            {
                (*itr)->Unmount();
                (*itr)->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
            }
        }
    }

    void createNewSpawnPoints()
    {
        CreatureCreatePos pos(m_creature->GetMap(), m_creature->GetPositionX(), m_creature->GetPositionY(),m_creature->GetPositionZ(), m_creature->GetOrientation());
        m_creature->SetSummonPoint(pos);
        for(std::list<Creature*>::iterator itr = masterAdds.begin(); itr != masterAdds.end(); ++itr)
		{
            if ((*itr) && (*itr)->isAlive())
            {
                CreatureCreatePos pos2((*itr)->GetMap(), (*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), (*itr)->GetOrientation());
                (*itr)->SetSummonPoint(pos2);
            }
        }
    }

    void WaypointReached(uint32 uiPointId)
	{
        //start mount
        if(mountPosition[id] == uiPointId)
            Mount();

        //end mount
        if (dismountPosition[id] == uiPointId)
        {
            Dismount();
            //new spawn points
            createNewSpawnPoints();
            //spawn summon object
            if (GameObject* summonObject = GetClosestGameObjectWithEntry(m_creature, object[id], 20.0f))
                summonObject->SetRespawnTime(0);
            //stop escort
            SetEscortPaused(true);
        }
	}

    void UpdateAI(const uint32 uiDiff)
	{
        //PHASE 3: Defending summon object point
        //only return if we are in phase 3 cause we need the escort movement below
        if ((!m_creature->SelectHostileTarget() || !m_creature->getVictim()) && HasEscortState(STATE_ESCORT_PAUSED))
            return;

        //combat check for phase 2 while the escort is in progress, cause we cant return here!
        if (m_creature->isInCombat())
        {
            if (m_uiHealTimer <= uiDiff)
            {
                if (Unit* pUnit = DoSelectLowestHpFriendly(60.0f, 1))
                {
                    if (DoCastSpellIfCan(pUnit, masterAttackSpell[id]) == CAST_OK)
                        m_uiHealTimer = urand(5000, 10000);
                }
            }
            else
                m_uiHealTimer -= uiDiff;

            if (m_uiAttackTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), masterAttackSpell[id]) == CAST_OK)
                    m_uiAttackTimer = urand(5000, 10000);
            }
            else
                m_uiAttackTimer -= uiDiff;

            if (m_uiSupportTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), masterSupportSpell[id]) == CAST_OK)
                    m_uiSupportTimer = urand(5000, 10000);
            }
            else
                m_uiSupportTimer -= uiDiff;

            //additional Spells for master 0
            if (!id)
            {
                if (m_uiBlitzTimer <= uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), Kettenblitz) == CAST_OK)
                        m_uiBlitzTimer = urand(5000, 10000);
                }
                else
                    m_uiBlitzTimer -= uiDiff;

                if (m_uiSchockTimer <= uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), Flammenschock) == CAST_OK)
                        m_uiSchockTimer = urand(5000, 10000);
                }
                else
                    m_uiSchockTimer -= uiDiff;

                if (m_uiSperreTimer <= uiDiff)
                {
                    if (m_creature->getVictim()->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    {
                        m_creature->CastSpell(m_creature->getVictim(), Zaubersperre, true);
                        m_uiSperreTimer = urand(5000, 10000);
                    }
                }
                else
                    m_uiSperreTimer -= uiDiff;
            }

            DoMeleeAttackIfReady();
        }

        //PHASE 2: Escorting
        if (HasEscortState(STATE_ESCORT_PAUSED))
            return;

        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            //handle group escorting
            if (updateMovementTimer <= uiDiff && !m_creature->isInCombat())
            {
                m_creature->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
                m_creature->SetSpeedRate(MOVE_RUN, 1.4f);
                mountAdds(false);
                updateMovementTimer = 2000;
            }
            else
                updateMovementTimer -= uiDiff;

            npc_escortAI::UpdateAI(uiDiff);
        }

        //PHASE 1: Start escorting
        if (HasEscortState(STATE_ESCORT_ESCORTING) || m_creature->isInCombat())
            return;

        GetCreatureListWithEntryInGrid(masterAdds, m_creature, masterAddId[id], 100.0f);
        mountAdds(false);
	    Start(false);
    }
};

static CreatureAI* GetAI_mob_AV_BossSummonerMaster(Creature* creature)
{
    return new mob_AV_BossSummonerMaster(creature);
}

const uint32 addBuff[2] = {12550, 22128};           //Blitzschlagschild, Dornen
const uint32 addAttackSpell[2] = {21401, 21668};    //Frostschock, Sternenfeuer
const uint32 addSupportSpell[2] = {12492, 22127};   //Welle der Heilung, Wucherwurzeln

struct MANGOS_DLL_DECL BG_AV_BossSummonerAdd : public ScriptedAI
{
    BG_AV_BossSummonerAdd (Creature* pCreature) : ScriptedAI(pCreature) 
    { 
        m_creature->SetMaxPower(POWER_MANA, 26432);
        m_creature->SetPower(POWER_MANA, m_creature->GetMaxPower(POWER_MANA));
        Reset(); 
    }

    uint32 attackTimer;
    uint32 supportTimer;

    int id;

    void Reset() 
    {
        attackTimer = 1000;
        supportTimer = 8000;
        if (m_creature->GetEntry() == masterAddId[0])
            id = 0;
        else
            id = 1;

        m_creature->CastSpell(m_creature, addBuff[id], true);
    }

    void Aggro(Unit*) { m_creature->CallForHelp(20.0f); }

    void EnterEvadeMode()
    {
        m_creature->RemoveAllAuras();
        m_creature->DeleteThreatList();
        m_creature->CombatStop(true);
        m_creature->SetLootRecipient(NULL);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (attackTimer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), addAttackSpell[id]) == CAST_OK)
                attackTimer = urand(5000, 10000);
        }
        else
            attackTimer -= uiDiff;

        if (supportTimer <= uiDiff)
        {
            if (id)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), addSupportSpell[id]) == CAST_OK)
                    supportTimer = urand (5000, 10000);
            }
            else
            {
                if (Unit* pUnit = DoSelectLowestHpFriendly(60.0f, 1))
                {
                    if (DoCastSpellIfCan(pUnit, addSupportSpell[id]) == CAST_OK)
                        supportTimer = urand (5000, 10000);
                }
            }
        }
        else
            supportTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

static CreatureAI* GetAI_BG_AV_BossSummonerAdd(Creature* creature)
{
    return new BG_AV_BossSummonerAdd(creature);
}

void AddSC_alterac_valley()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "mob_av_marshal_or_warmaster";
    pNewScript->GetAI = &GetAI_mob_av_marshal_or_warmaster;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "BG_AV_TeamSmith";
    pNewScript->GetAI = &GetAI_BG_AV_TeamSmith;
    pNewScript->pGossipHello = &GossipHello_BG_AV_TeamSmith;
    pNewScript->pGossipSelect = &GossipSelect_BG_AV_TeamSmith;
    pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "mob_AV_BossSummonerMaster";
	pNewScript->GetAI = &GetAI_mob_AV_BossSummonerMaster;
	pNewScript->RegisterSelf();

    pNewScript = new Script;
	pNewScript->Name = "mob_AV_BossSummonerAdd";
	pNewScript->GetAI = &GetAI_BG_AV_BossSummonerAdd;
	pNewScript->RegisterSelf();
}
