/* Copyright (C) 2006 - 2011 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "precompiled.h"

/**
   Function that uses a door or a button

   @param   guid The ObjectGuid of the Door/ Button that will be used
   @param   uiWithRestoreTime (in seconds) if == 0 autoCloseTime will be used (if not 0 by default in *_template)
   @param   bUseAlternativeState Use to alternative state
 */
void ScriptedInstance::DoUseDoorOrButton(ObjectGuid guid, uint32 uiWithRestoreTime, bool bUseAlternativeState)
{
    if (!guid)
        return;

    if (GameObject* pGo = instance->GetGameObject(guid))
    {
        if (pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR || pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
        {
            if (pGo->getLootState() == GO_READY)
                pGo->UseDoorOrButton(uiWithRestoreTime, bUseAlternativeState);
            else if (pGo->getLootState() == GO_ACTIVATED)
                pGo->ResetDoorOrButton();
        }
        else
            error_log("SD0: Script call DoUseDoorOrButton, but gameobject entry %u is type %u.", pGo->GetEntry(), pGo->GetGoType());
    }
}

/// Function that uses a door or button that is stored in m_mGoEntryGuidStore
void ScriptedInstance::DoUseDoorOrButton(uint32 uiEntry, uint32 uiWithRestoreTime /*= 0*/, bool bUseAlternativeState /*= false*/)
{
    EntryGuidMap::iterator find = m_mGoEntryGuidStore.find(uiEntry);
    if (find != m_mGoEntryGuidStore.end())
        DoUseDoorOrButton(find->second, uiWithRestoreTime, bUseAlternativeState);
    else
        // Output log, possible reason is not added GO to storage, or not yet loaded
        debug_log("SD0: Script call DoUseDoorOrButton(by Entry), but no gameobject of entry %u was created yet, or it was not stored by script for map %u.", uiEntry, instance->GetId());
}

/**
   Function that respawns a despawned GameObject with given time

   @param   guid The ObjectGuid of the GO that will be respawned
   @param   uiTimeToDespawn (in seconds) Despawn the GO after this time, default is a minute
 */
void ScriptedInstance::DoRespawnGameObject(ObjectGuid guid, uint32 uiTimeToDespawn)
{
    if (!guid)
        return;

    if (GameObject* pGo = instance->GetGameObject(guid))
    {
        //not expect any of these should ever be handled
        if (pGo->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE || pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR ||
            pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
            return;

        if (pGo->isSpawned())
            return;

        pGo->SetRespawnTime(uiTimeToDespawn);
        pGo->Refresh();
    }
}

/// Function that respawns a despawned GO that is stored in m_mGoEntryGuidStore
void ScriptedInstance::DoRespawnGameObject(uint32 uiEntry, uint32 uiTimeToDespawn)
{
    EntryGuidMap::iterator find = m_mGoEntryGuidStore.find(uiEntry);
    if (find != m_mGoEntryGuidStore.end())
        DoRespawnGameObject(find->second, uiTimeToDespawn);
    else
        // Output log, possible reason is not added GO to storage, or not yet loaded;
        debug_log("SD0: Script call DoRespawnGameObject(by Entry), but no gameobject of entry %u was created yet, or it was not stored by script for map %u.", uiEntry, instance->GetId());
}

/**
   Helper function to update a world state for all players in the map

   @param   uiStateId The WorldState that will be set for all players in the map
   @param   uiStateData The Value to which the State will be set to
 */
void ScriptedInstance::DoUpdateWorldState(uint32 uiStateId, uint32 uiStateData)
{
    Map::PlayerList const& lPlayers = instance->GetPlayers();

    if (!lPlayers.isEmpty())
    {
        for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
        {
            if (Player* pPlayer = itr->getSource())
                pPlayer->SendUpdateWorldState(uiStateId, uiStateData);
        }
    }
    else
        debug_log("SD0: DoUpdateWorldState attempt send data but no players in map.");
}

/// Get the first found Player* (with requested properties) in the map. Can return NULL.
Player* ScriptedInstance::GetPlayerInMap(bool bOnlyAlive /*=false*/, bool bCanBeGamemaster /*=true*/)
{
    Map::PlayerList const& lPlayers = instance->GetPlayers();

    for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
    {
        Player* pPlayer = itr->getSource();
        if (pPlayer && (!bOnlyAlive || pPlayer->isAlive()) && (bCanBeGamemaster || !pPlayer->isGameMaster()))
            return pPlayer;
    }

    return NULL;
}

/// Returns a pointer to a loaded GameObject that was stored in m_mGoEntryGuidStore. Can return NULL
GameObject* ScriptedInstance::GetSingleGameObjectFromStorage(uint32 uiEntry)
{
    EntryGuidMap::iterator find = m_mGoEntryGuidStore.find(uiEntry);
    if (find != m_mGoEntryGuidStore.end())
        return instance->GetGameObject(find->second);

    // Output log, possible reason is not added GO to map, or not yet loaded;
    debug_log("SD0: Script requested gameobject with entry %u, but no gameobject of this entry was created yet, or it was not stored by script for map %u.", uiEntry, instance->GetId());

    return NULL;
}

/// Returns a pointer to a loaded Creature that was stored in m_mGoEntryGuidStore. Can return NULL
Creature* ScriptedInstance::GetSingleCreatureFromStorage(uint32 uiEntry, bool bSkipDebugLog /*=false*/)
{
    EntryGuidMap::iterator find = m_mNpcEntryGuidStore.find(uiEntry);
    if (find != m_mNpcEntryGuidStore.end())
        return instance->GetCreature(find->second);

    // Output log, possible reason is not added GO to map, or not yet loaded;
    if (!bSkipDebugLog)
        debug_log("SD0: Script requested creature with entry %u, but no npc of this entry was created yet, or it was not stored by script for map %u.", uiEntry, instance->GetId());

    return NULL;
}

void ScriptedInstance::HandleGameObject(ObjectGuid guid, bool open)
{
    if (GameObject* pGo = instance->GetGameObject(guid))
        pGo->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
    else
        debug_log("SD0: InstanceData: HandleGameObject failed");
}

/// Function that uses a door or button that is stored in m_mGoEntryGuidStore
void ScriptedInstance::HandleGameObject(uint32 uiEntry, bool open)
{
    EntryGuidMap::iterator find = m_mGoEntryGuidStore.find(uiEntry);
    if (find != m_mGoEntryGuidStore.end())
        HandleGameObject(find->second, open);
    else
        // Output log, possible reason is not added GO to storage, or not yet loaded
        debug_log("SD0: Script call HandleGameObject(by Entry), but no gameobject of entry %u was created yet, or it was not stored by script for map %u.", uiEntry, instance->GetId());
}

void ScriptedInstance::InteractWithGo(ObjectGuid guid, bool state)
{
    if (GameObject* pGo = instance->GetGameObject(guid))
	{
		if (state)
			pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND + GO_FLAG_NO_INTERACT);
		else
			pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND + GO_FLAG_NO_INTERACT);
	}
}

void ScriptedInstance::InteractWithGo(uint32 uiEntry, bool state)
{
    EntryGuidMap::iterator find = m_mGoEntryGuidStore.find(uiEntry);
    if (find != m_mGoEntryGuidStore.end())
        InteractWithGo(find->second, state);
    else
        // Output log, possible reason is not added GO to storage, or not yet loaded
        debug_log("SD0: Script call InteractWithGo(by Entry), but no gameobject of entry %u was created yet, or it was not stored by script for map %u.", uiEntry, instance->GetId());
}

bool ScriptedInstance::SetRareBoss(Creature* pCreature, uint32 uiChance)
{
    if (uiChance < urand(0,100))
    {
        pCreature->setFaction(35);
        pCreature->SetVisibility(VISIBILITY_OFF);
        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        return false;
    }
    else
    {
        pCreature->setFaction(pCreature->GetCreatureInfo()->faction_A);
        pCreature->SetVisibility(VISIBILITY_ON);
        pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        return true;
    }
}

void ScriptedInstance::SetLavaState(Player* pPlayer, bool enter)
{
    if (!pPlayer)
        return;

    if (enter)
        pPlayer->m_LavaTimer = LAVA_DAMAGE_PERIOD;
    else
        pPlayer->m_LavaTimer = DISABLED_MIRROR_TIMER;
}

void ScriptedInstance::DoLavaDamage(const uint32 uiDiff)
{
    Map::PlayerList const& lPlayers = instance->GetPlayers();
    if (lPlayers.isEmpty())
        return;

    for(Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
    {
        Player* pPlayer = itr->getSource();
        if (pPlayer && ((pPlayer->m_LavaActive == true && !pPlayer->IsInWater()) || pPlayer->IsInWater()))
        {
            if (pPlayer->m_LavaTimer < uiDiff)
            {
                pPlayer->m_LavaTimer = LAVA_DAMAGE_PERIOD;
                pPlayer->EnvironmentalDamage(DAMAGE_LAVA, urand(600, 700));
            }
            else
                pPlayer->m_LavaTimer -= uiDiff;
        }
    }
}