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
#include "alterac_valley.h"

/*
DATABASE:
delete from instance_template where map = 30;
INSERT INTO `instance_template` VALUES ('30', '0', '60', '60', '80', '0', '0', '0', '0', 'instance_BG_AV');
*/

instance_BG_AV::instance_BG_AV(Map* pMap) : ScriptedInstance(pMap)
{
    memset(&m_uiEvent, 0, sizeof(m_uiEvent));
}

void instance_BG_AV::Initialize()
{
    for(uint8 i = 0; i < MAX_EVENTS; ++i)
        m_uiEvent[i] = NOT_STARTED;

    moveBossTimer[0] = 0;
    moveBossTimer[1] = 0;
}

void instance_BG_AV::SetData(uint32 uiType, uint32 uiData)
{
    if (uiType < MAX_EVENTS && uiType >= 0)
    {
        switch(uiType)
        {
            case EVENT_MASTERS_START_SUMMONING_H:   
                if(uiData == DONE)
                    moveBossTimer[0] = BOSS_START_TIME;
                break;
            case EVENT_MASTERS_START_SUMMONING_A:
                if(uiData == DONE)
                    moveBossTimer[1] = BOSS_START_TIME;
                break;
        }

        m_uiEvent[uiType] = uiData;
    }
}

uint32 instance_BG_AV::GetData(uint32 uiType)
{
    if (uiType < MAX_EVENTS && uiType >= 0)
        return m_uiEvent[uiType];
}

void instance_BG_AV::Update(uint32 uiDiff)
{
    for (uint8 i = 0; i < 2; i++)
    {
        if (moveBossTimer[i])
        {
            if (moveBossTimer[i] <= uiDiff)
            {
                i == 0 ? SetData(EVENT_MASTERS_START_SUMMONING_H, SPECIAL) : SetData(EVENT_MASTERS_START_SUMMONING_A, SPECIAL);
                moveBossTimer[i] = 0;
            }
            else
                moveBossTimer[i] -= uiDiff;
        }
    }
}

InstanceData* GetInstanceData_instance_BG_AV(Map* pMap)
{
    return new instance_BG_AV(pMap);
}

void AddSC_instance_alterac_valley()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "instance_BG_AV";
    pNewScript->GetInstanceData = &GetInstanceData_instance_BG_AV;
    pNewScript->RegisterSelf();
}