/************************************************
*スプライト表示　sprite_animu.h
*
*
*************************************************/


#include "sprite_animu.h"
#include "sprite.h"
#include "texture.h"
#include <DirectXMath.h>

using namespace DirectX;//ヘッダーには描かない

struct AnimPatternDate {
	int m_PatternId = -1;//パターンID
	int m_PatternMax = 0;//パターン数
	int m_HPatternMax = 0;//横のパターン最大数 
	XMUINT2 m_StartPosition = { 0,0 };//アニメーションのスタート座標
	XMUINT2 m_PatternSize = { 0,0 };//1パターンの幅,高さ
	double m_seconds_per_pattern = 0.1;
	bool m_IsLooped = true;//ループするか
};

struct AnimPlayDate {
	int m_PatternId = -1;//パターンID
	int m_PatternNum = 0;//現在のパターン now
	double m_accumulated_time= 0.0;//累積時間
	bool m_IsStopped = false;
};

static constexpr int ANIM_PATTERN_MAX = 128;
static AnimPatternDate g_AnimPattern[ANIM_PATTERN_MAX];
static constexpr int ANIM_PLAY_MAX = 256; //動く画像を最大何個配置するか
static AnimPlayDate g_AnimPlay[ANIM_PLAY_MAX];


void SpriteAnim_Initialize(){

	//アニメーションパターン管理情報の初期化（すべて使用していない）状況にする
	for (AnimPatternDate& date : g_AnimPattern) {
		date.m_PatternId = -1; // IDが -1 なら使用されていない
	}
	
	for (AnimPlayDate& date : g_AnimPlay) {
		date.m_PatternId = -1; // IDが -1 なら使用されていない
		date.m_IsStopped = false;
	}

}

void SpriteAnim_Finalize(){

}

void SpriteAnim_Update(double elapsed_time) {
	for (int i = 0;i < ANIM_PLAY_MAX;i++) {
		if (g_AnimPlay[i].m_PatternId < 0) continue;

		AnimPatternDate* pAnimPatternDate = &g_AnimPattern[g_AnimPlay[i].m_PatternId];

		if (g_AnimPlay[i].m_accumulated_time >= pAnimPatternDate->m_seconds_per_pattern) {		
			g_AnimPlay[i].m_PatternNum++;

			if (g_AnimPlay[i].m_PatternNum >= pAnimPatternDate->m_PatternMax) {
				if (pAnimPatternDate->m_IsLooped) {
					g_AnimPlay[i].m_PatternNum = 0;
				}
				else {
					g_AnimPlay[i].m_PatternNum = pAnimPatternDate->m_PatternMax - 1;
					g_AnimPlay[i].m_IsStopped = true; // アニメーションが止まったことを確認
				}
			}
			g_AnimPlay[i].m_accumulated_time -= 0.1;

		}
		g_AnimPlay[i].m_accumulated_time += elapsed_time;
	}
}
void SpriteAnim_Draw(int playid,float dx, float dy, float dw, float dh) {
	int anim_pattern_id = g_AnimPlay[playid].m_PatternId;
	AnimPatternDate* pAnimPatternDate = &g_AnimPattern[anim_pattern_id];
	
	Sprite_Draw(pAnimPatternDate->m_PatternId,
		dx , dy , dw, dh,
		pAnimPatternDate->m_StartPosition.x
		+ pAnimPatternDate->m_PatternSize.x
		* g_AnimPlay[playid].m_PatternNum,
		pAnimPatternDate->m_StartPosition.y,
		pAnimPatternDate->m_PatternSize.x,
		pAnimPatternDate->m_PatternSize.y);

}

int Spriteanim_RegisterPattern(int texId, int patternMax, int h_patternMax, double seconds_par_pattern, const DirectX::XMUINT2& pattern_size, const DirectX::XMUINT2& start_position, bool is_looped) {
	for (int i = 0;i < ANIM_PATTERN_MAX;i++) {
		// 空いている場所を探す
		if (g_AnimPattern[i].m_PatternId >= 0)continue;
		g_AnimPattern[i].m_PatternId = texId;
		g_AnimPattern[i].m_seconds_per_pattern = seconds_par_pattern;
		g_AnimPattern[i].m_PatternMax = patternMax;
		g_AnimPattern[i].m_HPatternMax = h_patternMax;
		g_AnimPattern[i].m_PatternSize = pattern_size;
		g_AnimPattern[i].m_StartPosition = start_position;
		g_AnimPattern[i].m_IsLooped = is_looped;
		return i; //管理番号　成功時数字を返す
	}
	return -1; //管理番号　-1　で足りなかった時
}

int SpriteAnim_CreatePlayer(int anim_pattern_id){
	for (int i = 0;i < ANIM_PLAY_MAX;i++) {
		if (g_AnimPlay[i].m_PatternId >= 0)continue;

		g_AnimPlay[i].m_PatternId = anim_pattern_id;
		g_AnimPlay[i].m_accumulated_time = 0.0;
		g_AnimPlay[i].m_PatternNum = 0;

		g_AnimPlay[i].m_IsStopped = false;

		return i;
	}

	return -1;
}

void SpriteAnim_DestroyPlayer(int index){
	g_AnimPlay[index].m_PatternId = -1;
}

bool SpriteAnim_IsStopped(int index){

	return g_AnimPlay[index].m_IsStopped;
}
