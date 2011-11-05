/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_DEADMINES_H
#define DEF_DEADMINES_H

enum
{
    MAX_ENCOUNTER           = 1,

    TYPE_DEFIAS_ENDDOOR     = 1,
    DATA_DEFIAS_DOOR        = 2,

    INST_SAY_ALARM1         = -1036000,
    INST_SAY_ALARM2         = -1036001,

    GO_DOOR_LEVER           = 101833,
    GO_IRON_CLAD            = 16397,
    GO_DEFIAS_CANNON        = 16398,
    NPC_MR_SMITE            = 646,
    NPC_PIRATE              = 657
};

class MANGOS_DLL_DECL instance_deadmines : public ScriptedInstance
{
    public:
        instance_deadmines(Map* pMap);
        ~instance_deadmines() {}

        void Initialize();

        void OnCreatureCreate(Creature* pCreature);
		void OnObjectCreate(GameObject* pGo);

        void SetData(uint32 uiType, uint32 uiData);
        uint32 GetData(uint32 uiType);

		void Update(uint32 uiDiff);
    protected:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string strInstData;

		uint32 m_uiIronDoor_Timer;
		uint32 m_uiDoor_Step;
};

#endif
