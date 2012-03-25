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

//AV is a Battleground, but for global quest handling we need Events based on scriptdev0 too!
#ifndef DEF_AV_H
#define DEF_AV_H

enum Instance_BG_AV
{
    EVENT_MASTERS_START_SUMMONING_H     = 0,
    EVENT_MASTERS_START_SUMMONING_A     = 1,
    EVENT_ENDBOSS_STATUS_H              = 2,
    EVENT_ENDBOSS_STATUS_A              = 3,

    MAX_EVENTS                          = 4,

    ARRIVED_BASE                        = 5,

    BOSS_START_TIME                     = 600000    //10 minutes
};

class MANGOS_DLL_DECL instance_BG_AV : public ScriptedInstance
{
    public:
        instance_BG_AV(Map* pMap);
        ~instance_BG_AV() {}

        void Initialize();
        void SetData(uint32 uiType, uint32 uiData);
        void Update(uint32 uiDiff);

        uint32 GetData(uint32 uiType);

    protected:
        uint32 moveBossTimer[2];
        uint32 m_uiEvent[MAX_EVENTS];
};

#endif