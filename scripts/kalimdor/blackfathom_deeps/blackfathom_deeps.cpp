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
SDName: Blackfathom_Deeps
SD%Complete: 10
SDComment: Quest support: 6921,
SDCategory: Blackfathom Deeps
EndScriptData */

/* ContentData
go_blackfathom_altar
go_blackfathom_fire
go_fathom_stone
EndContentData */

#include "precompiled.h"
#include "blackfathom_deeps.h"

/*######
## go_blackfathom_altar
## go_blackfathom_fire
######*/

enum Spells
{
    SPELL_BLESSING_OF_BLACKFATHOM             = 8733,
    SPELL_BLACKFATHOM_TELEPORT                = 8735
};

static float Spawns[3][3] =
{
    {-818.338f,-180.645f,-25.891f},
    {-823.959f,-190.843f,-25.8718f},
    {-813.305f,-191.297f,-25.8718f}
};

static float Creatures[5][3] =
{
    {4823,4821,4822},	//crab
    {4818,4819,4820},	//murloc
    {4825,4825,4825},	//turtle
    {4978,4978,4978},	//elemental
    {4815,4815,4815}	//makrura
};

bool GOUse_go_blackfathom_altar(Player* pPlayer, GameObject* pGo)
{
    if (!pPlayer->HasAura(SPELL_BLESSING_OF_BLACKFATHOM))
        pPlayer->CastSpell(pPlayer, SPELL_BLESSING_OF_BLACKFATHOM, true);

    if (pGo->GetEntry() == GO_ALTAR_OF_THE_DEEPS)
        pPlayer->CastSpell(pPlayer, SPELL_BLACKFATHOM_TELEPORT, true);

    return true;
}

bool GOUse_go_blackfathom_fire(Player* pPlayer, GameObject* pGo)
{
    instance_blackfathom_deeps* m_pInstance = (instance_blackfathom_deeps*)pGo->GetInstanceData();
    if (m_pInstance)
    {
        pGo->SetGoState(GO_STATE_ACTIVE);
        pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
        m_pInstance->SetData(TYPE_SHRINE, IN_PROGRESS);

        uint8 random = urand(0,4);
        for (uint8 i = 0; i < 4; ++i)
            pGo->SummonCreature(Creatures[random][i], Spawns[i][0], Spawns[i][1], Spawns[i][2], 1.59f, TEMPSUMMON_DEAD_DESPAWN, 5000);
        return true;
    }
    return false;
}

/*######
## go_fathom_stone
######*/

bool GOUse_go_fathom_stone(Player* pPlayer, GameObject* pGo)
{
    Position pos;
    pGo->GetClosePoint(pos.x, pos.y, pos.z, pPlayer->GetObjectBoundingRadius());    // possible crash :)
    pGo->SummonCreature(NPC_BARON_AQUANIS, pos.x, pos.y, pos.z, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);
    return true;
}

/* AddSC */

void AddSC_blackfathom_deeps()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "go_blackfathom_altar";
    pNewScript->pGOUse = &GOUse_go_blackfathom_altar;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_blackfathom_fire";
    pNewScript->pGOUse = &GOUse_go_blackfathom_fire;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_fathom_stone";
    pNewScript->pGOUse = &GOUse_go_fathom_stone;
    pNewScript->RegisterSelf();
}
