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

#include "precompiled.h"
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
            float fX, fY, fZ, fO;
            m_creature->GetSummonPoint(fX, fY, fZ, fO);
            if (m_creature->GetDistance2d(fX, fY) > 50.0f)
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
enum
{
    FACTION_FROSTWOLF       = 729,
    FACTION_STORMPIKE       = 730,

    BG_AV_WARCRY_BUFF_1     = 28418,
    BG_AV_WARCRY_BUFF_2     = 28419,
    BG_AV_WARCRY_BUFF_3     = 28420
};

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
}
