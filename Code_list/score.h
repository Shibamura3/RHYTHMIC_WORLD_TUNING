/*
	スコアの制御：score.h

	2025/07/09	hibiki sakuma
*/

#ifndef SCORE_H
#define SCORE_H

void Score_Initialize(float x,float y/*表示座標*/, int digit/*桁*/); // ゼロ埋め、左寄せなどの初期設定
void Score_Finalize(); // 演出で音を鳴らすなどした場合
void Score_Update(double elapsed_time);
void Score_Draw();

unsigned int Score_GetScore(); // 大きな数字を使用するため

void Score_AddScore(int Score);

void Score_Reset(); // 場面移動時などの場合の初期化用

#endif // !SCORE_H
