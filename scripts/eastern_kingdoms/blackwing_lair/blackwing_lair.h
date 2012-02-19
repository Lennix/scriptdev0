/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_BLACKWING_LAIR_H
#define DEF_BLACKWING_LAIR_H

enum Data
{
    TYPE_RAZORGORE,
    TYPE_VAELASTRASZ,
    TYPE_BROODLORD,
    TYPE_DRAGON_TRIO,
    TYPE_CHROMAGGUS,
    TYPE_NEFARIAN,
    VALUE_BREATH1,
    VALUE_BREATH2,
    VALUE_DRAGON1,
    VALUE_DRAGON2,
    MAX_ENCOUNTER
};

enum Creatures
{
    // Bosses
    NPC_RAZORGORE               = 12435,
    NPC_GRETHOK_THE_CONTROLLER  = 12557,
    NPC_VAELASTRASZ             = 13020,
    NPC_BROODLORD               = 12017,
    NPC_FIREMAW                 = 11983,
    NPC_EBONROC                 = 14601,
    NPC_FLAMEGOR                = 11981,
    NPC_CHROMAGGUS              = 14020,
    NPC_NEFARIAN                = 11583,
    NPC_VICTOR_NEFARIUS         = 10162,

    NPC_BLACK_DRAKONID          = 14265,
    NPC_BLUE_DRAKONID           = 14261,
    NPC_BRONZE_DRAKONID         = 14263,
    NPC_CHROMATIC_DRAKONID      = 14302,
    NPC_GREEN_DRAKONID          = 14262,
    NPC_RED_DRAKONID            = 14264,

    NPC_BLACKWING_GUARDSMAN     = 14456,
    NPC_BLACKWING_LEGIONNAIRE   = 12416,
    NPC_BLACKWING_MAGE          = 12420,
    NPC_DEATH_TALON_DRAGONSPAWN = 12422,
    NPC_BLACKWING_TECHNICIAN    = 13996,                    // Flees at Vael intro event
};

enum GameObjects
{
    GO_BLACK_DRAGON_EGG         = 177807,
    GO_PORTCULLIS_ENTRANCE      = 176964,
    GO_PORTCULLIS_RAZORGORE     = 176965,                       // Door after Razorgore
    GO_PORTCULLIS_VAELASTRASZ   = 179364,                       // Door after Vaelastrasz
    GO_PORTCULLIS_BROODLORD     = 179365,                       // Door after Broodlord
    GO_PORTCULLIS_CHROMAGGUS    = 179116,                       // Door before Chromaggus
    GO_PORTCULLIS_NEFARIAN      = 179117,                       // Door before Nefarian
    GO_SUPPRESSION_DEVICE       = 179784
};

enum Misc
{
    FACTION_FRIENDLY            = 35,
    FACTION_BLACKWING           = 103,
    SPELL_USE_DRAGON_ORB        = 23018,

    MAX_BLACKWING_ORC           = 40,
    MAX_BLACKWING_DRAGONKIN     = 12
};

struct Location
{
    float x, y, z, o;
};

static Location Corner[]=                       // Spawn positions of Razorgore encounter
{
    {-7659.32f, -1042.81f, 407.21f, 6.05f},
    {-7643.92f, -1064.61f, 407.21f, 0.50f},
    {-7623.02f, -1094.21f, 407.21f, 0.67f},
    {-7607.71f, -1116.54f, 407.21f, 1.37f},
    {-7584.31f, -990.123f, 407.21f, 4.50f},
    {-7568.75f, -1012.85f, 407.21f, 3.71f},
    {-7548.45f, -1041.96f, 407.21f, 3.79f},
    {-7532.35f, -1064.68f, 407.21f, 2.94f},
};

struct DrakonidInfo
{
    DrakonidInfo(float _x, float _y, float _z)
        : x(_x), y(_y), z(_z) {}

    float x, y, z;
};

typedef std::list<DrakonidInfo> DrakonidInfoList;

// Coords used to spawn Nefarius at the throne
static const float aNefariusSpawnLoc[4] = {-7466.16f, -1040.80f, 412.053f, 2.14675f};

class MANGOS_DLL_DECL instance_blackwing_lair : public ScriptedInstance
{
    public:
        instance_blackwing_lair(Map* pMap);
        ~instance_blackwing_lair() {}

        void Initialize();

        void OnCreatureCreate(Creature* pCreature);
        void OnCreatureEnterCombat(Creature* pCreature);
        void OnCreatureEvade(Creature* pCreature);
        void OnCreatureDeath(Creature* pCreature);
        void OnObjectCreate(GameObject* pGo);
        void OnObjectUse(GameObject* pGo);

        void SetData(uint32 uiType, uint32 uiData);
        uint32 GetData(uint32 uiType);

        const char* Save() { return strInstData.c_str(); }
        void Load(const char* chrIn);

        void Update(uint32 uiDiff);

        bool CheckDragonTrioState();
    protected:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string strInstData;

        bool m_bRazorgoreSummon;

        uint32 m_uiRazorgoreSummonTimer;
        uint32 m_uiDragonkinSummoned;
        uint32 m_uiOrcSummoned;

        GUIDList m_lBlackwingGuardsman;
        GUIDList m_lBlackDragonEgg;
        GUIDList m_lTempList;
        GUIDList m_lDragonTrio;

        GUIDList m_lTechnicianGuids;
};

#endif
