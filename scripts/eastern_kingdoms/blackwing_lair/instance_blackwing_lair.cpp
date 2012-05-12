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
SDName: Instance_Blackwing_Lair
SD%Complete: 0
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "blackwing_lair.h"

instance_blackwing_lair::instance_blackwing_lair(Map* pMap) : ScriptedInstance(pMap),

    m_uiRazorgoreSummonTimer(TIMER_START_SPAWMING_ADDS),
    m_uiDragonkinSummoned(0),
    m_uiOrcSummoned(0)
{
    Initialize();
}

void instance_blackwing_lair::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    m_lBlackwingGuardsman.clear();
    m_lBlackDragonEgg.clear();
    m_lDragonTrio.clear();
    m_lTempList.clear();
}

void instance_blackwing_lair::OnCreatureCreate(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_BLACKWING_GUARDSMAN:
            m_lBlackwingGuardsman.push_back(pCreature->GetObjectGuid());
            return;
        case NPC_BLACKWING_TECHNICIAN:
            // hide event vael goblins if we enter instance and vael is already defeated
            if (pCreature->IsWithinDist2d(aNefariusSpawnLoc[0], aNefariusSpawnLoc[1], 50.0f) && GetData(TYPE_VAELASTRASZ) == DONE)
            {
                pCreature->SetVisibility(VISIBILITY_OFF);
                pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE + UNIT_FLAG_NOT_SELECTABLE);
            }
            return;
        case NPC_GRETHOK_THE_CONTROLLER:
        case NPC_RAZORGORE:
        case NPC_VAELASTRASZ:
        case NPC_BROODLORD:
            break;
        case NPC_NEFARIAN:
            //BWL REALEASE PART ONE - NEFARIAN IS NOT AVAILABLE
            pCreature->SetVisibility(VISIBILITY_OFF);
            break;
        case NPC_FIREMAW:
        case NPC_EBONROC:
        case NPC_FLAMEGOR:
            //BWL REALEASE PART ONE - DRAGONTRIO IS NOT AVAILABLE
            pCreature->SetVisibility(VISIBILITY_OFF);
            m_lDragonTrio.push_back(pCreature->GetObjectGuid());
            return;
        case NPC_CHROMAGGUS:
            //BWL REALEASE PART ONE - CHROMAGGUS IS NOT AVAILABLE
            pCreature->SetVisibility(VISIBILITY_OFF);
            // VALUE_BREATH1, VALUE_BREATH2 and so forth are the ID-specific spells
            if (GetData(VALUE_BREATH1) == NOT_STARTED && GetData(VALUE_BREATH2) == NOT_STARTED)
                SetData(VALUE_BREATH1, DONE);
            return;
        default:
            return;
    }

    m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
}

void instance_blackwing_lair::OnCreatureEnterCombat(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_GRETHOK_THE_CONTROLLER:
        case NPC_RAZORGORE:
        {
            if (pCreature->GetEntry() != NPC_RAZORGORE)
            {
                if (Creature* pRazorgore = GetSingleCreatureFromStorage(NPC_RAZORGORE))
                    pRazorgore->SetInCombatWithZone();
            }
            else
                pCreature->SetInCombatWithZone();

            break;
        }
        case NPC_VAELASTRASZ:
            HandleGameObject(GO_PORTCULLIS_RAZORGORE, false);
            HandleGameObject(GO_PORTCULLIS_ENTRANCE, false);
            break;
        default:
            break;
    }
}

void instance_blackwing_lair::OnCreatureEvade(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_GRETHOK_THE_CONTROLLER:                // Reset Razorgore encounter
        case NPC_RAZORGORE:
        {
            m_uiRazorgoreSummonTimer = TIMER_START_SPAWMING_ADDS;
            m_uiDragonkinSummoned = 0;
            m_uiOrcSummoned = 0;
            m_lTempList.clear();

            for(GUIDList::iterator itr = m_lBlackDragonEgg.begin(); itr != m_lBlackDragonEgg.end(); ++itr)
            {
                if (GameObject* pEgg = instance->GetGameObject(*itr))
                    pEgg->Respawn();
            }

            for(GUIDList::iterator itr = m_lBlackwingGuardsman.begin(); itr != m_lBlackwingGuardsman.end(); ++itr)
            {
                Creature* pGuard = instance->GetCreature(*itr);
                if (pGuard && !pGuard->isAlive())
                    pGuard->Respawn();
            }

            if (GameObject* Orb = GetSingleGameObjectFromStorage(GO_ORB_OF_DOMINATION))
                Orb->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);

            Creature* pGrethok = GetSingleCreatureFromStorage(NPC_GRETHOK_THE_CONTROLLER);
            if (pGrethok && !pGrethok->isAlive())
                pGrethok->Respawn();
            break;
        }
        case NPC_VAELASTRASZ:
            HandleGameObject(GO_PORTCULLIS_RAZORGORE, true);
            break;
        default:
            break;
    }
}

void instance_blackwing_lair::OnCreatureDeath(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_GRETHOK_THE_CONTROLLER:                // Razorgore encounter - start destroying eggs phase
            if (GameObject* Orb = GetSingleGameObjectFromStorage(GO_ORB_OF_DOMINATION))
                Orb->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            m_lTempList = m_lBlackDragonEgg;
            break;
        case NPC_VAELASTRASZ:
            SetData(TYPE_VAELASTRASZ, DONE);
            break;
        case NPC_BROODLORD:
            SetData(TYPE_BROODLORD, DONE);
            break;
        case NPC_EBONROC:
        case NPC_FLAMEGOR:
        case NPC_FIREMAW:
            if (CheckDragonTrioState())
                SetData(TYPE_DRAGON_TRIO, DONE);
            break;
        case NPC_CHROMAGGUS:
            SetData(TYPE_CHROMAGGUS, DONE);
            break;
        case NPC_BLACKWING_LEGIONNAIRE:
        case NPC_BLACKWING_MAGE:
            m_uiOrcSummoned--;
            break;
        case NPC_DEATH_TALON_DRAGONSPAWN:
            m_uiDragonkinSummoned--;
            break;
    }
}

void instance_blackwing_lair::OnObjectCreate(GameObject* pGo)
{
    switch(pGo->GetEntry())
    {
        case GO_BLACK_DRAGON_EGG:
            m_lBlackDragonEgg.push_back(pGo->GetObjectGuid());
            return;
        case GO_PORTCULLIS_ENTRANCE:
            break;
        case GO_PORTCULLIS_RAZORGORE:
            if (m_auiEncounter[0] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORTCULLIS_VAELASTRASZ:
            if (m_auiEncounter[1] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORTCULLIS_BROODLORD:
            if (m_auiEncounter[2] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORTCULLIS_CHROMAGGUS:
            if (m_auiEncounter[3] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORTCULLIS_NEFARIAN:
            if (m_auiEncounter[4] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ORB_OF_DOMINATION:
            pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;
        default:
            return;
    }

    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_blackwing_lair::OnObjectUse(GameObject* pGo)
{
    switch(pGo->GetEntry())
    {
        case GO_BLACK_DRAGON_EGG:
        {
            m_lTempList.remove(pGo->GetObjectGuid());

            if (m_lTempList.empty())
            {
                //start phase 2 if all eggs destroyed
                SetData(TYPE_RAZORGORE, SPECIAL);
                //cant use orb of domination anymore
                if (GameObject* Orb = GetSingleGameObjectFromStorage(GO_ORB_OF_DOMINATION))
                    Orb->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                //despawn all adds
                for(std::list<Creature*>::const_iterator it = razorgoreAdds.begin(); it != razorgoreAdds.end(); ++it)
                {
                    if (*it)
                    {
                        (*it)->ForcedDespawn();
                        (*it)->AddObjectToRemoveList();
                    }
                }
                razorgoreAdds.clear();

                if (Creature* pRazorgore = GetSingleCreatureFromStorage(NPC_RAZORGORE))
                {
                    //razorgore gets full health
                    pRazorgore->SetHealthPercent(100.0f);

                    //handle last controller
                    if (Unit* pController = pRazorgore->GetCharmerOrOwner())
                    {
                        pController->RemoveAurasDueToSpell(SPELL_USE_DRAGON_ORB);
                        pRazorgore->RemoveAurasDueToSpell(SPELL_USE_DRAGON_ORB);
                        pRazorgore->AI()->AttackStart(pController);
                        pRazorgore->TauntApply(pController);
                    }
                }
            }
            else
                debug_log("BWL: Black Dragon Eggs left to destroy: %u", m_lTempList.size());
            break;
        }
        default:
            break;
    }
}

void instance_blackwing_lair::SetData(uint32 uiType, uint32 uiData)
{
    switch(uiType)
    {
        case TYPE_RAZORGORE:
            m_auiEncounter[0] = uiData;
            switch (uiData)
            {
                case FAIL:
                    HandleGameObject(GO_PORTCULLIS_ENTRANCE, true);
                    break;
                case IN_PROGRESS:
                    HandleGameObject(GO_PORTCULLIS_ENTRANCE, false);
                    break;
                case DONE:
                    HandleGameObject(GO_PORTCULLIS_ENTRANCE, true);
                    HandleGameObject(GO_PORTCULLIS_RAZORGORE, true);
                    break;
            }
            break;
        case TYPE_VAELASTRASZ:
            m_auiEncounter[1] = uiData;
            if (uiData == DONE)
            {
                HandleGameObject(GO_PORTCULLIS_RAZORGORE, true);
                HandleGameObject(GO_PORTCULLIS_VAELASTRASZ, true);
            }
            break;
        /* BWL REALEASE PART ONE - ENDS HERE
        case TYPE_BROODLORD:
            m_auiEncounter[2] = uiData;
            if (uiData == DONE)
                HandleGameObject(GO_PORTCULLIS_BROODLORD, true);
            break;
         */
        case TYPE_DRAGON_TRIO:
            m_auiEncounter[3] = uiData;
            if (uiData == DONE)
            {
                HandleGameObject(GO_PORTCULLIS_CHROMAGGUS, true);
                // workaround since mobs are aggroable through GOs
                if (Creature* pChromaggus = GetSingleCreatureFromStorage(NPC_CHROMAGGUS))
                    pChromaggus->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
            break;
        case TYPE_CHROMAGGUS:
            m_auiEncounter[4] = uiData;
            if (uiData == DONE)
                HandleGameObject(GO_PORTCULLIS_NEFARIAN, true);
            break;
        case VALUE_BREATH1:
            //genrate chromaggus breath and nefarian dragon
            for (uint8 i = 0; i < 2; i++)
            {
                uint8 uiPos1 = urand(0, 4);
                uint8 uiPos2 = 0;
                do { uiPos2 = urand(0, 4); } while (uiPos1 == uiPos2);
                m_auiEncounter[VALUE_BREATH1 + (i * 2)] = uiPos1;
                m_auiEncounter[VALUE_BREATH1+1 + (i * 2)] = uiPos2;
            }
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
            << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " " 
            << m_auiEncounter[6] << " " << m_auiEncounter[7] << " "
            << m_auiEncounter[8] << " " << m_auiEncounter[9];

        strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_blackwing_lair::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
        >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7] >> m_auiEncounter[8]
        >> m_auiEncounter[9];

    // Dont set the spell values to NOT_STARTED (MAX_ENCOUNTER -4)
    for(uint32 i = 0; i < MAX_ENCOUNTER - 4; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_blackwing_lair::GetData(uint32 uiType)
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

//IF RAZORGORE IS CONTROLLED HE WONT USE HIS SCRIPT ANYMORE, THEREFORE WE HAVE TO SCRIPT THIS SHIT HERE
void instance_blackwing_lair::Update(uint32 uiDiff)
{
    //IN_PROGRESS = Razorgore Phase 1
    if (GetData(TYPE_RAZORGORE) == IN_PROGRESS)
    {
        
        if (m_uiRazorgoreSummonTimer <= uiDiff)
        {
            if (Creature* pRazorgore = GetSingleCreatureFromStorage(NPC_RAZORGORE))
            {
                for (uint32 i = 0; i < 8; i++)
                {
                    /* SPAWNS BY VALUE
                     * ===========
                     * 0         = nothing
                     * 1 - 3     = orc
                     * 4         = dragon
                     */
                     
                    uint8 orcCount = 0;
                    if (m_uiOrcSummoned < MAX_BLACKWING_ORC)
                        orcCount = 3;

                    uint8 dragonCount = 0;
                    if (m_uiDragonkinSummoned < MAX_BLACKWING_DRAGONKIN)
                        dragonCount = 1;

                    if (orcCount || dragonCount)
                    {
                        uint32 spawnType = urand(1, orcCount + dragonCount);
                        if (!orcCount)
                            spawnType = 4;
                   
                         Creature* spawnedAdd = 0;

                        if (spawnType < 4)
                        {
                            ++m_uiOrcSummoned;
                            spawnedAdd = pRazorgore->SummonCreature(urand(0,1) ? NPC_BLACKWING_LEGIONNAIRE : NPC_BLACKWING_MAGE, Corner[i].x,
                                    Corner[i].y, Corner[i].z, Corner[i].o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000);                 
                        }
                        else
                        {
                            ++m_uiDragonkinSummoned;
                            spawnedAdd = pRazorgore->SummonCreature(NPC_DEATH_TALON_DRAGONSPAWN, Corner[i].x, Corner[i].y, Corner[i].z, Corner[i].o,
                                TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000);
                        }

                        //All adds have initial aggro on the Razorgore controller. The mage types will run to the center of the room and start to attack the controller with their ranged fireballs
                        Unit* pController = pRazorgore->GetCharmerOrOwner();
                        if (spawnedAdd)
                        {
                            razorgoreAdds.push_back(spawnedAdd);

                            if (pController && pController->isAlive())
                                spawnedAdd->AI()->AttackStart(pController);
                            else
                                spawnedAdd->SetInCombatWithZone(); 
                        }
                    }
                }
            }

            m_uiRazorgoreSummonTimer = TIMER_NEXT_ADD_SPAWN;
        }
        else
            m_uiRazorgoreSummonTimer -= uiDiff;
    }
}

bool instance_blackwing_lair::CheckDragonTrioState()
{
    for(GUIDList::iterator itr = m_lDragonTrio.begin(); itr != m_lDragonTrio.end(); ++itr)
    {
        Creature* pDragon = instance->GetCreature(*itr);
        if (pDragon && pDragon->isAlive())
            return false;
    }

    return true;
}

InstanceData* GetInstanceData_instance_blackwing_lair(Map* pMap)
{
    return new instance_blackwing_lair(pMap);
}

void AddSC_instance_blackwing_lair()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_blackwing_lair";
    pNewScript->GetInstanceData = &GetInstanceData_instance_blackwing_lair;
    pNewScript->RegisterSelf();
} 
