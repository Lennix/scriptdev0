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
SDName: Bosses Ebonroc, Firemaw, Flamegor
SD%Complete: 100
SDComment: -
SDCategory: Blackwing Lair
EndScriptData */


#include "precompiled.h"
#include "blackwing_lair.h"

enum eBlackwingDragon
{
    EMOTE_GENERIC_FRENZY        = -1000002,

    SPELL_SHADOW_FLAME          = 22539,
    SPELL_THRASH                = 3391,			//Firemaw
    SPELL_WING_BUFFET           = 23339,

    SPELL_SHADOW_OF_EBONROC     = 23340,        // Ebonroc
    SPELL_FLAME_BUFFET          = 23341,        // Firemaw
    SPELL_FRENZY                = 23342         // Flamegor
};

#define SUMMON_PLAYER_TRIGGER_DISTANCE 65.0f

struct MANGOS_DLL_DECL boss_blackwing_dragonAI : public ScriptedAI
{
    boss_blackwing_dragonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackwing_lair*)pCreature->GetInstanceData();
        Reset();

        switch(m_creature->GetEntry())
        {
            case NPC_EBONROC:
                m_uiSpecialSpellId = SPELL_SHADOW_OF_EBONROC;
                break;
            case NPC_FIREMAW:
                m_uiSpecialSpellId = SPELL_FLAME_BUFFET;
                break;
            case NPC_FLAMEGOR:
                m_uiSpecialSpellId = SPELL_FRENZY;
        }
    }

    instance_blackwing_lair* m_pInstance;

    uint32 m_uiShadowFlameTimer;
    uint32 m_uiWingBuffetTimer;

    uint32 m_uiSpecialSpellId;
    uint32 m_uiSpecialSpellTimer;

    bool spellHits;
    bool meleeHits;

    void Reset()
    {
        m_uiShadowFlameTimer = urand(8000, 15000);
        m_uiWingBuffetTimer = urand(12000, 25000);
        m_uiSpecialSpellTimer = urand(2000, 8000);
        spellHits = false;
        meleeHits = false;
    }
    
    // --- MeleeHit Abfrage --- unschöne implementierung funktioniert aber !!!
    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)			//
    {																//
        if (pCaster ==  m_creature)									//
            spellHits = true;										//
    }																//
    void DamageDeal(Unit* pAttacker, uint32 pDamage)				//
    {																//
        if (spellHits)												//
            spellHits = !spellHits;												//
        else														//
            meleeHits = true;										//
    }																//
    // --- MeleeHit Abfrage --- unschöne implemntierung funktioniert aber !!!
    
    //Wing Buffet - reduziert die aggro der spieler erheblich die von wing buffet getroffen wurden
    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell)
    {
        if (pSpell->Id == SPELL_WING_BUFFET )
        {
            m_creature->getThreatManager().modifyThreatPercent(pTarget, -75);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;
        
        // --- SPECIAL ABILITY ---
        if (m_uiSpecialSpellTimer <= uiDiff)
        {
            switch(m_uiSpecialSpellId)
            {
                //FLAMEGOR
                case SPELL_FRENZY :
                {
                    if (DoCastSpellIfCan(m_creature, m_uiSpecialSpellId) == CAST_OK)
                    {
                        DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                        m_uiSpecialSpellTimer = (7000, 9000);
                    }
                    break;
                }

                //FIREMAW
                case SPELL_FLAME_BUFFET :
                {
                    Map *pMap = m_creature->GetMap();
                    Map::PlayerList const &PlayerList = pMap->GetPlayers();
                    if (!PlayerList.isEmpty())
                    {
                        for(Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
                        {
                            Player* pPlayer = itr->getSource();
                            if (pPlayer)
                            {
                                //Alle Spieler LineOfSight bekommen den Debuff alle 2 Sekunden ab
                                if(m_creature->IsWithinLOSInMap(pPlayer))
                                {
                                    m_creature->CastSpell(pPlayer, m_uiSpecialSpellId, true);
                                }
                            }
                        }
                    }
                    m_uiSpecialSpellTimer = 2000;
                    break;
                }

                //EBONROC
                case SPELL_SHADOW_OF_EBONROC :
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), m_uiSpecialSpellId) == CAST_OK)
                        m_uiSpecialSpellTimer = (15000, 20000);
                }
            }
        }
        else
            m_uiSpecialSpellTimer -= uiDiff;
        
        // --- FIREMAW ABILITY ---
        if (m_creature->GetEntry() == NPC_FIREMAW)
        {
            //Thrash - für jeden getroffenen  meleeHit chance von 35% Thrash zu bekommen (normal attackspeed 2.0)
            if (meleeHits)
            {
                uint8 roll = urand(0,3);
                if (roll < 1)
                    m_creature->CastSpell(m_creature->getVictim(), SPELL_THRASH, true);
                meleeHits = !meleeHits;
            }
        }
        
        // --- TRIO ABILITIES ---

        // Shadow Flame
        if (m_uiShadowFlameTimer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_FLAME) == CAST_OK)
                m_uiShadowFlameTimer = urand(15000, 22000);
        }
        else
            m_uiShadowFlameTimer -= uiDiff;

        // Wing Buffet - reduziert aggro bei hit (siehe Oben)
        if (m_uiWingBuffetTimer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WING_BUFFET) == CAST_OK)
                m_uiWingBuffetTimer = 25000;      
        }
        else
            m_uiWingBuffetTimer -= uiDiff;
    
        
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_blackwing_dragon(Creature* pCreature)
{
    return new boss_blackwing_dragonAI(pCreature);
}

void AddSC_boss_blackwing_dragon()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "boss_blackwing_dragon";
    pNewScript->GetAI = &GetAI_boss_blackwing_dragon;
    pNewScript->RegisterSelf();
}