/*
    ノーツの制御：ui.cpp

    2026/05/22	hibiki sakuma
*/
#include "ui.h"
#include "combo.h"
#include "lane.h"
#include "resource_manager.h"
#include "text_manager.h"
#include "sprite.h"
#include "texture.h"
#include "direct3d.h"
#include <vector>
#include <DirectXMath.h>
#include <string>
using namespace DirectX;

// 定数宣言
static constexpr float JUDEG_ANIM_OFFSETY = 30.0f; // 判定アニメーション上方向
static constexpr float JUDEG_TIMER = 0.5f; // 判定画像の表示時間
static constexpr float SKILL_POPUP_TIME = 3.0f; // ポップアップ表示時間
static constexpr float SKILL_POPUP_SWITCH_TIME = 1.5f; // ポップアップ切り替え時間
static constexpr float SKILL_POPUP_ENTER_TIMER = 8.0f;
static constexpr float SKILL_POPUP_EXIT_TIMER = 5.0f;

// 画面サイズ用変数
static float w = -1; // 画面サイズ用
static float h = -1; // 画面サイズ用

// コンボ関連
static float g_Judeg_PosX = -1;
static float g_Judeg_PosY = -1;
static float g_Judeg_SizeX = -1;
static float g_Judeg_SizeY = -1;

// ゲージ関連
static float g_Gage_X = -1;
static float g_Gage_Y = -1;
static float g_PlayerHP_OffSetX = -1;
static float g_PlayerHP_OffSetY = -1;
static float g_PlayerFEVER_OffSetX = -1;
static float g_PlayerFEVER_OffSetY = -1;
static float g_EnemyHP_OffSetX = -1;
static float g_EnemyHP_OffSetY = -1;

static float g_PlayerHpRate = -1;
static float g_PlayerFeverRate = -1;
static float g_EnemyHpRate = -1;

// 判定用変数
static int g_JudgeType = -1;   // -1=なし 0=Perfect 1=Great 2=Miss
static float g_JudgeTimer = 0.0f;

// ゲーム画面表示
static int g_Score = 0;
static Rank_Type g_Rank = Rank_D;
static int g_KillCount = 0;

static int g_UI_TextBox_Id = -1; // UI画面で使用するテキストボックス
static int g_RankTex_Id[Rank_MAX];

static std::wstring g_SkillPopupText = L"";
static float g_SkillPopupTimer = 0.0f;

static float g_SkillPopupX = 0.0f; // スライド用
static float g_SkillPopupY = 0.0f; // 表示位置

static float g_popupWidth = 0.0f;
static float g_targetX = 0.0f;
static float g_hideX = 0.0f;

void UI_Initialize() {
    Lane_Initialize(); // レーンの描画
    Combo_Initialize(3); // コンボの初期化→UI

    w = (float)Direct3D_GetBackBufferWidth();
    h = (float)Direct3D_GetBackBufferHeight();

    // 判定
    g_Judeg_PosX  = w * 0.25f;
    g_Judeg_PosY  = h * 0.3f;
    g_Judeg_SizeX = w * 0.15f;
    g_Judeg_SizeY = h * 0.1f;

    // ゲージのサイズ
    g_Gage_X = w * 0.3f;
    g_Gage_Y = h * 0.05f;

    // ゲージがケースに収まるように調整
    g_PlayerHP_OffSetX = w * 0.01f;
    g_PlayerHP_OffSetY = h * 0.0025f;
    g_PlayerFEVER_OffSetX =w * 0.035f;
    g_PlayerFEVER_OffSetY =h * 0.005f;
    g_EnemyHP_OffSetX = w * 0.005f;
    g_EnemyHP_OffSetY = h * 0.005f;

    // 割合の初期化
    g_PlayerHpRate = 1.0f;
    g_PlayerFeverRate = 0.0f;
    g_EnemyHpRate = 1.0f;

    // 表示内容の初期化
    g_Score = 0;
    g_Rank = Rank_D;
    g_KillCount = 0;

    g_SkillPopupText = L"";
    g_SkillPopupTimer = 0.0f;
    g_UI_TextBox_Id = Resouce_Manager_GetTexId(TextBox);
    g_popupWidth = w * 0.25f;
    g_targetX = w * 0.03f;
    g_hideX = -g_popupWidth;
    g_SkillPopupX = g_hideX;
    g_SkillPopupY = h * 0.25f;

    // ランク画像の設定
    g_RankTex_Id[(int)Rank_D] = Resouce_Manager_GetTexId(Rank_IMG_D);
    g_RankTex_Id[(int)Rank_C] = Resouce_Manager_GetTexId(Rank_IMG_C);
    g_RankTex_Id[(int)Rank_B] = Resouce_Manager_GetTexId(Rank_IMG_B);
    g_RankTex_Id[(int)Rank_A] = Resouce_Manager_GetTexId(Rank_IMG_A);
    g_RankTex_Id[(int)Rank_S] = Resouce_Manager_GetTexId(Rank_IMG_S);


}

void UI_Finalize(){

}

void UI_Update(double elapsed_time){

    // 判定関連
    if (g_JudgeTimer > 0.0f)
    {
        g_JudgeTimer -= (float)elapsed_time;

        if (g_JudgeTimer <= 0.0f)
        {
            g_JudgeType = -1;
        }
    }

    // コンボ更新
    Combo_Update(elapsed_time);

    // スキルポップアップ出現
    if (g_SkillPopupTimer > SKILL_POPUP_SWITCH_TIME){
        g_SkillPopupX += (g_targetX - g_SkillPopupX) * SKILL_POPUP_ENTER_TIMER * (float)elapsed_time;
    } else {
        g_SkillPopupX += (g_hideX - g_SkillPopupX) * SKILL_POPUP_EXIT_TIMER * (float)elapsed_time;
    }

    if (g_SkillPopupTimer > 0.0f){
        g_SkillPopupTimer -= (float)elapsed_time;

        if (g_SkillPopupTimer < 0.0f){
            g_SkillPopupTimer = 0.0f;
        }
    }
}

void UI_Draw(){
    // レーンの描画
    Lane_Draw();

    // 判定
    int judge_texId = -1;
    switch (g_JudgeType) { // 画像の設定
    case Judge_Perfect:
        judge_texId = Resouce_Manager_GetTexId(Combo_Perfect);
        break;
    case Judge_Great:
        judge_texId = Resouce_Manager_GetTexId(Combo_Great);
        break;
    case Judge_Miss:
        judge_texId = Resouce_Manager_GetTexId(Combo_Miss);
        break;
    default:
        // エラー
        break;
    }
    if (judge_texId != -1) {
        // アニメーションの追加
        float totalTime = 0.5f;
        float t = g_JudgeTimer / totalTime; // 1 → 0

        float scale = 1.0f + (1.0f - t) * 0.5f;
        float judge_alpha = t;
        float offsetY = (1.0f - t) * JUDEG_ANIM_OFFSETY;

        // サイズ適用
        float drawSizeX = g_Judeg_SizeX * scale;
        float drawSizeY = g_Judeg_SizeY * scale;

        XMFLOAT4 judge_color = { 1.0f ,1.0f ,1.0f,judge_alpha };

        Sprite_Draw(
            judge_texId,
            g_Judeg_PosX - drawSizeX * 0.5f, g_Judeg_PosY - drawSizeY * 0.5f - offsetY,
            drawSizeX, drawSizeY,
            judge_color
        );

    }

    // コンボ描画
    Combo_Draw();

    // 各種ゲージの描画
    float playerHpInnerWidth = g_Gage_X - g_PlayerHP_OffSetX * 2.0f;
    float playerHpWidth = playerHpInnerWidth * g_PlayerHpRate;
    int tw = Texture_Width(Resouce_Manager_GetTexId(Player_HP_Point));
    int th = Texture_Height(Resouce_Manager_GetTexId(Player_HP_Point));
    int uvWidth = static_cast<int>(tw * g_PlayerHpRate);
    Sprite_Draw( // PlayerHP
        Resouce_Manager_GetTexId(Player_HP_Case),
        0.0f, h * 0.1f,
       g_Gage_X, g_Gage_Y
    );
    Sprite_Draw(
        Resouce_Manager_GetTexId(Player_HP_Point),
        0.0f + g_PlayerHP_OffSetX, h * 0.1f + g_PlayerHP_OffSetY,
        playerHpWidth, g_Gage_Y - g_PlayerHP_OffSetY * 2.0f,
        0,0,
        uvWidth,th
    );

    float playerFeverInnerWidth = g_Gage_X - g_PlayerFEVER_OffSetX * 2.0f;
    float playerFeverWidth = playerFeverInnerWidth * g_PlayerFeverRate;
    tw = Texture_Width(Resouce_Manager_GetTexId(Player_FEVER_Point));
    th = Texture_Height(Resouce_Manager_GetTexId(Player_FEVER_Point));
    uvWidth = static_cast<int>(tw * g_PlayerFeverRate);
    Sprite_Draw( // PlayerFEVER
        Resouce_Manager_GetTexId(Player_FEVER_Case),
        0.0f, h * 0.15f,
        g_Gage_X, g_Gage_Y
    );
    Sprite_Draw(
        Resouce_Manager_GetTexId(Player_FEVER_Point),
        0.0f + g_PlayerFEVER_OffSetX, h * 0.15f + g_PlayerFEVER_OffSetY,
        playerFeverWidth, g_Gage_Y - g_PlayerFEVER_OffSetY * 2.0f,
        0,0,
        uvWidth,th
    );

    float enemyHpInnerWidth = g_Gage_X - g_EnemyHP_OffSetX * 2.0f;
    float enemyHpWidth = enemyHpInnerWidth * g_EnemyHpRate;
    tw = Texture_Width(Resouce_Manager_GetTexId(Enemy_HP_Point));
    th = Texture_Height(Resouce_Manager_GetTexId(Enemy_HP_Point));
    uvWidth = static_cast<int>(tw * g_EnemyHpRate);
    Sprite_Draw( // EnemyHP
        Resouce_Manager_GetTexId(Enemy_HP_Case),
        w * 0.7f, h * 0.1f,
        g_Gage_X, g_Gage_Y
    );
    Sprite_Draw(
        Resouce_Manager_GetTexId(Enemy_HP_Point),
        w * 0.7f + g_EnemyHP_OffSetX, h * 0.1f + g_EnemyHP_OffSetY,
        enemyHpWidth, g_Gage_Y - g_EnemyHP_OffSetY * 2.0f,
        0,0,
        uvWidth,th
    );

    // スキルポップアップ 
    if (g_SkillPopupTimer > 0.0f) {
        Sprite_Draw(
            g_UI_TextBox_Id,
            g_SkillPopupX, g_SkillPopupY,
            w * 0.25f, h * 0.05f
        );
    }

    // ランク・スコア用背景BOX
    Sprite_Draw(
        g_UI_TextBox_Id,
        w * 0.005f, h * 0.005f,
        w * 0.35f, h * 0.075f,
        { 1.0f,1.0f,0.0f,1.0f }
    );

    Sprite_Draw(
        g_RankTex_Id[g_Rank],
        w * 0.01f, h * 0.01f,
        h * 0.07f, h * 0.07f // 縦基準で正方形
    );

    // テキスト表示
    Text_Begin();
    wchar_t text[64];
    
    // スコア
    swprintf_s(
        text,
        L"SCORE : %d",
        g_Score
    );
    Text_Draw(
        text,
        w * 0.05f, h * 0.02f
    );

    // 撃破数
    if (g_KillCount > 0)
    {
        swprintf_s(
            text,
            L"x %d",
            g_KillCount
        );
        Text_Draw(
            text,
            w * 0.9f, h * 0.05f
        );
    }
    
    // スキルポップアップ（スキル名）
    if (g_SkillPopupTimer > 0.0f) {
        Text_Draw(
            g_SkillPopupText.c_str(),
            g_SkillPopupX, g_SkillPopupY
        );
    }

    Text_End();
}

void UI_SetJudge(int type) {
    g_JudgeType = type;
    g_JudgeTimer = JUDEG_TIMER;
}

void UI_SetPlayerHpRate(float rate){
    g_PlayerHpRate = rate;
}

void UI_SetPlayerFeverRate(float rate){
    g_PlayerFeverRate = rate;
}

void UI_SetEnemyHpRate(float rate){
    g_EnemyHpRate = rate;
}

void UI_SetScore(int score){
    g_Score = score;
}

void UI_SetKillCount(int count){
    g_KillCount = count;
}

void UI_SetRank(Rank_Type rank) {
    if (rank < Rank_D || rank >= Rank_MAX){
        g_Rank = Rank_D;
        return;
    }

    g_Rank = rank;
}

void UI_ShowSkillPopup(const wchar_t* skillName){
    g_SkillPopupText = skillName;
    g_SkillPopupTimer = SKILL_POPUP_TIME;
}