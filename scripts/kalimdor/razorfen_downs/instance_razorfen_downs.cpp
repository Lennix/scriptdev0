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
SDName: Instance Razorfen Downs
SD%Complete:
SDComment:
SDCategory: Razorfen Downs
EndScriptData */

#include "precompiled.h"
#include "razorfen_downs.h"

static float Spawns[8][4] =
{
    {2513.32f, 854.348f, 47.6782f, 0.0543563f},
    {2514.23f, 861.676f, 47.6782f, 5.97233f},
    {2517.42f, 867.145f, 47.6782f, 5.78384f},
    {2522.61f, 873.89f, 47.6782f, 5.43825f},
    {2507.02f, 858.804f, 47.6782f, 6.12941f},
    {2509.44f, 866.787f, 47.5382f, 5.91342f},
    {2513.62f, 874.704f, 47.272f, 5.71315f},
    {2505.87f, 851.137f, 47.678f, 0.019007f}
};

instance_razorfen_downs::instance_razorfen_downs(Map* pMap) : ScriptedInstance(pMap),
	m_uiGongUse(0)
{
    Initialize();
}

void instance_razorfen_downs::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    m_uiSpectreGUID.clear();
}
    
void instance_razorfen_downs::OnCreatureCreate(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_FROZEN_SPECTRE:
			m_uiSpectreGUID.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_RAGGLESNOUT:
            SetRareBoss(pCreature, 50);
            break;
    }
}
    
void instance_razorfen_downs::OnObjectCreate(GameObject* pGo)
{
    switch(pGo->GetEntry())
    {
        case GO_GONG:
            m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
            if(m_auiEncounter[0] == DONE)
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND + GO_FLAG_NO_INTERACT);
            break;
    }
}

void instance_razorfen_downs::SetData(uint32 uiType, uint32 uiData)
{
    switch(uiType)
    {
        case DATA_GONG_USE:
            SummonWave(uiData);
            m_uiGongUse = uiData;
            break;
        case TYPE_TUTENKASH:
            if (uiData == DONE)
				if (GameObject* Gong = GetSingleGameObjectFromStorage(GO_GONG))
                    Gong->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND + GO_FLAG_NO_INTERACT);
            m_auiEncounter[0] = uiData;
            break;
        case TYPE_AMNENNAR:
            m_auiEncounter[1] = uiData;
            if (uiData == NOT_STARTED || uiData == DONE)
                DespawnSpectres();
            break;                
    }
        
    if (uiData = DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1];

        strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}
    
uint32 instance_razorfen_downs::GetData(uint32 uiType)
{
    switch(uiType)
    {
        case DATA_GONG_USE:
            return m_uiGongUse;
        case TYPE_TUTENKASH:
            return m_auiEncounter[0];
        case TYPE_AMNENNAR:
            return m_auiEncounter[1];
    }
    return 0;
}
    
void instance_razorfen_downs::DespawnSpectres()
{
    for(GUIDList::iterator itr = m_uiSpectreGUID.begin(); itr != m_uiSpectreGUID.end(); ++itr)
    {
        if (Creature* Spectre = instance->GetCreature(*itr))
            Spectre->RemoveFromWorld();
    }
}
    
void instance_razorfen_downs::SummonWave(uint32 uiWave)
{
	if (GameObject* Gong = GetSingleGameObjectFromStorage(GO_GONG))
    {
        switch(uiWave)
        {
            case 1:
                SetData(TYPE_TUTENKASH, IN_PROGRESS);
                for(uint32 i = 0; i < MAX_TOMB_FIEND; ++i)
                    Gong->SummonCreature(NPC_TOMB_FIEND, Spawns[i][0], Spawns[i][1], Spawns[i][2], Spawns[i][3], TEMPSUMMON_DEAD_DESPAWN, 5000);
                break;
            case 2:
                SetData(TYPE_TUTENKASH, IN_PROGRESS);
                for(uint32 i = 0; i < MAX_TOMB_REAVER; ++i)
                    Gong->SummonCreature(NPC_TOMB_REAVER, Spawns[i][0], Spawns[i][1], Spawns[i][2], Spawns[i][3], TEMPSUMMON_DEAD_DESPAWN, 5000);
                break;
            case 3:
                SetData(TYPE_TUTENKASH, DONE);
                Gong->SummonCreature(NPC_TUTENKASH, Spawns[0][0], Spawns[0][1], Spawns[0][2], Spawns[0][3], TEMPSUMMON_DEAD_DESPAWN, 30000);
                break;
        }
    }
}

void instance_razorfen_downs::Load(const char* in)
{
    if (!in)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(in);

    std::istringstream loadStream(in);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1];
        
    for(uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

InstanceData* GetInstanceData_instance_razorfen_downs(Map* pMap)
{
    return new instance_razorfen_downs(pMap);
}

void AddSC_instance_razorfen_downs()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "instance_razorfen_downs";
    pNewscript->GetInstanceData = &GetInstanceData_instance_razorfen_downs;
    pNewscript->RegisterSelf();
}
