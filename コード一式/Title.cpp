/*
	タイトル画面の制御：Title.h

	2026/05/22	hibiki sakuma
*/

#include "Title.h"

#include "Audio.h"
#include "Key_logger.h"
#include "direct3d.h"
#include "fade.h"
#include "pad_logger.h"
#include "resource_manager.h"
#include "scene.h"
#include "sprite.h"

#include <cmath>
#include <cstdlib>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

// 定数宣言
// 背景ノーツ
static constexpr int BG_NOTES_MAX = 10;
static constexpr int BG_ANIM_PATTERN = 3;

static constexpr float BG_NOTE_MIN_SPEED = 90.0f;
static constexpr int BG_NOTE_SPEED_RANDOM_RANGE = 30;
static constexpr int BG_NOTE_PHASE_RANDOM_RANGE = 100;

static constexpr float BG_NOTE_WAVE_SPEED = 2.0f;
static constexpr float BG_NOTE_WAVE_POWER = 20.0f;

static constexpr float BG_NOTE_RESET_X = -100.0f;
static constexpr int BG_NOTE_RESPAWN_RANDOM_RANGE = 200;

static constexpr float BG_NOTE_DRAW_SIZE = 64.0f;

// フェード
static constexpr double TITLE_FADE_IN_TIME = 1.0;
static constexpr double TITLE_FADE_OUT_TIME = 0.5;

// タイトルロゴ
static constexpr float TITLE_LOGO_WIDTH_RATE = 0.60f;
static constexpr float TITLE_LOGO_HEIGHT_RATE = 0.20f;

static constexpr float TITLE_LOGO_CENTER_X_RATE = 0.50f;
static constexpr float TITLE_LOGO_Y_RATE = 0.10f;

static constexpr float TITLE_LOGO_FLOAT_SPEED = 2.0f;
static constexpr float TITLE_LOGO_FLOAT_RANGE = 10.0f;

// スタート画像
static constexpr float TITLE_START_WIDTH_RATE = 0.40f;
static constexpr float TITLE_START_HEIGHT_RATE = 0.10f;

static constexpr float TITLE_START_CENTER_X_RATE = 0.50f;
static constexpr float TITLE_START_Y_RATE = 0.80f;

static constexpr float TITLE_START_BASE_ALPHA = 0.50f;
static constexpr float TITLE_START_ALPHA_RANGE = 0.50f;
static constexpr float TITLE_START_BLINK_SPEED = 3.0f;

// キャラクター立ち絵
static constexpr float TITLE_CHARACTER_X_RATE = 0.60f;
static constexpr float TITLE_CHARACTER_Y_RATE = 0.15f;
static constexpr float TITLE_CHARACTERSIZE_X_RATE = 0.70f;
static constexpr float TITLE_CHARACTERSIZE_Y_RATE = 1.4f;

// 画像読み込み用
static int g_TitleBack_TexId = -1;
static int g_TitleLogo_TexId = -1;
static int g_TitleStart_TexId = -1;
static int g_NoteTex[BG_ANIM_PATTERN]{};
static int g_TitleChara_TexId = -1;
static float g_Title_Logo_X = -1;
static float g_Title_Logo_Y = -1;
static float g_Title_Start_X = -1;
static float g_Title_Start_Y = -1;

// 音源読み込み用
static int g_Start_SE = -1;
static int g_Title_BGM = -1;

static bool g_Title_FadeFlag = false;

// 演出関連
struct BgNote {
	int texId;
	float x;
	float y;
	float speed;
	float offset;
};
static std::vector<BgNote> g_BgNotes;

static double g_AccumulatedTime = 0.0; // 経過時間

void Title_Initialize()
{
    // タイトル画面へのフェードイン
    Fade_Start(TITLE_FADE_IN_TIME, false);

    // 使用する画像の取得
    g_TitleBack_TexId = Resouce_Manager_GetTexId(Title_Back);
    g_TitleLogo_TexId = Resouce_Manager_GetTexId(Title_Logo);
    g_TitleStart_TexId = Resouce_Manager_GetTexId(Title_Start);
    g_TitleChara_TexId = Resouce_Manager_GetTexId(Riff_Smile);
    g_NoteTex[0] = Resouce_Manager_GetTexId(Anim_P1);
    g_NoteTex[1] = Resouce_Manager_GetTexId(Anim_P2);
    g_NoteTex[2] = Resouce_Manager_GetTexId(Anim_P3);

    const float screenWidth = (float)Direct3D_GetBackBufferWidth();
    const float screenHeight = (float)Direct3D_GetBackBufferHeight();

    // 画面サイズに応じた画像サイズ
    g_Title_Logo_X = screenWidth * TITLE_LOGO_WIDTH_RATE;
    g_Title_Logo_Y = screenHeight * TITLE_LOGO_HEIGHT_RATE;
    g_Title_Start_X = screenWidth * TITLE_START_WIDTH_RATE;
    g_Title_Start_Y = screenHeight * TITLE_START_HEIGHT_RATE;

    // タイトル画面の状態を初期化
    g_Title_FadeFlag = false;
    g_AccumulatedTime = 0.0;

    g_BgNotes.clear();
    g_BgNotes.reserve(BG_NOTES_MAX);

    for (int i = 0; i < BG_NOTES_MAX; ++i) {
        BgNote note{};

        note.x = static_cast<float>(rand() % (int)screenWidth);
        note.y = static_cast<float>(rand() % (int)screenHeight);
        note.speed = BG_NOTE_MIN_SPEED + static_cast<float>(rand() % BG_NOTE_SPEED_RANDOM_RANGE);
        note.offset = static_cast<float>(rand() % BG_NOTE_PHASE_RANDOM_RANGE);
        note.texId = g_NoteTex[rand() % BG_ANIM_PATTERN];

        g_BgNotes.push_back(note);
    }

    // 使用する音源の取得
    g_Start_SE = Resouce_Manager_GetAudioId(Title_AnyPress_SE);
    g_Title_BGM = Resouce_Manager_GetAudioId(Title_BGM);

    PlayAudio(g_Title_BGM, true);
}

void Title_Finalize(){
	// サウンドストップはココに入れる
	StopAudio(g_Title_BGM);
	StopAudio(g_Start_SE);

	g_Start_SE = -1;
	g_Title_BGM = -1;

}

void Title_Update(double elapsed_time){
    const bool anyPressed = KeyLogger_IsAnyTrigger() || PadLogger_IsAnyButtonTrigger(0);

    if (!g_Title_FadeFlag &&  Fade_GetState() == FADE_STATE_FINISHED_IN && anyPressed){
        g_Title_FadeFlag = true;

        if (g_Start_SE >= 0){
            PlayAudio(g_Start_SE, false);
        }

        Fade_Start(TITLE_FADE_OUT_TIME, true);
    }

    if (g_Title_FadeFlag && Fade_GetState() == FADE_STATE_FINISHED_OUT){
        Scene_Change(SCENE_MUSIC_SELEC);

        return;
    }

    const float elapsedTime = static_cast<float>(elapsed_time);
    const float animationTime = static_cast<float>(g_AccumulatedTime);

    const int screenWidth = Direct3D_GetBackBufferWidth();

    const int screenHeight = Direct3D_GetBackBufferHeight();

    // 背景ノーツを更新
    for (auto& note : g_BgNotes)
    {
        note.x -= note.speed * elapsedTime;

        const float wave =
            sinf(
                animationTime *
                BG_NOTE_WAVE_SPEED +
                note.offset
            ) * BG_NOTE_WAVE_POWER;

        note.y += wave * elapsedTime;

        // 画面左側へ出たら右側へ再配置
        if (note.x < BG_NOTE_RESET_X){
            note.x = static_cast<float>( screenWidth ) +
                static_cast<float>(rand() % BG_NOTE_RESPAWN_RANDOM_RANGE);

            note.y = static_cast<float>(rand() %screenHeight);
        }
    }

    g_AccumulatedTime += elapsed_time;
}

void Title_Draw(){
    const float screenWidth = static_cast<float>(Direct3D_GetBackBufferWidth());
    const float screenHeight = static_cast<float>(Direct3D_GetBackBufferHeight());

    const float animationTime = static_cast<float>(g_AccumulatedTime);

    // 背景
    Sprite_Draw(
        g_TitleBack_TexId,
        0.0f, 0.0f,
        screenWidth, screenHeight
    );

    // 背景ノーツ
    for (const auto& note : g_BgNotes){
        Sprite_Draw(
            note.texId,
            note.x, note.y,
            BG_NOTE_DRAW_SIZE, BG_NOTE_DRAW_SIZE
        );
    }

    // キャラクター立ち絵
    Sprite_Draw(
        g_TitleChara_TexId,
        screenWidth * TITLE_CHARACTER_X_RATE, screenHeight * TITLE_CHARACTER_Y_RATE,
        screenWidth * TITLE_CHARACTERSIZE_X_RATE, screenHeight * TITLE_CHARACTERSIZE_Y_RATE

    );

    // タイトルロゴの浮遊演出
    const float logoX = screenWidth * TITLE_LOGO_CENTER_X_RATE - g_Title_Logo_X * 0.5f;

    const float logoY = screenHeight * TITLE_LOGO_Y_RATE +
        sinf( animationTime * TITLE_LOGO_FLOAT_SPEED) *
        TITLE_LOGO_FLOAT_RANGE;

    Sprite_Draw(
        g_TitleLogo_TexId,
        logoX, logoY,
        g_Title_Logo_X, g_Title_Logo_Y
    );

    // AnyPress画像の点滅
    const float startAlpha =
        TITLE_START_BASE_ALPHA +
        sinf( animationTime * TITLE_START_BLINK_SPEED) * TITLE_START_ALPHA_RANGE;

    const XMFLOAT4 startColor ={ 1.0f, 1.0f, 1.0f, startAlpha};

    const float startX = screenWidth * TITLE_START_CENTER_X_RATE - g_Title_Start_X * 0.5f;
    const float startY = screenHeight * TITLE_START_Y_RATE;

    Sprite_Draw(
        g_TitleStart_TexId,
        startX, startY,
        g_Title_Start_X, g_Title_Start_Y,
        startColor
    );
}