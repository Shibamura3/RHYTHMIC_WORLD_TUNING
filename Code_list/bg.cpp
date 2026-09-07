/*
	背景の制御：bg.h

	2026/05/20	hibiki sakuma
*/

#include "bg.h"
#include "sprite.h"
#include "texture.h"
#include "resource_manager.h"
#include "direct3d.h"

static constexpr int LAYER_MAX = 3;

struct BgLayer {
	int texId;

	float scrollSpeed; // px/sec
	float offsetX;

	float y;
	float height;
};
enum BG_ID{
	BG_SKY,
	BG_BUILDING,
	BG_GROUND,

	BG_MAX,
};
static BgLayer g_Layers[LAYER_MAX]; // 0:空 1:建物 2:地面


// 画像読み込み用
static int g_BG_ID0 = -1; // 一番下の背景
// 変数宣言
static double g_AccumulatedTime = 0.0; // 経過時間
static float g_scroll_speed = 100.0f; // 秒間何ピクセル移動する
static float g_offsetX_01 = 0.0f;
static float g_offsetX_02 = 0.0f;

void Bg_Initialize(){

	float h = (float)Direct3D_GetBackBufferHeight();

	// 空（一番遅い）
	g_Layers[BG_SKY].texId = Resouce_Manager_GetTexId(BG_sky);
	g_Layers[BG_SKY].scrollSpeed = 20.0f;
	g_Layers[BG_SKY].offsetX = 0.0f;
	g_Layers[BG_SKY].y = 0.0f;
	g_Layers[BG_SKY].height = h * 0.3f;

	// 建物（中間）
	g_Layers[BG_BUILDING].texId = Resouce_Manager_GetTexId(BG_building);
	g_Layers[BG_BUILDING].scrollSpeed = 100.0f;
	g_Layers[BG_BUILDING].offsetX = 0.0f;
	g_Layers[BG_BUILDING].y = h * 0.15f;
	g_Layers[BG_BUILDING].height = h * 0.7f;

	// 地面（速い）
	g_Layers[BG_GROUND].texId = Resouce_Manager_GetTexId(BG_ground);
	g_Layers[BG_GROUND].scrollSpeed = 300.0f;
	g_Layers[BG_GROUND].offsetX = 0.0f;
	g_Layers[BG_GROUND].y = h * 0.75f;
	g_Layers[BG_GROUND].height = h * 0.25f;

	g_BG_ID0 = Resouce_Manager_GetTexId(Title_Back);
}

void Bg_Finalize(){

}

void Bg_Update(double elapsed_time){
	for (int i = 0; i < LAYER_MAX; i++){
		g_Layers[i].offsetX += g_Layers[i].scrollSpeed * (float)elapsed_time;

		// ループ処理（超重要）
		float base_X = (float)Direct3D_GetBackBufferWidth();

		while (g_Layers[i].offsetX >= base_X){
			g_Layers[i].offsetX -= base_X;
		}
	}
}

void Bg_Draw() {
	float base_X = (float)Direct3D_GetBackBufferWidth();
	float base_Y = (float)Direct3D_GetBackBufferHeight();
	Sprite_Draw(g_BG_ID0,
		0.0f, 0.0f,
		base_X, base_Y);

	// 背面（空→建物→地面）
	for (int i = 0; i < LAYER_MAX; i++) {
		BgLayer& layer = g_Layers[i];

		// 3枚描画する
		for (int j = -1; j < 2; j++){
			float x = -layer.offsetX + base_X * j;
			Sprite_Draw(
				layer.texId,
				x, layer.y,
				base_X, layer.height
			);
		}
	}

}
