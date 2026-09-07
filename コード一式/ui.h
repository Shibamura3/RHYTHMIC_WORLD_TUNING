/*
    UIの制御：ui.h

    2026/05/22	hibiki sakuma
*/

#ifndef UI_H
#define UI_H

#include "score_type.h"

void UI_Initialize();
void UI_Finalize();
void UI_Update(double elapsed_time);
void UI_Draw();

// 判定情報のセット
enum Judge_Type {
    Judge_Perfect,
    Judge_Great,
    Judge_Miss,

    Jidge_MAX
};

void UI_SetJudge(int type);

// 外部から情報をセットする
void UI_SetPlayerHpRate(float rate);
void UI_SetPlayerFeverRate(float rate);
void UI_SetEnemyHpRate(float rate);
void UI_SetScore(int score);
void UI_SetKillCount(int count);
void UI_SetRank(Rank_Type rank);

void UI_ShowSkillPopup(const wchar_t* skillName);

#endif // !UI_H
