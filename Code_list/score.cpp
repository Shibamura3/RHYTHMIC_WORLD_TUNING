/*
	スコアの制御：score.cpp

	2025/07/09	hibiki sakuma
*/

#include "score.h"
#include "texture.h"
#include "sprite.h"
#include <algorithm>
#define pix_x 50
#define pix_y 78

//変数宣言
static unsigned int g_Score = 0; // 内部スコア
static unsigned int g_ViewScore = 0; // 表示スコア
static unsigned int g_CounterStop = 1;
static int g_digit = 1; // 0桁はないため1で初期化
static float g_Score_x = 0.0f, g_Score_y = 0.0f;
static int g_ScoreTexture = -1;

static void drawNumber(float x, float y, int number); // ローカル関数 一文字の表示

void Score_Initialize(float x, float y, int digit){
	g_Score = 0;
	g_ViewScore = 0;
	g_digit = digit;
	g_Score_x = x;
	g_Score_y = y;

	//スコアカンストの得点
	for (int i = 0;i < g_digit;i++) {
		g_CounterStop *= 10; // 10の桁数分累乗
	}
	g_CounterStop--; // 9999といった表示になる

	g_ScoreTexture = Texture_Load(L"resuce/img/0123456789.png");

}

void Score_Finalize(){

}

void Score_Update(double elapsed_time){
	g_ViewScore = std::min(g_ViewScore+=1, g_Score);
}

void Score_Draw(){
	unsigned int temp_score = std::min(g_ViewScore , g_CounterStop); // min 比較して小さい方を採用 
	for (int i = 0;i < g_digit;i++) {
		int num = temp_score % 10; // 表示したい1桁を抽出
		float x = g_Score_x + 100 * (g_digit - 1 - i);
		drawNumber(x, g_Score_y, num);
		temp_score /= 10;
	}
}

unsigned int Score_GetScore(){
	return g_Score;
}

void Score_AddScore(int Score){
	g_ViewScore = g_Score;
	g_Score += Score;
}

void Score_Reset(){
	g_Score = 0;
}

void drawNumber(float x, float y, int number){
	Sprite_Draw(g_ScoreTexture, x , y,100.0f,100.0f, pix_x * number , 0, pix_x, pix_y);
}
