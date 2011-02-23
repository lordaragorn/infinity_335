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
SDName: Boss_Janalai
SD%Complete: 100
SDComment:
SDCategory: Zul'Aman
EndScriptData */

#include "precompiled.h"
#include "zulaman.h"

enum
{
    SAY_AGGRO                 =  -1568000,//(-1568000,'Spirits of da wind be your doom!',12031,1,0,'janalai SAY_AGGRO'),
    SAY_FIRE_BOMBS            =  -1568001,//(-1568001,'I burn ya now!',12032,1,0,'janalai SAY_FIRE_BOMBS'),
    SAY_SUMMON_HATCHER        =  -1568002,//(-1568002,'Where ma hatcha? Get to work on dem eggs!',12033,1,0,'janalai SAY_SUMMON_HATCHER'),
    SAY_ALL_EGGS              =  -1568003,//(-1568003,'I show you strength... in numbers.',12034,1,0,'janalai SAY_ALL_EGGS'),
    SAY_BERSERK               =  -1568004,//(-1568004,'You done run outta time!',12035,1,0,'janalai SAY_BERSERK'),
    SAY_SLAY_1                =  -1568005,//(-1568005,'It all be over now, mon!',12036,1,0,'janalai SAY_SLAY_1'),
    SAY_SLAY_2                =  -1568006,//(-1568006,'Tazaga-choo!',12037,1,0,'janalai SAY_SLAY_2'),
    SAY_DEATH                 =  -1568007,//(-1568007,'Zul\'jin... got a surprise for you...',12038,1,0,'janalai SAY_DEATH'),
    SAY_EVENT_STRANGERS       =  -1568008,//(-1568008,'Come, strangers. The spirit of the dragonhawk hot be hungry for worthy souls.',12039,1,0,'janalai SAY_EVENT_STRANGERS'),
    SAY_EVENT_FRIENDS         =  -1568009,//(-1568009,'Come, friends. Your bodies gonna feed ma hatchlings, and your souls are going to feed me with power!',12040,1,0,'janalai SAY_EVENT_FRIENDS'),
// Jan'alai
    SPELL_FLAME_BREATH        =  43140,
    SPELL_FIRE_WALL           =  43113,
    SPELL_ENRAGE              =  44779,
    SPELL_SUMMON_PLAYERS      =  43097,
    SPELL_TELE_TO_CENTER      =  43098 ,// coord
    SPELL_HATCH_ALL           =  43144,
    SPELL_BERSERK             =  45078,
// -- Fire Bomb Spells
    SPELL_FIRE_BOMB_CHANNEL    = 42621, // last forever
    SPELL_FIRE_BOMB_THROW      = 42628, // throw visual
    SPELL_FIRE_BOMB_DUMMY      = 42629, // bomb visual
    SPELL_FIRE_BOMB_DAMAGE     = 42630,
// -- Summons
    MOB_AMANI_HATCHER          = 23818,
    MOB_HATCHLING              = 23598,  // 42493
    MOB_EGG                    = 23817,
    MOB_FIRE_BOMB              = 23920,
// -- Hatcher Spells
    SPELL_HATCH_EGG            = 43734,   // 42471
// -- Hatchling Spells
    SPELL_FLAMEBUFFET          = 43299
};

const int area_dx = 44;
const int area_dy = 51;

float JanalainPos[1][3] =
{
    {-33.93f, 1149.27f, 19.0f}
};

float FireWallCoords[4][4] =
{
    {-10.13f, 1149.27f, 19.0f, 3.1415f},
    {-33.93f, 1123.90f, 19.0f, 0.5f*3.1415f},
    {-54.80f, 1150.08f, 19.0f, 0.0f},
    {-33.93f, 1175.68f, 19.0f, 1.5f*3.1415f}
};

float hatcherway[2][5][3] =
{
    {
        {-87.46f,1170.09f,6.0f},
        {-74.41f,1154.75f,6.0f},
        {-52.74f,1153.32f,19.0f},
        {-33.37f,1172.46f,19.0f},
        {-33.09f,1203.87f,19.0f}
    },
    {
        {-86.57f,1132.85f,6.0f},
        {-73.94f,1146.00f,6.0f},
        {-52.29f,1146.51f,19.0f},
        {-33.57f,1125.72f,19.0f},
        {-34.29f,1095.22f,19.0f}
    }
};

struct MANGOS_DLL_DECL boss_janalaiAI : public ScriptedAI
{
    boss_janalaiAI(Creature *c) : ScriptedAI(c)
    {
        pInstance =((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    ScriptedInstance *pInstance;

    uint32 FireBreathTimer;
    uint32 BombTimer;
    uint32 BombSequenceTimer;
    uint32 BombCount;
    uint32 HatcherTimer;
    uint32 EnrageTimer;
    uint32 ResetTimer;

    bool noeggs;
    bool enraged;
    bool isBombing;

    bool isFlameBreathing;

    uint64 FireBombGUIDs[40];

    void Reset()
    {
        if (pInstance)
            pInstance->SetData(TYPE_JANALAI, NOT_STARTED);

        FireBreathTimer = 8000;
        BombTimer = 30000;
        BombSequenceTimer = 1000;
        BombCount = 0;
        HatcherTimer = 10000;
        EnrageTimer = 300000;
        ResetTimer = 5000;

        noeggs = false;
        isBombing =false;
        enraged = false;

        isFlameBreathing = false;


        for(uint8 i = 0; i < 40; i++)
            FireBombGUIDs[i] = 0;

        HatchAllEggs(1);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

		if (pInstance)
		{
            pInstance->SetData(TYPE_JANALAI, DONE);
			if (pInstance->GetData64(DATA_BOSSKILLED)>=4) {
				if (GameObject* pEncounterDoor = pInstance->instance->GetGameObject(pInstance->GetData64(DATA_HEXLORDGATE)))
					pEncounterDoor->SetGoState(GO_STATE_ACTIVE);
			}
		}
    }

    void KilledUnit(Unit* victim)
    {
        switch(rand()%2)
        {
            case 0: DoScriptText(SAY_SLAY_1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY_2, m_creature); break;
        }
    }

    void Aggro(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (pInstance)
            pInstance->SetData(TYPE_JANALAI, IN_PROGRESS);
			m_creature->SetInCombatWithZone();
    }

    void DamageDeal(Unit* target, uint32 &damage)
    {
        if (isFlameBreathing)
        {
            if (!m_creature->HasInArc(M_PI_F/6, target))
                damage = 0;
        }
    }

    void FireWall()
    {
        uint8 WallNum;
        Creature* wall = NULL;
        for (uint8 i = 0; i < 4; i++)
        {
            if (i == 0 || i == 2)
                WallNum = 3;
            else
                WallNum = 2;

            for (uint8 j = 0; j < WallNum; j++)
            {
                if (WallNum == 3)
                    wall = m_creature->SummonCreature(MOB_FIRE_BOMB, FireWallCoords[i][0],FireWallCoords[i][1]+5*(j-1),FireWallCoords[i][2],FireWallCoords[i][3],TEMPSUMMON_TIMED_DESPAWN,15000);
                else
                    wall = m_creature->SummonCreature(MOB_FIRE_BOMB, FireWallCoords[i][0]-2+4*j,FireWallCoords[i][1],FireWallCoords[i][2],FireWallCoords[i][3],TEMPSUMMON_TIMED_DESPAWN,15000);
                if (wall) wall->CastSpell(wall, SPELL_FIRE_WALL, true);
            }
        }
    }

    void SpawnBombs()
    {
        float dx, dy;
        for ( int i(0); i < 40; i++)
        {
            dx =(rand()%(area_dx))-(area_dx/2);
            dy =(rand()%(area_dy))-(area_dy/2);

            Creature* bomb = DoSpawnCreature(MOB_FIRE_BOMB, dx, dy, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
            if (bomb) FireBombGUIDs[i] = bomb->GetGUID();
        }
        BombCount = 0;
    }

    bool HatchAllEggs(uint32 action) //1: reset, 2: isHatching all
    {
        std::list<Creature*> templist;
        float x, y, z;
        m_creature->GetPosition(x, y, z);

        {
            CellPair pair(MaNGOS::ComputeCellPair(x, y));
            Cell cell(pair);
            cell.SetNoCreate();

            AllCreaturesOfEntryInRangeCheck check(m_creature, MOB_EGG, 100);
            MaNGOS::CreatureListSearcher<AllCreaturesOfEntryInRangeCheck> searcher(templist, check);

            TypeContainerVisitor<MaNGOS::CreatureListSearcher<AllCreaturesOfEntryInRangeCheck>, GridTypeMapContainer> cSearcher(searcher);

           // CellLock<GridReadGuard> cell_lock(cell, pair);
            cell.Visit(pair, cSearcher, *(m_creature->GetMap()),*m_creature,100);
        }

        //error_log("Eggs %d at middle", templist.size());
        if (!templist.size())
            return false;

        for (std::list<Creature*>::iterator i = templist.begin(); i != templist.end(); ++i)
        {
            if (action == 1)
               (*i)->SetDisplayId(10056);
            else if (action == 2 &&(*i)->GetDisplayId() != 11686)
               (*i)->CastSpell(*i, SPELL_HATCH_EGG, false);
        }
        return true;
    }

    void Boom()
    {
        std::list<Creature*> templist;
        float x, y, z;
        m_creature->GetPosition(x, y, z);

        {
            CellPair pair(MaNGOS::ComputeCellPair(x, y));
            Cell cell(pair);
            cell.SetNoCreate();

            AllCreaturesOfEntryInRangeCheck check(m_creature, MOB_FIRE_BOMB, 100);
            MaNGOS::CreatureListSearcher<AllCreaturesOfEntryInRangeCheck> searcher(templist, check);

            TypeContainerVisitor<MaNGOS::CreatureListSearcher<AllCreaturesOfEntryInRangeCheck>, GridTypeMapContainer> cSearcher(searcher);

            //CellLock<GridReadGuard> cell_lock(cell, pair);
            cell.Visit(pair, cSearcher, *(m_creature->GetMap()),*m_creature,100);
        }
        for (std::list<Creature*>::iterator i = templist.begin(); i != templist.end(); ++i)
        {
           (*i)->CastSpell(*i, SPELL_FIRE_BOMB_DAMAGE, true);
           (*i)->RemoveAllAuras();
        }
    }

    void HandleBombSequence()
    {
        if (BombCount < 40)
        {
            if (Unit *FireBomb = m_creature->GetMap()->GetUnit(FireBombGUIDs[BombCount]))
            {
                FireBomb->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DoCast(FireBomb, SPELL_FIRE_BOMB_THROW, true);
                FireBomb->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            }
            BombCount++;
            if (BombCount == 40)
            {
                BombSequenceTimer = 5000;
            }else BombSequenceTimer = 100;
        }
        else
        {
            Boom();
            isBombing = false;
            BombTimer = 20000+rand()%20000;
            m_creature->RemoveAurasDueToSpell(SPELL_FIRE_BOMB_CHANNEL);
            if (EnrageTimer <= 10000)
                EnrageTimer = 0;
            else
                EnrageTimer -= 10000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (isFlameBreathing)
        {
            if (!m_creature->IsNonMeleeSpellCasted(false))
            {
                isFlameBreathing = false;
            }else return;
        }

        if (isBombing)
        {
            if(BombSequenceTimer < diff)
            {
                HandleBombSequence();
            }else BombSequenceTimer -= diff;
            return;
        }

        if (!m_creature->SelectHostileTarget() && !m_creature->getVictim())
            return;

        //enrage if under 25% hp before 5 min.
        if (!enraged && m_creature->GetHealth() * 4 < m_creature->GetMaxHealth())
            EnrageTimer = 0;

        if (EnrageTimer < diff)
        {
            if (!enraged)
            {
                m_creature->CastSpell(m_creature, SPELL_ENRAGE, true);
                enraged = true;
                EnrageTimer = 300000;
            }
            else
            {
                DoScriptText(SAY_BERSERK, m_creature);

                m_creature->CastSpell(m_creature, SPELL_BERSERK, true);
                EnrageTimer = 300000;
            }
        }else EnrageTimer -= diff;

        if (BombTimer < diff)
        {
            DoScriptText(SAY_FIRE_BOMBS, m_creature);

            m_creature->AttackStop();
            m_creature->GetMotionMaster()->Clear();
            m_creature->Relocate(JanalainPos[0][0],JanalainPos[0][1],JanalainPos[0][2],0);
            m_creature->GetMotionMaster()->MovePoint(0, JanalainPos[0][0], JanalainPos[0][1], JanalainPos[0][2]);
            m_creature->StopMoving();
            m_creature->CastSpell(m_creature, SPELL_FIRE_BOMB_CHANNEL, false);
            DoTeleportPlayer(m_creature, JanalainPos[0][0], JanalainPos[0][1],JanalainPos[0][2], 0);
            m_creature->CastSpell(m_creature, SPELL_TELE_TO_CENTER, true);

            FireWall();
            SpawnBombs();
            isBombing = true;
            BombSequenceTimer = 100;

            //Teleport every Player into the middle
            Map *map = m_creature->GetMap();
            if (!map->IsDungeon()) return;
            Map::PlayerList const &PlayerList = map->GetPlayers();
            if (!PlayerList.isEmpty())
                return;
            for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
				if (i->getSource()->isAlive())
					DoTeleportPlayer(i->getSource(), JanalainPos[0][0]-5+rand()%10, JanalainPos[0][1]-5+rand()%10, JanalainPos[0][2], 0);
            }
/*Test*/    m_creature->CastSpell(JanalainPos[0][0]-5+rand()%10, JanalainPos[0][1]-5+rand()%10, JanalainPos[0][2], SPELL_SUMMON_PLAYERS, true); // core bug, spell does not work if too far
            return;
        }else BombTimer -= diff;

        if (!noeggs)
        {
            if (100 * m_creature->GetHealth() < 35 * m_creature->GetMaxHealth())
            {
                m_creature->AttackStop();
                m_creature->GetMotionMaster()->Clear();
                m_creature->Relocate(JanalainPos[0][0],JanalainPos[0][1],JanalainPos[0][2],0);
                m_creature->GetMotionMaster()->MovePoint(0, JanalainPos[0][0], JanalainPos[0][1], JanalainPos[0][2]);
                m_creature->StopMoving();
                m_creature->CastSpell(m_creature, SPELL_HATCH_ALL, false);
                HatchAllEggs(2);
                noeggs = true;
            }
            else if (HatcherTimer < diff)
            {
                if (HatchAllEggs(0))
                {
                    DoScriptText(SAY_SUMMON_HATCHER, m_creature);

                    m_creature->SummonCreature(MOB_AMANI_HATCHER,hatcherway[0][0][0],hatcherway[0][0][1],hatcherway[0][0][2],0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,10000);
                    m_creature->SummonCreature(MOB_AMANI_HATCHER,hatcherway[1][0][0],hatcherway[1][0][1],hatcherway[1][0][2],0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,10000);
                    HatcherTimer = 90000;
                }
                else
                    noeggs = true;
            }else HatcherTimer -= diff;
        }

        if (ResetTimer < diff)
        {
            float x, y, z;
            m_creature->GetPosition(x, y, z);
            if (x < -70 || x > 0 || y > 1176 || y < 1121 || z < 18)
                EnterEvadeMode();
            ResetTimer = 5000;
        }else ResetTimer -= diff;

        DoMeleeAttackIfReady();

        if (FireBreathTimer < diff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
            {
                m_creature->AttackStop();
                m_creature->GetMotionMaster()->Clear();
                m_creature->CastSpell(target, SPELL_FLAME_BREATH, false);
                m_creature->StopMoving();
                isFlameBreathing = true;
            }
            FireBreathTimer = 8000;
        }else FireBreathTimer -= diff;
    }
};

CreatureAI* GetAI_boss_janalaiAI(Creature *_Creature)
{
    return new boss_janalaiAI(_Creature);
}

struct MANGOS_DLL_DECL mob_janalai_firebombAI : public ScriptedAI
{
    mob_janalai_firebombAI(Creature *c) : ScriptedAI(c){Reset();}

    void Reset() {}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_FIRE_BOMB_THROW)
            m_creature->CastSpell(m_creature, SPELL_FIRE_BOMB_DUMMY, true);
    }

    void Aggro(Unit* who) {}

    void AttackStart(Unit* who) {}

    void MoveInLineOfSight(Unit* who) {}

    void UpdateAI(const uint32 diff) {}
};

CreatureAI* GetAI_mob_janalai_firebombAI(Creature *_Creature)
{
    return new mob_janalai_firebombAI(_Creature);
}

struct MANGOS_DLL_DECL mob_janalai_hatcherAI : public ScriptedAI
{
    mob_janalai_hatcherAI(Creature *c) : ScriptedAI(c)
    {
        pInstance =((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    ScriptedInstance *pInstance;

    uint32 waypoint;
    uint32 HatchNum;
    uint32 WaitTimer;

    bool side;
    bool hasChangedSide;
    bool isHatching;

    void Reset()
    {
        side =(m_creature->GetPositionY() < 1150);
        waypoint = 0;
        isHatching = false;
        hasChangedSide = false;
        WaitTimer = 1;
        HatchNum = 0;
    }

    bool HatchEggs(uint32 num)
    {
        std::list<Creature*> templist;
        float x, y, z;
        m_creature->GetPosition(x, y, z);

        {
            CellPair pair(MaNGOS::ComputeCellPair(x, y));
            Cell cell(pair);
            cell.SetNoCreate();

            AllCreaturesOfEntryInRangeCheck check(m_creature, 23817, 50);
            MaNGOS::CreatureListSearcher<AllCreaturesOfEntryInRangeCheck> searcher(templist, check);

            TypeContainerVisitor<MaNGOS::CreatureListSearcher<AllCreaturesOfEntryInRangeCheck>, GridTypeMapContainer> cSearcher(searcher);

            //CellLock<GridReadGuard> cell_lock(cell, pair);
            cell.Visit(pair, cSearcher, *(m_creature->GetMap()),*m_creature,100);
        }

        //error_log("Eggs %d at %d", templist.size(), side);

        for(std::list<Creature*>::iterator i = templist.begin(); i != templist.end() && num > 0; ++i)
        {
            if((*i)->GetDisplayId() != 11686)
            {
               (*i)->CastSpell(*i, SPELL_HATCH_EGG, false);
                num--;
            }
        }

        if(num)
            return false;   // no more templist
        else
            return true;
    }

    void Aggro(Unit* who) {}
    void AttackStart(Unit*) {}
    void MoveInLineOfSight(Unit*) {}
    void MovementInform(uint32, uint32)
    {
        if(waypoint == 5)
        {
            isHatching = true;
            HatchNum = 1;
            WaitTimer = 5000;
        }
        else
            WaitTimer = 1;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!pInstance || !(pInstance->GetData(TYPE_JANALAI) == IN_PROGRESS))
        {
            m_creature->SetVisibility(VISIBILITY_OFF);
            m_creature->SetDeathState(JUST_DIED);
            return;
        }

        if(!isHatching)
        {
            if(WaitTimer)
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(0,hatcherway[side][waypoint][0],hatcherway[side][waypoint][1],hatcherway[side][waypoint][2]);
                waypoint++;
                WaitTimer = 0;
            }
        }
        else
        {
            if(WaitTimer < diff)
            {
                if(HatchEggs(HatchNum))
                {
                    HatchNum++;
                    WaitTimer = 10000;
                }
                else if(!hasChangedSide)
                {
                    side = side ? 0 : 1;
                    isHatching = false;
                    waypoint = 3;
                    WaitTimer = 1;
                    hasChangedSide = true;
                }
                else
                {
                    m_creature->SetVisibility(VISIBILITY_OFF);
                    m_creature->SetDeathState(JUST_DIED);
                }
            }else WaitTimer -= diff;
        }
    }
};

CreatureAI* GetAI_mob_janalai_hatcherAI(Creature *_Creature)
{
    return new mob_janalai_hatcherAI(_Creature);
}

struct MANGOS_DLL_DECL mob_hatchlingAI : public ScriptedAI
{
    mob_hatchlingAI(Creature *c) : ScriptedAI(c)
    {
        pInstance =((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    ScriptedInstance *pInstance;
    uint32 BuffetTimer;

    void Reset()
    {
        BuffetTimer = 7000;
        if(m_creature->GetPositionY() > 1150)
            m_creature->GetMotionMaster()->MovePoint(0, hatcherway[0][3][0]+rand()%4-2,1150+rand()%4-2,hatcherway[0][3][2]);
        else
            m_creature->GetMotionMaster()->MovePoint(0, hatcherway[1][3][0]+rand()%4-2,1150+rand()%4-2,hatcherway[1][3][2]);

		m_creature->CanFly();
    }

    void Aggro(Unit *who) { m_creature->SetInCombatWithZone(); }

    void UpdateAI(const uint32 diff)
    {
        if(!pInstance || !(pInstance->GetData(TYPE_JANALAI) == IN_PROGRESS))
        {
            m_creature->SetVisibility(VISIBILITY_OFF);
            m_creature->SetDeathState(JUST_DIED);
            return;
        }

        if(!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if(BuffetTimer < diff)
        {
            m_creature->CastSpell(m_creature->getVictim(), SPELL_FLAMEBUFFET, false);
            BuffetTimer = 5000;
        }else BuffetTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hatchlingAI(Creature *_Creature)
{
    return new mob_hatchlingAI(_Creature);
}

struct MANGOS_DLL_DECL mob_eggAI : public ScriptedAI
{
    mob_eggAI(Creature *c) : ScriptedAI(c){}
    void Reset() {}
    void Aggro(Unit* who) {}
    void AttackStart(Unit* who) {}
    void MoveInLineOfSight(Unit* who) {}
    void UpdateAI(const uint32 diff) {}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_HATCH_EGG)
        {
            DoSpawnCreature(MOB_HATCHLING, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
            m_creature->SetDisplayId(11686);
        }
    }
};

CreatureAI* GetAI_mob_eggAI(Creature *_Creature)
{
    return new mob_eggAI(_Creature);
}

void AddSC_boss_janalai()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_janalai";
    newscript->GetAI = &GetAI_boss_janalaiAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_janalai_firebomb";
    newscript->GetAI = &GetAI_mob_janalai_firebombAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_janalai_hatcher";
    newscript->GetAI = &GetAI_mob_janalai_hatcherAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_janalai_hatchling";
    newscript->GetAI = &GetAI_mob_hatchlingAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_janalai_egg";
    newscript->GetAI = &GetAI_mob_eggAI;
    newscript->RegisterSelf();
}
