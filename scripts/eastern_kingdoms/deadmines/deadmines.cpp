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
SDName: Deadmines
SD%Complete: 90
SDComment: Contains GO for event at end door
SDCategory: Deadmines
EndScriptData */

#include "precompiled.h"
#include "deadmines.h"

bool GOUse_go_door_lever_dm(Player* pPlayer, GameObject* pGo)
{
    instance_deadmines* m_pInstance = (instance_deadmines*)pGo->GetInstanceData();

    if (!m_pInstance)
        return false;

	GameObject* pGoDoor = m_pInstance->GetSingleGameObjectFromStorage(GO_IRON_CLAD);

    if (pGoDoor && pGoDoor->GetGoState() == 1)
        return false;

    return true;
}

bool GOUse_go_defias_cannon(Player* pPlayer, GameObject* pGo)
{
    instance_deadmines* m_pInstance = (instance_deadmines*)pGo->GetInstanceData();

    if (!m_pInstance)
        return false;

    if (m_pInstance->GetData(TYPE_DEFIAS_ENDDOOR) == DONE || m_pInstance->GetData(TYPE_DEFIAS_ENDDOOR) == IN_PROGRESS)
        return false;

    m_pInstance->SetData(TYPE_DEFIAS_ENDDOOR, IN_PROGRESS);
    return false;
}

void AddSC_deadmines()
{
    Script* pNewscript;

    pNewscript = new Script;
    pNewscript->Name = "go_door_lever_dm";
    pNewscript->pGOUse = &GOUse_go_door_lever_dm;
    pNewscript->RegisterSelf();

    pNewscript = new Script;
    pNewscript->Name = "go_defias_cannon";
    pNewscript->pGOUse = &GOUse_go_defias_cannon;
    pNewscript->RegisterSelf();
}
