/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Akilzon
SD%Complete: 0
SDComment: Place Holder
SDCategory: Zul'Aman
EndScriptData */

#include "precompiled.h"
#include "def_zulaman.h"

enum
{
    SAY_EVENT1              = -1568024,
    SAY_EVENT2              = -1568025,
    SAY_AGGRO               = -1568026,
    SAY_SUMMON              = -1568027,
    SAY_SUMMON_ALT          = -1568028,
    SAY_ENRAGE              = -1568029,
    SAY_SLAY1               = -1568030,
    SAY_SLAY2               = -1568031,
    SAY_DEATH               = -1568032,
    EMOTE_STORM             = -1568033,

    SPELL_STATIC_DISRUPTION = 43622,
    SPELL_STATIC_VISUAL     = 45265,
    SPELL_CALL_LIGHTNING    = 43661,
    SPELL_GUST_OF_WIND      = 43621,
    SPELL_ELECTRICAL_STORM  = 43648,
    SPELL_BERSERK           = 45078,
    SPELL_EAGLE_SWOOP       = 44732,

    MOB_SOARING_EAGLE       = 24858,

    SE_LOC_X_MAX            = 400,
    SE_LOC_X_MIN            = 335,
    SE_LOC_Y_MAX            = 1435,
    SE_LOC_Y_MIN            = 1370,

    MOB_TEMP_TRIGGER        = 23920,
};