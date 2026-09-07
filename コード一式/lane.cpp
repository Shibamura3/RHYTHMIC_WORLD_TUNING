/*
	レーンの描画：lane.h

	2026/07/10	hibiki sakuma
*/

#include "lane.h"
#include "resource_manager.h"
#include "sprite.h"
#include "direct3d.h"
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

// 定数宣言
static constexpr float JUDEG_ANIM_OFFSETY = 30.0f; // 判定アニメーション上方向
static constexpr float JUDEG_TIMER = 0.5f; // 判定画像の表示時間
// 画像用変数
static float w = -1; // 画面サイズ用
static float h = -1; // 画面サイズ用
static float g_Lane_PosX = -1;
static float g_Lane1_PosY = -1;
static float g_Lane2_PosY = -1;
static float g_Lane_SizeX = -1;
static float g_Lane_SizeY = -1;

static float g_Judeg_PosX = -1;
static float g_Judeg_PosY = -1;
static float g_Judeg_SizeX = -1;
static float g_Judeg_SizeY = -1;


void Lane_Initialize(){
    w = (float)Direct3D_GetBackBufferWidth();
    h = (float)Direct3D_GetBackBufferHeight();

    // レーン
    g_Lane_PosX = w * 0.2f;
    g_Lane1_PosY = h * 0.4f;
    g_Lane2_PosY = h * 0.6f;
    g_Lane_SizeX = w * 0.8f;
    g_Lane_SizeY = h * 0.1f;

}


void Lane_Draw(){
    // レーン
    float lane_alpha = 0.5f;
    XMFLOAT4 lane_color = { 1,1,1,lane_alpha };
    Sprite_Draw(Resouce_Manager_GetTexId(Lane),
        g_Lane_PosX, g_Lane1_PosY - g_Lane_SizeY * 0.5f,
        g_Lane_SizeX, g_Lane_SizeY,
        lane_color);
    Sprite_Draw(Resouce_Manager_GetTexId(Lane),
        g_Lane_PosX, g_Lane2_PosY - g_Lane_SizeY * 0.5f,
        g_Lane_SizeX, g_Lane_SizeY,
        lane_color);
}
