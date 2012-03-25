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
DATABASE^^
-- AV_Immunity_Settings --
update creature_template set mechanic_immune_mask = 617299803 where 
entry = 11946 or entry = 12121 or entry = 12122 or entry = 14770 or entry = 14771 or entry = 14772 or entry = 14773 or entry = 14774 or entry = 14775 or entry = 14776 or entry = 14777 
or entry = 11948 or entry = 14762 or entry = 14763 or entry = 14764 or entry = 14765 or entry = 14766 or entry = 14767 or entry = 14768 or entry = 14769
or entry = 11949 or entry = 11947
or entry = 13419 or entry = 13256;

-- AV_Disable_Pathfinding --
update creature_template set flags_extra = 4096 where entry = 13140 or entry = 13154 or entry = 13597 or entry = 13144 or entry = 13147 or entry = 13296  or entry = 13298;
*/
#include "precompiled.h"
#include "alterac_valley.h"
#include "escort_ai.h"
#include "BattleGround.h"

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

static const uint32 Data[2] = {EVENT_ENDBOSS_STATUS_A, EVENT_ENDBOSS_STATUS_H};

enum Spells
{
    SPELL_CHARGE                                  = 22911,
    SPELL_CLEAVE                                  = 40504,
    SPELL_DEMORALIZING_SHOUT                      = 23511,
    SPELL_ENRAGE                                  = 8599,
    SPELL_WHIRLWIND1                            = 15589,
    SPELL_WHIRLWIND2                            = 13736,
    
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
    mob_av_marshal_or_warmasterAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
        m_pInstance = (instance_BG_AV*)pCreature->GetInstanceData();
        Reset();
    }
    
    instance_BG_AV* m_pInstance;

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
        m_creature->CallForHelp(50.0f);
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
            if (m_creature->getFaction() != 1214)
                team = 0;
            else
                team = 1;

            if (m_pInstance->GetData(Data[team]) == FAIL)
                EnterEvadeMode();

            m_uiEvadeTimer = 2*IN_MILLISECONDS;
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
 delete from battleground_events where event1 = 66;
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
update creature_template set ScriptName = "mob_AV_BossSummonerMaster" where entry = 13236 or entry = 13442;
update creature_template set ScriptName = "mob_AV_BossSummonerAdd" where entry = 13443 or entry = 13284;
update creature_template set ScriptName = "mob_AV_Boss" where entry = 13256 or entry = 13419;

-- SET OBJECTS --
delete from gameobject where guid = 632777 and id = 178465;
INSERT INTO `gameobject` VALUES ('632777', '178465', '30', '-366.143', '-130.525', '26.4224', '5.50564', '0', '0', '0.379054', '-0.925375', '-25', '100', '1');
delete from gameobject where guid = 632888 and id = 178670;
INSERT INTO `gameobject` VALUES ('632888', '178670', '30', '-199.538', '-343.331', '6.79235', '1.95407', '0', '0', '0.828843', '0.559481', '-25', '100', '1');

-- SET EVENTSCRIPTS --
delete from scripted_event_id where id = 7060 or id = 7268;
INSERT INTO `scripted_event_id` VALUES ('7060', 'event_spell_BG_AV_BOSS');
INSERT INTO `scripted_event_id` VALUES ('7268', 'event_spell_BG_AV_BOSS');

-- SET EVENTCORE --
 delete from creature_battleground where guid = 150112;
 INSERT INTO `creature_battleground` VALUES ('150112', '75', '0');
 delete from battleground_events where event1 = 75;
 INSERT INTO `battleground_events` VALUES ('30', '75', '0', 'Alliance Boss Summon Master');
 delete from battleground_events where event1 = 76;
 INSERT INTO `battleground_events` VALUES ('30', '76', '0', 'Horde Boss Summon Master');
 delete from creature_battleground where guid = 150106;
 INSERT INTO `creature_battleground` VALUES ('150106', '76', '0');
*/

static const uint32 masterModellId[2] = {14578, 14331};
static const uint32 masterModellSpell[2] = {23249, 23221};
static const uint32 addModellId[2] = {10278, 6444};
static const uint32 addModellSpell[2] = {6746, 16056};
static const uint32 masterAddId[2] = {13284, 13443};
static const uint32 masterId[2] = {13236, 13442};
static const uint32 object[2] = {178465, 178670};
static const uint32 mountPosition[2] = {3, 6};
static const uint32 dismountPosition[2] = {29, 38};
static const uint32 spellSummonObject[2] = {21267, 21646};

static const uint32 masterAttackSpell[2] = {15234, 22206};
static const uint32 masterHealSpell[2] = {15982, 15981};
static const uint32 masterSupportSpell[2] = {15786,22127};

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
        //core will handle this
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_OUTDOORPVP);

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
        m_creature->SetMaxPower(POWER_MANA, 42189);
        m_creature->SetPower(POWER_MANA, m_creature->GetMaxPower(POWER_MANA));

        m_pInstance = (instance_BG_AV*)creature->GetInstanceData();
        Reset(); 
    }

    instance_BG_AV* m_pInstance;

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
                //keep adds close at their master
                if ((*itr)->GetDistance(m_creature) > 5.0f)
                    (*itr)->SetSpeedRate(MOVE_RUN, 1.5f);
                else
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

    void WaypointReached(uint32 uiPointId)
	{
        //start mount
        if(mountPosition[id] == uiPointId)
            Mount();

        //end mount
        if (dismountPosition[id] == uiPointId)
        {
            Dismount();
            //stop escort
            SetEscortPaused(true);
            //new spawn point
            CreatureCreatePos pos(m_creature->GetMap(), m_creature->GetPositionX(), m_creature->GetPositionY(),m_creature->GetPositionZ(), m_creature->GetOrientation());
            m_creature->SetSummonPoint(pos);
            //spawn summon object
            if (GameObject* summonObject = GetClosestGameObjectWithEntry(m_creature, object[id], 20.0f))
            {
                DoCastSpellIfCan(m_creature, spellSummonObject[id]);
                summonObject->SetRespawnTime(0);
            }
            //set global data
            uint32 Data;
            id == 0 ? Data = EVENT_MASTERS_START_SUMMONING_H : Data = EVENT_MASTERS_START_SUMMONING_A;
            m_pInstance->SetData(Data, IN_PROGRESS);
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
        if (HasEscortState(STATE_ESCORT_ESCORTING) || !m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_OUTDOORPVP) || m_creature->isInCombat())
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

static const uint32 addBuff[2] = {12550, 22128};           //Blitzschlagschild, Dornen
static const uint32 addAttackSpell[2] = {21401, 21668};    //Frostschock, Sternenfeuer
static const uint32 addSupportSpell[2] = {12492, 22127};   //Welle der Heilung, Wucherwurzeln

struct MANGOS_DLL_DECL BG_AV_BossSummonerAdd : public ScriptedAI
{
    BG_AV_BossSummonerAdd (Creature* pCreature) : ScriptedAI(pCreature) 
    { 
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
        m_creature->SetMaxPower(POWER_MANA, 26432);
        m_creature->SetPower(POWER_MANA, m_creature->GetMaxPower(POWER_MANA));

        m_pInstance = (instance_BG_AV*)pCreature->GetInstanceData();
        Reset(); 
    }

    instance_BG_AV* m_pInstance;

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

        //follow your master while we are defending the summon object
        uint32 Data = 0;
        id == 0 ? Data = m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_H) : Data = m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_A);
        if (Data >= IN_PROGRESS)
        {
            Creature* myMaster = GetClosestCreatureWithEntry(m_creature, masterId[id], 300.0f);
            if (myMaster && myMaster->isAlive())
                m_creature->GetMotionMaster()->MoveFollow(myMaster, urand(3, 8), urand(0, 10));
        }

        Reset();
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE + UNIT_FLAG_NOT_SELECTABLE))
        {
            CreatureCreatePos pos(m_creature->GetMap(), m_creature->GetPositionX(), m_creature->GetPositionY(),m_creature->GetPositionZ(), m_creature->GetOrientation());
            m_creature->SetSummonPoint(pos);
        }

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

/*
DATENBANK:
-- HORDE BOSS WAYPOINTS --
delete from script_waypoint where entry = 13256;
INSERT INTO `script_waypoint` VALUES (13256, '0', -291, -198, 8, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '1', -229, -238, 6, '0', 'wait 10 minutes');
INSERT INTO `script_waypoint` VALUES (13256, '2', -174, -233, 10, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '3', -110, -262, 6, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '4', -53, -235, 10, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '5', -28, -231, 9, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '6', 4, -243, 11, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '7', 72, -244, 16, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '8', 111, -339, 40, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '9', 125, -371, 42, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '10', 143, -392, 42, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '11', 200, -410, 42, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '12', 230, -419, 38, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '13', 251, -412, 32, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '14', 274, -392, 11, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '15', 297, -382, 2, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '16', 383, -391, -1, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '17', 462, -369, -1, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '18', 498, -336, -1, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '19', 520, -324, 0, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '20', 546, -322, 8, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '21', 581, -332, 29, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '22', 601, -338, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '23', 623, -323, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '24', 635, -299, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '25', 635, -267, 30, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '26', 630, -231, 37, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '27', 624, -189, 38, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '28', 620, -154, 33, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '29', 618, -131, 33, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '30', 629, -98, 40, '0', '');
INSERT INTO `script_waypoint` VALUES (13256, '31', 634, -48, 42, '0', '');

-- ALLIANCE BOSS WAYPOINTS --
delete from script_waypoint where entry = 13419;
INSERT INTO `script_waypoint` VALUES (13419, '0', -273, -294, 6, '0', 'wait 10 minutes');
INSERT INTO `script_waypoint` VALUES (13419, '1', -402, -283, 13, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '2', -441, -277, 20, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '3', -487, -284, 28, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '4', -523, -341, 34, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '5', -544, -339, 37, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '6', -578, -315, 45, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '7', -620, -352, 55, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '8', -624, -395, 58, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '9', -671, -379, 65, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '10', -712, -364, 66, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '11', -718, -408, 67, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '12', -760, -429, 64, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '13', -812, -447, 54, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '14', -844, -393, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '15', -898, -382, 48, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '16', -1003, -399, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '17', -1043, -385, 50, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '18', -1062, -363, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '19', -1092, -368, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '20', -1139, -349, 51, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '21', -1198, -366, 53, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '22', -1240, -368, 59, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '23', -1248, -342, 59, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '24', -1231, -316, 61, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '25', -1212, -294, 70, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '26', -1194, -273, 72, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '27', -1206, -253, 72, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '28', -1241, -250, 73, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '29', -1262, -279, 74, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '30', -1279, -289, 87, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '31', -1293, -289, 90, '0', '');
INSERT INTO `script_waypoint` VALUES (13419, '32', -1343, -294, 91, '0', '');
*/
enum event_summon_boss
{
    EVENT_LOKHOLAR      = 7060,
    EVENT_IVUS          = 7268,

    CALL_LOKHOLAR       = 21287,
    VISUAL_IVUS         = 21649,
    SPELL_GROW          = 21307,

    NPC_LOKHOLAR        = 13256,
    NPC_IVUS            = 13419
};

bool ProcessEventId_event_spell_BG_AV_BOSS(uint32 uiEventId, Object* pSource, Object* pTarget, bool bIsStart)
{
    if (pSource->GetTypeId() == TYPEID_PLAYER)
    {
        instance_BG_AV* m_pInstance = (instance_BG_AV*)((Player*)pSource)->GetInstanceData();
        if (m_pInstance)
        {
            if (uiEventId == EVENT_LOKHOLAR && m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_H == IN_PROGRESS))
            {
                ((Player*)pSource)->CastSpell(-325.0f, -168.0f, 9.0f, CALL_LOKHOLAR, true);
                m_pInstance->SetData(EVENT_MASTERS_START_SUMMONING_H, DONE);
            } 
            else if (uiEventId == EVENT_IVUS && m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_A == IN_PROGRESS))
            {
                if (Creature* myIvus = ((Player*)pSource)->SummonCreature(NPC_IVUS, -259.0f, -342.0f, 6.6f, 2.9, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000))
                    myIvus->CastSpell(myIvus, VISUAL_IVUS, true);
                m_pInstance->SetData(EVENT_MASTERS_START_SUMMONING_A, DONE);
            }
        }
    }
    return true;
}

static const uint32 bossSpell1[2] = {19133, 21670}; // Frostschock, Feenfeuer
static const uint32 bossSpell2[2] = {21367, 21669}; // Blizzard, Mondfeuer
static const uint32 bossSpell3[2] = {14907, 21668}; // Frostnova, Sternenfeuer
static const uint32 bossSpell4[2] = {16869, 20654}; // Eisgrabmal, Wucherwurzeln
static const uint32 bossSpell5[2] = {21369, 21667}; // Frostblitz, Zorn
static const uint32 bossWaypointWait[2] = {1, 0};
static const uint32 bossWaypointEnd[2] = {31, 32};

struct MANGOS_DLL_DECL mob_AV_Boss : public npc_escortAI
{
    mob_AV_Boss(Creature* creature) : npc_escortAI(creature) 
    { 
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
        m_creature->SetMaxPower(POWER_MANA, 112932);
        m_creature->SetPower(POWER_MANA, m_creature->GetMaxPower(POWER_MANA));

        m_pInstance = (instance_BG_AV*)creature->GetInstanceData();
        Reset(); 
    }

    instance_BG_AV* m_pInstance;

    int id;

    uint32 bossSpell1Timer;
    uint32 bossSpell2Timer;
    uint32 bossSpell3Timer;
    uint32 bossSpell4Timer;
    uint32 bossSpell5Timer;

    void Reset() 
    {
        bossSpell1Timer = urand(5000, 15000);
        bossSpell2Timer = urand(5000, 15000);
        bossSpell3Timer = urand(5000, 15000);
        bossSpell4Timer = urand(5000, 15000);
        bossSpell5Timer = urand(5000, 15000);

        if (m_creature->GetEntry() == NPC_LOKHOLAR)
            id = 0;
        else
            id = 1;
    };

    void defendPosition()
    {
        SetEscortPaused(true);
        m_creature->GetMotionMaster()->MoveRandom();
        CreatureCreatePos pos(m_creature->GetMap(), m_creature->GetPositionX(), m_creature->GetPositionY(),m_creature->GetPositionZ(), m_creature->GetOrientation());
        m_creature->SetSummonPoint(pos);       
    }

    void WaypointReached(uint32 uiPointId)
	{
        if (bossWaypointWait[id] == uiPointId)
            defendPosition();

        if (bossWaypointEnd[id] == uiPointId)
        {
            uint32 Type;
            id == 0 ? Type = EVENT_MASTERS_START_SUMMONING_H : Type = EVENT_MASTERS_START_SUMMONING_A;
            m_pInstance->SetData(Type, ARRIVED_BASE);

            //not finished now
            defendPosition();
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        //we have to stop that this aura will be removed by going evade mode
        if (!id)
        {
            if (pVictim->GetTypeId() == TYPEID_PLAYER)
                m_creature->MonsterYell("I drink in your suffering, puny mortal. Let your essence congeal with Lokholar!", LANG_UNIVERSAL);

            m_creature->CastSpell(m_creature, SPELL_GROW, true);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {   
        if (m_creature->isInCombat())
        {
            
            if (bossSpell1Timer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), bossSpell1[id]) == CAST_OK)
                    bossSpell1Timer = urand(5000, 15000);
            }
            else
                bossSpell1Timer -= uiDiff;

            if (bossSpell2Timer <= uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, bossSpell2[id]) == CAST_OK)
                    {
                        if (id)
                            bossSpell2Timer = urand(3000, 5000);
                        else
                            bossSpell2Timer = urand(10000, 20000);
                    }
                }
            }
            else
                bossSpell2Timer -= uiDiff;

            //FROSTNOVA OF THE ICELORD WILL CRASH THE SERVER (therefore id condition)!!!
            if (bossSpell3Timer <= uiDiff && id)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), bossSpell3[id]) == CAST_OK)
                    bossSpell3Timer = urand(5000, 10000);
            }
            else
                bossSpell3Timer -= uiDiff;

            if (bossSpell4Timer <= uiDiff)
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(pTarget, bossSpell4[id]) == CAST_OK)
                        bossSpell4Timer = urand(5000, 15000);
                }
            }
            else
                bossSpell4Timer -= uiDiff;

            if (bossSpell5Timer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), bossSpell5[id]) == CAST_OK)
                    bossSpell5Timer = urand(5000, 8000);
            }
            else
                bossSpell5Timer -= uiDiff;

            DoMeleeAttackIfReady();
        }

        uint32 Data;
        id == 0 ? Data = m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_H) : Data = m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_A);
        if (Data == ARRIVED_BASE)
            return;

        if (HasEscortState(STATE_ESCORT_PAUSED))
        {
            uint32 Data;
            id == 0 ? Data = m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_H) : Data = m_pInstance->GetData(EVENT_MASTERS_START_SUMMONING_A);
            if (Data == SPECIAL)
                SetEscortPaused(false);

            return;
        }

        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_creature->RemoveSplineFlag(SPLINEFLAG_WALKMODE);
            m_creature->SetSpeedRate(MOVE_RUN, 1.3f);
            npc_escortAI::UpdateAI(uiDiff);        
        }

        //PHASE 1: Start escorting
        if (HasEscortState(STATE_ESCORT_ESCORTING) || m_creature->isInCombat())
            return;

        Start(true);
    }
};

static CreatureAI* GetAI_mob_AV_Boss(Creature* creature)
{
    return new mob_AV_Boss(creature);
}

/*
DATABASE:
update creature_template set ScriptName = 'mob_av_bowman' where entry = 13358 or entry = 13359;
*/
enum myBowman
{
	SPELL_SHOOT = 22121,
	SHOOT_RANGE	= 100
};

struct MANGOS_DLL_DECL mob_av_bowman : public ScriptedAI
{
    mob_av_bowman(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

	uint32 shootTimer;
	Player* pTarget;

	void Reset()
	{
		shootTimer = 0;
		pTarget = 0;

		SetCombatMovement(false);
	}

	void getShootTarget()
	{
		pTarget = GetPlayerAtMinimumRange(SHOOT_RANGE);
        if (pTarget && m_creature->IsFriendlyTo(pTarget))
            pTarget = 0;
	}
	
	void UpdateAI(const uint32 uiDiff)
	{
		if (!pTarget)
		{
			getShootTarget();
			return;
		}
		else
		{
			if (pTarget->isAlive() && pTarget->IsWithinDist(m_creature, SHOOT_RANGE))
			{
				if (m_creature->IsWithinLOSInMap(pTarget))
				{
					if (shootTimer <= uiDiff)
					{
						m_creature->CastSpell(pTarget, SPELL_SHOOT, true);
						shootTimer = 3000;
					}
					else 
						shootTimer -= uiDiff;
				}
				else
					getShootTarget();
			}
			else
				getShootTarget();
		}
	}

};

CreatureAI* GetAI_mob_av_bowman(Creature* pCreature)
{
    return new mob_av_bowman(pCreature);
}

/*
DATABASE:
update creature_template set ScriptName = 'mob_av_mounted' where entry = 13152 or entry = 13137 or entry = 13143 or entry = 13144 or entry = 13147
or entry = 13300 or entry = 13296 or entry = 13138 or entry = 13298 or entry = 13297 or entry = 13318 or entry = 13299;
*/

struct MANGOS_DLL_DECL mob_av_mounted : public ScriptedAI
{
    mob_av_mounted(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

	void Reset()
	{
		Mount(m_creature->GetEntry());
	}

	void Aggro(Unit* /*pWho*/)
	{
		m_creature->Unmount();
        m_creature->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
	}

	void Mount(uint32 entry)
	{
		uint32 MountDisplayId = 0;
		uint32 MountSpell = 0;
		switch(entry)
		{
			//Wolf
			case 13152:
			case 13137:
				MountDisplayId = 14334;
				MountSpell = 22724;
				break;
			//Kodo
			case 13143:
				MountDisplayId = 14348;
				MountSpell = 22718;
				break;
			//Raptor
			case 13144:
				MountDisplayId = 14388;
				MountSpell = 22721;
				break;
			//Skeletal Horse
			case 13147:
				MountDisplayId = 10671;
				MountSpell = 17643;
				break;
			//Horse
			case 13300:
			case 13296:
			case 13138:
				MountDisplayId = 14337;
				MountSpell = 22717;
				break;
			//Strider
			case 13298:
				MountDisplayId = 6569;
				MountSpell = 10969;
				break;
			//Ram
			case 13297:
			case 13318:
				MountDisplayId = 14577;
				MountSpell = 22720;
				break;
			//Tiger
			case 13299:
				MountDisplayId = 9991;
				MountSpell = 22723;
				break;
		}
		m_creature->Mount(MountDisplayId, MountSpell);
	}

	void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_av_mounted(Creature* pCreature)
{
    return new mob_av_mounted(pCreature);
}

/*
DATABASE:
update creature_template set ScriptName = 'mob_av_windrider' where entry = 13181 or entry = 13180 or entry = 13179 or entry = 13438  or entry = 13439 or entry = 13437;
*/
//later we have script more content here but no time today(only visual handling atm)
struct MANGOS_DLL_DECL mob_av_windrider : public ScriptedAI
{
    mob_av_windrider(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

	void Reset()
	{
        m_creature->setFaction(35);
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
	}
};

CreatureAI* GetAI_mob_av_windrider(Creature* pCreature)
{
    return new mob_av_windrider(pCreature);
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

    pNewScript = new Script;
    pNewScript->Name = "event_spell_BG_AV_BOSS";
    pNewScript->pProcessEventId = &ProcessEventId_event_spell_BG_AV_BOSS;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
	pNewScript->Name = "mob_AV_Boss";
	pNewScript->GetAI = &GetAI_mob_AV_Boss;
	pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_av_bowman";
    pNewScript->GetAI = &GetAI_mob_av_bowman;
    pNewScript->RegisterSelf();

	pNewScript = new Script;
    pNewScript->Name = "mob_av_mounted";
    pNewScript->GetAI = &GetAI_mob_av_mounted;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_av_windrider";
    pNewScript->GetAI = &GetAI_mob_av_windrider;
    pNewScript->RegisterSelf();
}
