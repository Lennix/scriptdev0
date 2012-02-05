/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Instance_Onyxias_Lair
SD%Complete: 80
SDComment: 
SDCategory: Onyxia's Lair
EndScriptData */

#include "precompiled.h"
#include "onyxias_lair.h"

/* Onyxia's Lair encounters:
0 - Onyxia */

instance_onyxias_lair::instance_onyxias_lair(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
}

void instance_onyxias_lair::Initialize()
{   
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
    m_uiOnyxiaWarderGUID.clear();
}

bool instance_onyxias_lair::IsEncounterInProgress() const
{
    for(uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;
    return false;
}

void instance_onyxias_lair::OnCreatureCreate(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_ONYXIA:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_ONYXIA_WARDER:
            m_uiOnyxiaWarderGUID.push_back(pCreature->GetObjectGuid());
            break;
    }
}

void instance_onyxias_lair::SetData(uint32 uiType, uint32 uiData)
{
    switch(uiType)
    {
        case TYPE_ONYXIA:
            m_auiEncounter[0] = uiData;
            if (uiData == DONE)
            {
                // Despawn Onyxia Warders
                if (!m_uiOnyxiaWarderGUID.empty())
                    for (GUIDList::iterator itr = m_uiOnyxiaWarderGUID.begin(); itr != m_uiOnyxiaWarderGUID.end(); ++itr)
                        if (Creature* pWarder = instance->GetCreature(*itr))
                            pWarder->ForcedDespawn();
            }
            else if (uiData == IN_PROGRESS)
            {
                // Respawn Onyxia Warders once Onyxia is aggroed
                if (!m_uiOnyxiaWarderGUID.empty())
                    for (GUIDList::iterator itr = m_uiOnyxiaWarderGUID.begin(); itr != m_uiOnyxiaWarderGUID.end(); ++itr)
                        if (Creature* pWarder = instance->GetCreature(*itr))
                            pWarder->Respawn();
            }
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0];

        strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_onyxias_lair::GetData(uint32 uiType)
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_onyxias_lair::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0];

    for(uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

InstanceData* GetInstanceData_instance_onyxias_lair(Map* pMap)
{
    return new instance_onyxias_lair(pMap);
}

void AddSC_instance_onyxias_lair()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_onyxias_lair";
    pNewScript->GetInstanceData = &GetInstanceData_instance_onyxias_lair;
    pNewScript->RegisterSelf();
}
