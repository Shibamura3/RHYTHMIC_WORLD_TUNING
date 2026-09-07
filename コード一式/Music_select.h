/*
    選曲画面の制御：Music_select.h

    2026/05/27	hibiki sakuma
*/
#ifndef MUSIC_SELECT_H
#define MUSIC_SELECT_H

#include "enemy.h"

#include <string>

enum class GameDifficulty
{
    Easy,
    Normal,
    Hard,
    Max, // 難易度の繰り返し表示の最大数

    Edit, //エディットモードの判定用
};

void Music_Select_Initialize();
void Music_Select_Finalize();
void Music_Select_Update(double elapsed_time);
void Music_Select_Draw();

int Music_Select_GetId();
int Music_Select_GetMaxScore();
int Music_Select_GetBgmAudioId();
EnemyType Music_Select_GetEnemyId();
GameDifficulty Music_Select_GetDifficulty();
const std::string& Music_Select_GetSongId();

#endif // !MUSIC_SELECT_H
