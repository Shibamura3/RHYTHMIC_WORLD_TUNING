/*
	コンボの制御：combo.h

	2026/06/19	hibiki sakuma
*/
#include "combo.h"
#include "direct3d.h"
#include "texture.h"
#include "sprite.h"
#include "resource_manager.h"
#include <algorithm>
#include <DirectXMath.h>
using namespace DirectX;
// 定数宣言
static constexpr int NUM = 10;

// 変数宣言
static int g_Combo = 0;
static int g_ViewCombo = 0;
static int g_MaxDigit = 3;
static bool g_IsPerfectChain = true; // ALLPerfect判定
// 画像用変数宣言
static float g_ComboNum_PosX = -1;
static float g_ComboNum_PosY = -1;
static float g_ComboNum_SizeX = -1;
static float g_ComboNum_SizeY = -1;
static float g_ComboText_SizeX = -1;
static float g_ComboText_SizeY = -1;

static constexpr int pix_x = 125; // 切り取りサイズ
static constexpr int pix_y = 170;
static int g_ComboNum_TexId = -1;
static int g_ComboText_TexId = -1;

static void DrawNumber(float x, float y, int number, XMFLOAT4 color);

void Combo_Initialize(int digit) {
    float w = (float)Direct3D_GetBackBufferWidth();
    float h = (float)Direct3D_GetBackBufferHeight();
    g_Combo = 0;
    g_ViewCombo = 0;
    g_MaxDigit = digit;
    g_IsPerfectChain = true; // 最初はAP継続扱い

    g_ComboNum_PosX = w * 0.3f;
    g_ComboNum_PosY = h * 0.05f;
    g_ComboNum_SizeX = w * 0.07f;
    g_ComboNum_SizeY = h * 0.15f;
    g_ComboText_SizeX = w * 0.2f;
    g_ComboText_SizeY = h * 0.25f;

    g_ComboNum_TexId = Resouce_Manager_GetTexId(Combo_Num);
    g_ComboText_TexId = Resouce_Manager_GetTexId(Combo_Text);
} 

void Combo_Update(double elapsed_time) {
    g_ViewCombo = std::min(g_ViewCombo + 1, g_Combo);
}

void Combo_Draw() {
    bool isStarted = false;

    XMFLOAT4 color;

    if (g_IsPerfectChain) {
        color = { 1.0f, 1.0f, 1.0f, 1.0f };
    } else {
        color = { 0.7f, 0.7f, 0.7f, 1.0f };
    }

    for (int i = 0; i < g_MaxDigit; i++) {
        int index = g_MaxDigit - 1 - i;

        int digitVal = (g_ViewCombo / (int)pow(10, index)) % 10;

        if (digitVal != 0 || isStarted || index == 0) {
            isStarted = true;

            float x = g_ComboNum_PosX + g_ComboNum_SizeX * (g_MaxDigit - 1 - index);

            DrawNumber(x, g_ComboNum_PosY, digitVal, color);
        }
    }

    // 数字の右に文字を表示
    float rightX = g_ComboNum_PosX + g_ComboNum_SizeX * g_MaxDigit;
    Sprite_Draw(
        g_ComboText_TexId,
        rightX, g_ComboNum_PosY,
        g_ComboText_SizeX, g_ComboText_SizeY,
        color
    );
}

void Combo_Add() {
    g_Combo++;
}

void Combo_Reset() {
    g_Combo = 0;
    g_ViewCombo = 0;
}

void Combo_SetPerfectChain(bool flag){
    g_IsPerfectChain = flag;
}

void DrawNumber(float x, float y, int number, XMFLOAT4 color) {
    Sprite_Draw(
        g_ComboNum_TexId,
        x, y,
        g_ComboNum_SizeX, g_ComboNum_SizeY,
        pix_x * number, 0,
        pix_x, pix_y,
        color
    );
}
