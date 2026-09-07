/*
    キャラクター選択画面の制御：Character_select.h

    2026/08/27	hibiki sakuma
*/
#ifndef CHARACTER_SELECT_H
#define CHARACTER_SELECT_H

#include "player.h"

void Character_Select_Initialize();
void Character_Select_Finalize();
void Character_Select_Update(double elapsed_time);
void Character_Select_Draw();

PlayerCharacter Character_Select_GetCharacter();

#endif // !CHARACTER_SELECT_H
