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
SDName: Instance_Deadmines
SD%Complete: 0
SDComment: Placeholder
SDCategory: Deadmines
EndScriptData */

#include "precompiled.h"
#include "deadmines.h"

instance_deadmines::instance_deadmines(Map* pMap) : ScriptedInstance(pMap),
	m_uiIronDoor_Timer(0),
	m_uiDoor_Step(0)
{
	Initialize();
}

void instance_deadmines::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_deadmines::OnCreatureCreate(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_MR_SMITE)
        m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
}

void instance_deadmines::OnObjectCreate(GameObject* pGo)
{
    switch(pGo->GetEntry())
	{
		case GO_IRON_CLAD:
		case GO_DEFIAS_CANNON:
			m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
			break;
	}
}

void instance_deadmines::SetData(uint32 uiType, uint32 uiData)
{
    if (uiType == TYPE_DEFIAS_ENDDOOR)
    {
        if (uiData == IN_PROGRESS)
        {
			if (GameObject* pGo = GetSingleGameObjectFromStorage(GO_IRON_CLAD))
            {
                pGo->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                m_uiIronDoor_Timer = 3000;
            }
        }
        m_auiEncounter[0] = uiData;
    }
}

uint32 instance_deadmines::GetData(uint32 uiType)
{
	if (uiType < MAX_ENCOUNTER)
		return m_auiEncounter[uiType];

	return 0;
}

void instance_deadmines::Update(uint32 uiDiff)
{
    if (m_uiIronDoor_Timer)
    {
        if (m_uiIronDoor_Timer <= uiDiff)
        {
            if (Creature* pMrSmite = GetSingleCreatureFromStorage(NPC_MR_SMITE))
            {
                switch(m_uiDoor_Step)
                {
                    case 0:
                        DoScriptText(INST_SAY_ALARM1,pMrSmite);
                        m_uiIronDoor_Timer = 2000;
                        ++m_uiDoor_Step;
                        break;
                    case 1:
                        if (Creature* pi1 = pMrSmite->SummonCreature(NPC_PIRATE, 93.68f,-678.63f,7.71f,2.09f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1800000))
                            pi1->GetMotionMaster()->MovePoint(0,100.11f,-670.65f,7.42f);
                        if (Creature* pi2 = pMrSmite->SummonCreature(NPC_PIRATE,102.63f,-685.07f,7.42f,1.28f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1800000))
                            pi2->GetMotionMaster()->MovePoint(0,100.11f,-670.65f,7.42f);
                        ++m_uiDoor_Step;
                        m_uiIronDoor_Timer = 10000;
                        break;
                    case 2:
                        DoScriptText(INST_SAY_ALARM2,pMrSmite);
                        m_uiDoor_Step = 0;
                        m_uiIronDoor_Timer = 0;
                        debug_log("SD2: Instance Deadmines: Iron door event reached end.");
                        break;
                }
            }
            else
                m_uiIronDoor_Timer = 0;
        }
        else
            m_uiIronDoor_Timer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_deadmines(Map* pMap)
{
    return new instance_deadmines(pMap);
}

void AddSC_instance_deadmines()
{
    Script* pNewscript;
    pNewscript = new Script;
    pNewscript->Name = "instance_deadmines";
    pNewscript->GetInstanceData = &GetInstanceData_instance_deadmines;
    pNewscript->RegisterSelf();
}
