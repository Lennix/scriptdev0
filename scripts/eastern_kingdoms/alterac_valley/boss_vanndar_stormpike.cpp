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
-- AV ALLIANCE_ENDBOSS_BASECAMP --
delete from creature where (guid = 52044 or guid = 52035) and id = 12127;
delete from creature where (guid = 53606 or guid = 53607) and id = 13331;
delete from creature where (guid = 53622 or guid = 53621) and id = 13422;
delete from creature where guid = 150121  and id = 13816;
update creature_template set ScriptName = 'mob_av_marshal_or_warmaster' where entry = 14762 or entry = 14763 or entry = 14764 or entry = 14765 or entry = 14766 or entry = 14767 or entry = 14768 or entry = 14769;
delete from creature where id = 11948 or id = 14762 or id = 14763 or id = 14764 or id = 14765 or id = 14766 or id = 14767 or id = 14768 or id = 14769;
INSERT INTO `creature` VALUES ('150142', '11948', '30', '0', '0', '722.43', '-10.9982', '50.7046', '3.42085', '120', '0', '0', '160587', '0', '0', '0');
INSERT INTO `creature` VALUES ('54289', '14762', '30', '0', '0', 723.058, 	-14.1548, 	50.7046, 	3.40339, '490', '0', '0', '61800', '0', '0', '0');
INSERT INTO `creature` VALUES ('54290', '14763', '30', '0', '0', 721.104, 	-7.64155, 	50.7046, 	3.45575, '490', '0', '0', '26500', '0', '0', '0');
INSERT INTO `creature` VALUES ('54291', '14764', '30', '0', '0', 715.691, 	-4.72233, 	50.2187, 	3.47321, '490', '0', '0', '59071', '0', '0', '0');
INSERT INTO `creature` VALUES ('54292', '14765', '30', '0', '0', 720.046, 	-19.9413, 	50.2187, 	3.36849, '490', '5', '0', '56083', '0', '0', '0');
INSERT INTO `creature` VALUES ('54294', '14766', '30', '0', '0', '700.621', '-10.9393', '50.1354', '5.84022', '490', '0', '0', '45124', '0', '0', '0');
INSERT INTO `creature` VALUES ('54293', '14767', '30', '0', '0', '704.332', '-22.1019', '50.1354', '1.01396', '25', '5', '0', '59087', '0', '0', '0');
INSERT INTO `creature` VALUES ('54295', '14768', '30', '0', '0', '701.899', '-18.748', '50.1354', '0.259967', '490', '0', '0', '61423', '0', '0', '0');
INSERT INTO `creature` VALUES ('54296', '14769', '30', '0', '0', '700.664', '-14.8941', '50.1354', '0.299237', '490', '0', '0', '14633', '0', '0', '0');
delete from creature_battleground where event1 = 71 or event1 = 72 or event1 = 73 or event1 = 74;
INSERT INTO `creature_battleground` VALUES ('54294', '71', '0');
INSERT INTO `creature_battleground` VALUES ('54293', '72', '0');
INSERT INTO `creature_battleground` VALUES ('54295', '73', '0');
INSERT INTO `creature_battleground` VALUES ('54296', '74', '0');
delete from battleground_events where event1 = 71 or event1 = 72 or event1 = 73 or event1 = 74;
INSERT INTO `battleground_events` VALUES ('30', '71', '0', 'warmaster spawn');
INSERT INTO `battleground_events` VALUES ('30', '72', '0', 'warmaster spawn');
INSERT INTO `battleground_events` VALUES ('30', '73', '0', 'warmaster spawn');
INSERT INTO `battleground_events` VALUES ('30', '74', '0', 'warmaster spawn');
*/

#include "precompiled.h"

enum Spells
{
    SPELL_AVATAR        = 19135,
    SPELL_STORMBOLT     = 20685,
    SPELL_THUNDERCLAP   = 15588,
};

enum Yells
{
    YELL_AGGRO          = -1030008,
    YELL_EVADE          = -1030009,
    YELL_RESPAWN1       = -1030010,
    YELL_RESPAWN2       = -1030011,
    YELL_RANDOM1        = -1030012,
    YELL_RANDOM2        = -1030013,
    YELL_RANDOM3        = -1030014,
    YELL_RANDOM4        = -1030015,
    YELL_RANDOM5        = -1030016,
    YELL_RANDOM6        = -1030017,
    YELL_RANDOM7        = -1030018,
};

struct MANGOS_DLL_DECL boss_vanndar_stormpikeAI : public ScriptedAI
{
    boss_vanndar_stormpikeAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiAvatarTimer;
    uint32 m_uiStormboltTimer;
    uint32 m_uiThunderclapTimer;
    uint32 m_uiYellTimer;
    uint32 m_uiEvadeTimer;

    void Reset()
    {
        m_uiAvatarTimer = 3*IN_MILLISECONDS;
        m_uiStormboltTimer = 6*IN_MILLISECONDS;
        m_uiThunderclapTimer = 4*IN_MILLISECONDS;
        m_uiYellTimer = urand(20*IN_MILLISECONDS,30*IN_MILLISECONDS);
        m_uiEvadeTimer = 5*IN_MILLISECONDS;
    }

    void Aggro(Unit* /*pWho*/)
    {
        DoScriptText(YELL_AGGRO, m_creature);
        m_creature->CallForHelp(50.0f);
    }

    void JustReachedHome()
    {
        switch(urand(0,1))
        {
            case 0:
                DoScriptText(YELL_RESPAWN1, m_creature);
                break;
            case 1:
                DoScriptText(YELL_RESPAWN2, m_creature);
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Avatar
        if (m_uiAvatarTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_AVATAR);
            m_uiAvatarTimer = urand(15*IN_MILLISECONDS,20*IN_MILLISECONDS);
        }
        else
            m_uiAvatarTimer -= uiDiff;

        // Thundeclap
        if (m_uiThunderclapTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_THUNDERCLAP);
            m_uiThunderclapTimer = urand(5*IN_MILLISECONDS,15*IN_MILLISECONDS);
        }
        else
            m_uiThunderclapTimer -= uiDiff;

        // Stormbolt
        if (m_uiStormboltTimer <= uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_STORMBOLT);
            m_uiStormboltTimer = urand(10*IN_MILLISECONDS,25*IN_MILLISECONDS);
        }
        else
            m_uiStormboltTimer -= uiDiff;

        // Yell
        if (m_uiYellTimer <= uiDiff)
        {
            switch(urand(0,6))
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
                case 5:
                    DoScriptText(YELL_RANDOM6, m_creature);
                    break;
                case 6:
                    DoScriptText(YELL_RANDOM7, m_creature);
                    break;
            }
            m_uiYellTimer = urand(20*IN_MILLISECONDS,30*IN_MILLISECONDS);
        }
        else
            m_uiYellTimer -= uiDiff;

        // Check if creature is not outside of building
        if (m_uiEvadeTimer <= uiDiff)
        {
            float fX, fY, fZ, fO;
            m_creature->GetSummonPoint(fX, fY, fZ, fO);
            if (m_creature->GetDistance2d(fX, fY) > 50.0f)
            {
                EnterEvadeMode();
                DoScriptText(YELL_EVADE, m_creature);
            }
            m_uiEvadeTimer = 5*IN_MILLISECONDS;
        }
        else
            m_uiEvadeTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_vanndar_stormpike(Creature* pCreature)
{
    return new boss_vanndar_stormpikeAI(pCreature);
}

void AddSC_boss_vanndar_stormpike()
{
    Script* pNewscript;
    pNewscript = new Script;
    pNewscript->Name = "boss_vanndar_stormpike";
    pNewscript->GetAI = &GetAI_boss_vanndar_stormpike;
    pNewscript->RegisterSelf();
}
