/*
    プレイヤーキャラクター選択画面の制御：Player_select.h

    2026/07/15	hibiki sakuma
*/
#ifndef PLAYER_SELECT_H
#define PLAYER_SELECT_H

#include "player.h"

void Player_Select_Initialize();
void Player_Select_Finalize();
void Player_Select_Update(double elapsed_time);
void Player_Select_Draw();

int Player_Select_GetId();
PlayerCharacter Player_Select_GetPlayerId();

#endif // !PLAYER_SELECT_H