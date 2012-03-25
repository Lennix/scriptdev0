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
-- AV HORDE_ENDBOSS_BASECAMP --
update creature_template set ScriptName = 'mob_av_marshal_or_warmaster', flags_extra = 4096 where entry = 14770 or entry = 14771 or entry = 14772 or entry = 14773 or entry = 14774 or entry = 14775 or entry = 14776 or entry = 14777;
update creature_template set flags_extra = 4096 where entry = 11946 or entry = 12121 or entry = 12122;
delete from creature where id = 11946 or id = 12121 or id = 12122 or id = 14770 or id = 14771 or id = 14772 or id = 14773 or id = 14774 or id = 14775 or id = 14776 or id = 14777;
INSERT INTO `creature` VALUES ('150143', '11946', '30', '0', '0', '-1370.9', '-219.793', '98.4258', '5.04381', '120', '0', '0', '158400', '0', '0', '0');
INSERT INTO `creature` VALUES ('51987', '12121', '30', '0', '0', '-1369.71', '-214.568', '99.3712', '5.65565', '1785', '0', '0', '9132', '0', '0', '0');
INSERT INTO `creature` VALUES ('51988', '12122', '30', '0', '0', '-1376.2', '-217.254', '99.3713', '4.59538', '1785', '0', '0', '9183', '0', '0', '0');
INSERT INTO `creature` VALUES ('54297', '14774', '30', '0', '0', '-1360.65', '-233.507', '98.3973', '2.59731', '25', '5', '0', '59557', '0', '0', '0');
INSERT INTO `creature` VALUES ('54298', '14775', '30', '0', '0', '-1369.87', '-236.527', '98.4254', '1.23935', '25', '5', '0', '60663', '0', '0', '0');
INSERT INTO `creature` VALUES ('150152', '14773', '30', '0', '0', '-1376.4', '-225.599', '98.4262', '5.8135', '120', '0', '0', '60489', '0', '0', '0');
INSERT INTO `creature` VALUES ('150150', '14772', '30', '0', '0', '-1367.53', '-218.8', '98.4262', '4.63384', '120', '0', '0', '60900', '0', '0', '0');
INSERT INTO `creature` VALUES ('54299', '14770', '30', '0', '0', '-1363.49', '-235.971', '98.3966', '1.98154', '490', '0', '0', '60575', '0', '0', '0');
INSERT INTO `creature` VALUES ('54300', '14771', '30', '0', '1416', '-1365.95', '-236.724', '98.4081', '1.84488', '25', '5', '0', '60089', '0', '0', 0);
INSERT INTO `creature` VALUES ('150151', '14777', '30', '0', '0', '-1373.73', '-221.015', '98.4268', '5.18756', '120', '0', '0', '58547', '0', '0', '0');
INSERT INTO `creature` VALUES ('150153', '14776', '30', '0', '0', '-1362.79', '-219.883', '98.3969', '3.95447', '120', '0', '0', '60076', '0', '0', '0');
delete from creature_battleground where event1 = 68 or event1 = 67 or event1 = 69 or event1 = 70;
INSERT INTO `creature_battleground` VALUES ('54297', '68', '0');
INSERT INTO `creature_battleground` VALUES ('54298', '67', '0');
INSERT INTO `creature_battleground` VALUES ('54299', '69', '0');
INSERT INTO `creature_battleground` VALUES ('54300', '70', '0');
delete from battleground_events where event1 = 68 or event1 = 67 or event1 = 69 or event1 = 70;
INSERT INTO `battleground_events` VALUES ('30', '67', '0', 'warmaster spawn');
INSERT INTO `battleground_events` VALUES ('30', '68', '0', 'warmaster spawn');
INSERT INTO `battleground_events` VALUES ('30', '69', '0', 'warmaster spawn');
INSERT INTO `battleground_events` VALUES ('30', '70', '0', 'warmaster spawn');
*/

#include "precompiled.h"
#include "alterac_valley.h"

enum Dreakthar_Spells
{
    SPELL_FRENZY            = 8269,
    SPELL_KNOCKDOWN         = 19128,
    SPELL_WHIRLWIND       = 15589,
    SPELL_WHIRLWIND2      = 13736,
    SPELL_SWEEPING_STRIKES  = 18765, // not sure
    SPELL_CLEAVE          = 20677, // not sure
    SPELL_WINDFURY          = 35886, // not sure
    SPELL_STORMPIKE         = 51876, // not sure
};

static const float centerX = -1372;
static const float centerY = -216;

enum Yells
{
    YELL_AGGRO              = -1030000,
    YELL_EVADE              = -1030001,
    YELL_RESPAWN            = -1030002,
    YELL_RANDOM1            = -1030003,
    YELL_RANDOM2            = -1030004,
    YELL_RANDOM3            = -1030005,
    YELL_RANDOM4            = -1030006,
    YELL_RANDOM5            = -1030007,
};

struct MANGOS_DLL_DECL boss_drektharAI : public ScriptedAI
{
    boss_drektharAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
        m_pInstance = (instance_BG_AV*)pCreature->GetInstanceData();
        Reset();
    }  

    instance_BG_AV* m_pInstance;

    uint32 m_uiFrenzyTimer;
    uint32 m_uiKnockdownTimer;
    uint32 m_uiWhirlwindTimer;
    uint32 m_uiWhirlwind2Timer;
    uint32 m_uiYellTimer;
    uint32 m_uiEvadeTimer;
    Creature* wolf[2];

    void Reset()
    {
        m_uiWhirlwindTimer = urand(1*IN_MILLISECONDS,20*IN_MILLISECONDS);
        m_uiWhirlwind2Timer = urand(1*IN_MILLISECONDS,20*IN_MILLISECONDS);
        m_uiKnockdownTimer = 12*IN_MILLISECONDS;
        m_uiFrenzyTimer = 6*IN_MILLISECONDS;
        m_uiYellTimer = urand(20*IN_MILLISECONDS,30*IN_MILLISECONDS);
        m_uiEvadeTimer = 5*IN_MILLISECONDS;
    }

    void Aggro(Unit* /*pWho*/)
    {
        m_pInstance->SetData(EVENT_ENDBOSS_STATUS_H, IN_PROGRESS);
        DoScriptText(YELL_AGGRO, m_creature);
        m_creature->CallForHelp(50.0f);
        for (uint8 i = 0; i < 2; i++)
            wolf[i] = GetClosestCreatureWithEntry(m_creature, (12121 + i), 50.0f);
    }

    void JustReachedHome()
    {
        DoScriptText(YELL_RESPAWN, m_creature);
        for (uint8 i = 0; i < 2; i++)
        {
            if (wolf[i] && wolf[i]->isDead())
                wolf[i]->Respawn();
            else if (wolf[i] && wolf[i]->isAlive())
                wolf[i]->AI()->EnterEvadeMode();
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Frenzy
        if (m_uiFrenzyTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_FRENZY);
            m_uiFrenzyTimer = urand(20*IN_MILLISECONDS,30*IN_MILLISECONDS);
        }
        else
            m_uiFrenzyTimer -= uiDiff;

        // Knockdown
        if (m_uiKnockdownTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_KNOCKDOWN);
            m_uiKnockdownTimer = urand(10*IN_MILLISECONDS,15*IN_MILLISECONDS);
        }
        else
            m_uiKnockdownTimer -= uiDiff;

        // Whirlwind 1
        if (m_uiWhirlwindTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_WHIRLWIND);
            m_uiWhirlwindTimer =  urand(8*IN_MILLISECONDS,18*IN_MILLISECONDS);
        }
        else
            m_uiWhirlwindTimer -= uiDiff;

        // Whirlwind 2
        if (m_uiWhirlwind2Timer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_WHIRLWIND2);
            m_uiWhirlwind2Timer = urand(7*IN_MILLISECONDS,25*IN_MILLISECONDS);
        }
        else
            m_uiWhirlwind2Timer -= uiDiff;

        // Yell
        if (m_uiYellTimer <= uiDiff)
        {
            switch(urand(0,5))
            {
                case 0:
                    DoScriptText(YELL_RANDOM1, m_creature);
                    break;
                case 1:
                    DoScriptText(YELL_RANDOM2, m_creature);
                    break;
                case 2:
                    DoScriptText(YELL_RANDOM3, m_creature);
                    break;
                case 3:
                    DoScriptText(YELL_RANDOM4, m_creature);
                    break;
                case 4:
                    DoScriptText(YELL_RANDOM5, m_creature);
                    break;
            }
            m_uiYellTimer = urand(20*IN_MILLISECONDS,30*IN_MILLISECONDS);
        }
        else
            m_uiYellTimer -= uiDiff;

        // Check if creature is not outside of building     
        if (m_uiEvadeTimer <= uiDiff)
        {
            if (m_creature->GetDistance2d(centerX, centerY) > 40.0f)
            {
                m_pInstance->SetData(EVENT_ENDBOSS_STATUS_H, FAIL);
                EnterEvadeMode();
                DoScriptText(YELL_EVADE, m_creature);
            }
            for (uint8 i = 0; i < 2; i++)
            {
                if (wolf[i] && wolf[i]->isAlive() && wolf[i]->GetDistance2d(centerX, centerY) > 35.0f)
                    wolf[i]->AI()->EnterEvadeMode();
            }
            m_uiEvadeTimer = 2*IN_MILLISECONDS;
        }
        else
            m_uiEvadeTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_drekthar(Creature* pCreature)
{
    return new boss_drektharAI(pCreature);
}

void AddSC_boss_drekthar()
{
    Script* pNewscript;
    pNewscript = new Script;
    pNewscript->Name = "boss_drekthar";
    pNewscript->GetAI = &GetAI_boss_drekthar;
    pNewscript->RegisterSelf();
}
