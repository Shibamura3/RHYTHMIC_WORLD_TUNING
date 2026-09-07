/*
	リザルト画面の制御：Result.h

	2026/05/22	hibiki sakuma
*/

#include "Result.h"
#include "resource_manager.h"
#include "text_manager.h"
#include "play_record_manager.h"
#include "sprite.h"
#include "texture.h"
#include "Key_logger.h"
#include "pad_logger.h"
#include "fade.h"
#include "scene.h"
#include "Audio.h"
#include "direct3d.h"
using namespace DirectX;

// 定数宣言
// // 共通
static constexpr int INVALID_ID = -1;

// フェード
static constexpr float FADE_IN_DURATION = 1.0f;
static constexpr float FADE_OUT_DURATION = 0.5f;

// ナビキャラクターウィンドウ
static constexpr float NAVI_WINDOW_X_RATE = 0.67f;
static constexpr float NAVI_WINDOW_Y_RATE = 0.04f;
static constexpr float NAVI_WINDOW_WIDTH_RATE = 0.31f;
static constexpr float NAVI_WINDOW_HEIGHT_RATE = 0.39f;

// ナビキャラクター
static constexpr float NAVI_CHARACTER_X_RATE = 0.72f;
static constexpr float NAVI_CHARACTER_Y_RATE = 0.05f;
static constexpr float NAVI_CHARACTER_WIDTH_RATE = 0.22f;
static constexpr float NAVI_CHARACTER_HEIGHT_RATE = 0.34f;

// プレイヤーキャラクターウィンドウ
static constexpr float PLAYER_WINDOW_X_RATE = 0.67f;
static constexpr float PLAYER_WINDOW_Y_RATE = 0.47f;
static constexpr float PLAYER_WINDOW_WIDTH_RATE = 0.31f;
static constexpr float PLAYER_WINDOW_HEIGHT_RATE = 0.39f;

// プレイヤーキャラクター
static constexpr float PLAYER_CHARACTER_X_RATE = 0.70f;
static constexpr float PLAYER_CHARACTER_Y_RATE = 0.49f;
static constexpr float PLAYER_CHARACTER_WIDTH_RATE = 0.27f;
static constexpr float PLAYER_CHARACTER_HEIGHT_RATE = 0.36f;

// プレイヤー立ち絵の切り抜き範囲
static constexpr float PLAYER_TEXTURE_CROP_X_RATE = 0.0f;
static constexpr float PLAYER_TEXTURE_CROP_Y_RATE = 0.0f;
static constexpr float PLAYER_TEXTURE_CROP_WIDTH_RATE = 1.0f;
static constexpr float PLAYER_TEXTURE_CROP_HEIGHT_RATE = 0.50f;

// 画面遷移指示
static constexpr float MOVE_GUIDE_WIDTH_RATE = 0.40f;
static constexpr float MOVE_GUIDE_HEIGHT_RATE = 0.10f;
static constexpr float MOVE_GUIDE_Y_RATE = 0.90f;
static constexpr float MOVE_GUIDE_ALPHA_BASE = 0.50f;
static constexpr float MOVE_GUIDE_ALPHA_AMPLITUDE = 0.50f;
static constexpr float MOVE_GUIDE_BLINK_SPEED = 3.0f;

// ランク
static constexpr float RANK_X_RATE = 0.03f;
static constexpr float RANK_Y_RATE = 0.05f;
static constexpr float RANK_SIZE_RATE = 0.18f;

// スコア
static constexpr float SCORE_BOX_X_RATE = 0.18f;
static constexpr float SCORE_BOX_Y_RATE = 0.05f;
static constexpr float SCORE_BOX_W_RATE = 0.45f;
static constexpr float SCORE_BOX_H_RATE = 0.08f;

// コメント
static constexpr float COMMENT_BOX_X_RATE = 0.18f;
static constexpr float COMMENT_BOX_Y_RATE = 0.18f;
static constexpr float COMMENT_BOX_W_RATE = 0.45f;
static constexpr float COMMENT_BOX_H_RATE = 0.08f;

// 結果詳細
static constexpr float RESULT_BOX_X_RATE = 0.03f;
static constexpr float RESULT_BOX_Y_RATE = 0.32f;
static constexpr float RESULT_BOX_W_RATE = 0.60f;
static constexpr float RESULT_BOX_H_RATE = 0.55f;

// 色
static constexpr XMVECTORF32 TEXT_COLOR{ 1.0f, 1.0f, 1.0f, 1.0f};

// 結果詳細見出し
static constexpr float RESULT_TITLE_X_RATE = 0.07f;
static constexpr float RESULT_TITLE_Y_RATE = 0.35f;
static constexpr float RESULT_TITLE_SCALE = 1.20f;

// 最大コンボ
static constexpr float MAX_COMBO_LABEL_X_RATE = 0.08f;
static constexpr float MAX_COMBO_VALUE_X_RATE = 0.36f;
static constexpr float MAX_COMBO_Y_RATE = 0.43f;
static constexpr float MAX_COMBO_TEXT_SCALE = 1.00f;

// 判定画像
static constexpr float JUDGE_IMAGE_X_RATE = 0.08f;
static constexpr float JUDGE_IMAGE_WIDTH_RATE = 0.18f;
static constexpr float JUDGE_IMAGE_HEIGHT_RATE = 0.1f;

// 判定数
static constexpr float JUDGE_VALUE_X_RATE = 0.36f;
static constexpr float JUDGE_VALUE_Y_OFFSET_RATE = 0.01f;
static constexpr float JUDGE_VALUE_SCALE = 1.20f;

// 各判定のY座標
static constexpr float PERFECT_Y_RATE = 0.53f;
static constexpr float GREAT_Y_RATE = 0.64f;
static constexpr float MISS_Y_RATE = 0.75f;

// スコア文字
static constexpr float SCORE_TEXT_X_RATE = 0.22f;
static constexpr float SCORE_TEXT_Y_RATE = 0.075f;
static constexpr float SCORE_TEXT_SCALE = 1.20f;

// コメント・結果文字
static constexpr float RESULT_MESSAGE_X_RATE = 0.7f;
static constexpr float RESULT_MESSAGE_Y_RATE = 0.85f;
static constexpr float RESULT_MESSAGE_SCALE = 1.50f;

// ミッション結果
static constexpr float MISSION_RESULT_X_RATE = 0.21f;
static constexpr float MISSION_RESULT_Y_RATE = 0.205f;
static constexpr float MISSION_RESULT_SCALE = 1.10f;

// 特別結果
static constexpr float SPECIAL_RESULT_X_RATE = 0.43f;
static constexpr float SPECIAL_RESULT_Y_RATE = 0.205f;
static constexpr float SPECIAL_RESULT_SCALE = 1.10f;

// 画像・描画共通
static constexpr float SCREEN_LEFT = 0.0f;
static constexpr float SCREEN_TOP = 0.0f;
static constexpr float SPRITE_ALPHA = 1.0f;
static constexpr XMFLOAT4 DEFAULT_SPRITE_COLOR{1.0f, 1.0f, 1.0f, SPRITE_ALPHA};

static constexpr XMVECTORF32 GOLD{ 1.0f, 0.84f,0.0f, 1.0f };
static constexpr int TEXT_NUM = 128;

// 変数宣言
// 画像関連
enum Result_TexId { // キャラクター画像番号
    Result_Fail,
    Result_Clear,
    Result_Fullcombo,
    Result_AllPerfect,
    Result_MAX,
};
static int g_Result_Back_TexId = INVALID_ID;
static int g_ResultMove_TexId = INVALID_ID;
static int g_ResultCharaTexId[Result_MAX]{};
static float g_Result_Move_X = 0.0f;
static float g_Result_Move_Y = 0.0f;
static int g_Result_TextBoxTexId = INVALID_ID;

// ゲーム結果
static ResultType g_ResultType = ResultType::FAIL;
static int g_Result_Perfect = 0;
static int g_Result_Great = 0;
static int g_Result_Miss = 0;
static int g_Result_MaxCombo = 0;
static bool g_Is_FullCombo = false;
static bool g_Is_AllPerfect = false;
static int g_Result_PlayerTexId = INVALID_ID;
static int g_Result_RankTexId = INVALID_ID;
static GameResultData g_ResultData;

static double g_AccumulatedTime = 0.0; // 経過時間

// 画面遷移フラグ
static bool g_Result_FadeFlag = false;

void Result_Initialize(){
    //画面遷移　フェードイン処理
    Fade_Start(FADE_IN_DURATION, false);

    // 画像読み込み
    g_Result_Back_TexId = Resouce_Manager_GetTexId(Result_Back);
    g_ResultMove_TexId = Resouce_Manager_GetTexId(Title_Start);
    g_ResultCharaTexId[Result_Fail] = Resouce_Manager_GetTexId(Rezz_Fail);
    g_ResultCharaTexId[Result_Clear] = Resouce_Manager_GetTexId(Rezz_Clear);
    g_ResultCharaTexId[Result_Fullcombo] = Resouce_Manager_GetTexId(Rezz_Fullcombo);
    g_ResultCharaTexId[Result_AllPerfect] = Resouce_Manager_GetTexId(Rezz_AllPerfect); 
    g_Result_TextBoxTexId = Resouce_Manager_GetTexId(TextBox);
    // 画像サイズの画面依存
    g_Result_Move_X = Direct3D_GetBackBufferWidth() * MOVE_GUIDE_WIDTH_RATE;
    g_Result_Move_Y = Direct3D_GetBackBufferHeight() * MOVE_GUIDE_HEIGHT_RATE;

    g_AccumulatedTime = 0.0;
    g_Result_FadeFlag = false;

    PlayRecord_Update(
        g_ResultData.songId,
        g_ResultData.difficulty,
        g_ResultData.score,
        g_ResultData.rank,
        g_ResultData.resultType == ResultType::CLEAR
    );

    PlayAudio(Resouce_Manager_GetAudioId(Result_BGM), true);
}

void Result_Finalize(){
    StopAudio(Resouce_Manager_GetAudioId(Result_BGM));
}

void Result_Update(double elapsed_time){
    g_AccumulatedTime += elapsed_time;

    const bool anyPressed = KeyLogger_IsAnyTrigger() || PadLogger_IsAnyButtonTrigger(0);

    if (!g_Result_FadeFlag && Fade_GetState() == FADE_STATE_FINISHED_IN && anyPressed){
        g_Result_FadeFlag = true;

        PlayAudio(Resouce_Manager_GetAudioId(Check_SE), false);

        Fade_Start(FADE_OUT_DURATION, true);
    }

    if (g_Result_FadeFlag && Fade_GetState() == FADE_STATE_FINISHED_OUT){
        Scene_Change(SCENE_MUSIC_SELEC);
    }
}

void Result_Draw(){
    float BASE_X = (float)Direct3D_GetBackBufferWidth();
    float BASE_Y = (float)Direct3D_GetBackBufferHeight();
    // 画像表示
    // 背景表示
    Sprite_Draw(g_Result_Back_TexId, 0, 0, BASE_X, BASE_Y);

    // プレイヤーキャラクター用ウィンドウ
    Sprite_Draw(
        g_Result_TextBoxTexId,
        BASE_X * PLAYER_WINDOW_X_RATE,
        BASE_Y * PLAYER_WINDOW_Y_RATE,
        BASE_X * PLAYER_WINDOW_WIDTH_RATE,
        BASE_Y * PLAYER_WINDOW_HEIGHT_RATE
    );
    // 使用したキャラクターの立ち絵表示
    if (g_Result_PlayerTexId != INVALID_ID){
        const int textureWidth = static_cast<int>(Texture_Width(g_Result_PlayerTexId));
        const int textureHeight = static_cast<int>(Texture_Height(g_Result_PlayerTexId));

        const int cropX = static_cast<int>(static_cast<float>(textureWidth) *PLAYER_TEXTURE_CROP_X_RATE);
        const int cropY = static_cast<int>(static_cast<float>(textureHeight) *PLAYER_TEXTURE_CROP_Y_RATE);

        const int cropWidth = static_cast<int>(static_cast<float>(textureWidth) * PLAYER_TEXTURE_CROP_WIDTH_RATE);
        const int cropHeight = static_cast<int>(static_cast<float>(textureHeight) *PLAYER_TEXTURE_CROP_HEIGHT_RATE);

        Sprite_Draw(
            g_Result_PlayerTexId,
            BASE_X * PLAYER_CHARACTER_X_RATE, BASE_Y * PLAYER_CHARACTER_Y_RATE,
            BASE_X * PLAYER_CHARACTER_WIDTH_RATE, BASE_Y * PLAYER_CHARACTER_HEIGHT_RATE,
            cropX, cropY,
            cropWidth, cropHeight
        );
    }

    // スコア表示ウィンドウ
    Sprite_Draw(
        g_Result_TextBoxTexId,
        BASE_X * SCORE_BOX_X_RATE, BASE_Y * SCORE_BOX_Y_RATE,
        BASE_X * SCORE_BOX_W_RATE, BASE_Y * SCORE_BOX_H_RATE
    );

    // ゲーム結果ウィンドウ
    Sprite_Draw(
        g_Result_TextBoxTexId,
        BASE_X * RESULT_BOX_X_RATE, BASE_Y * RESULT_BOX_Y_RATE,
        BASE_X * RESULT_BOX_W_RATE, BASE_Y * RESULT_BOX_H_RATE
    );

    // コメントの表示ウィンドウ
    Sprite_Draw(
        g_Result_TextBoxTexId,
        BASE_X * COMMENT_BOX_X_RATE, BASE_Y * COMMENT_BOX_Y_RATE,
        BASE_X * COMMENT_BOX_W_RATE, BASE_Y * COMMENT_BOX_H_RATE
    );

    // ナビキャラクター用ウィンドウ
    Sprite_Draw(
        g_Result_TextBoxTexId,
        BASE_X * NAVI_WINDOW_X_RATE,
        BASE_Y * NAVI_WINDOW_Y_RATE,
        BASE_X * NAVI_WINDOW_WIDTH_RATE,
        BASE_Y * NAVI_WINDOW_HEIGHT_RATE
    );
    // 結果に応じたキャラクターの表示
    int charaTex = INVALID_ID;
    if (g_Is_AllPerfect) {
        charaTex = g_ResultCharaTexId[Result_AllPerfect];
    } else if (g_Is_FullCombo) {
        charaTex = g_ResultCharaTexId[Result_Fullcombo];
    } else if (g_ResultType == ResultType::CLEAR) {
        charaTex = g_ResultCharaTexId[Result_Clear];
    } else {
        charaTex = g_ResultCharaTexId[Result_Fail];
    }

    Sprite_Draw(
        charaTex,
        BASE_X * NAVI_CHARACTER_X_RATE, BASE_Y * NAVI_CHARACTER_Y_RATE,
        BASE_X * NAVI_CHARACTER_WIDTH_RATE, BASE_Y * NAVI_CHARACTER_HEIGHT_RATE
    );

    // 画面遷移指示
    const float alpha =
        MOVE_GUIDE_ALPHA_BASE +
        sinf(static_cast<float>(g_AccumulatedTime) * MOVE_GUIDE_BLINK_SPEED) *
        MOVE_GUIDE_ALPHA_AMPLITUDE;

    XMFLOAT4 color = { 1.0f,1.0f,1.0f,alpha };
    Sprite_Draw(
        g_ResultMove_TexId,
        BASE_X * 0.5f - g_Result_Move_X * 0.5f, BASE_Y * MOVE_GUIDE_Y_RATE,
        g_Result_Move_X, g_Result_Move_Y,
        color
    );

    // ランク表示
    Sprite_Draw(
        g_Result_RankTexId,
        BASE_X * RANK_X_RATE, BASE_Y * RANK_Y_RATE,
        BASE_Y * RANK_SIZE_RATE, BASE_Y * RANK_SIZE_RATE
    );

    // 判定画像
    Sprite_Draw(
        Resouce_Manager_GetTexId(Combo_Perfect),
        BASE_X * JUDGE_IMAGE_X_RATE, BASE_Y * PERFECT_Y_RATE,
        BASE_X * JUDGE_IMAGE_WIDTH_RATE,  BASE_Y * JUDGE_IMAGE_HEIGHT_RATE
    );

    Sprite_Draw(
        Resouce_Manager_GetTexId(Combo_Great),
        BASE_X * JUDGE_IMAGE_X_RATE, BASE_Y * GREAT_Y_RATE,
        BASE_X * JUDGE_IMAGE_WIDTH_RATE, BASE_Y * JUDGE_IMAGE_HEIGHT_RATE
    );

    Sprite_Draw(
        Resouce_Manager_GetTexId(Combo_Miss),
        BASE_X * JUDGE_IMAGE_X_RATE, BASE_Y * MISS_Y_RATE,
        BASE_X * JUDGE_IMAGE_WIDTH_RATE, BASE_Y * JUDGE_IMAGE_HEIGHT_RATE
    );

    Text_Begin();

    // 結果詳細見出し
    Text_Draw(
        L"GAME RESULT",
        BASE_X* RESULT_TITLE_X_RATE,
        BASE_Y* RESULT_TITLE_Y_RATE,
        TEXT_COLOR,
        RESULT_TITLE_SCALE
    );

    // スコア表示
    wchar_t scoreText[TEXT_NUM];

    swprintf_s(
        scoreText,
        L"SCORE : %d",
        g_ResultData.score
    );

    Text_Draw(
        scoreText,
        BASE_X* SCORE_TEXT_X_RATE,
        BASE_Y* SCORE_TEXT_Y_RATE,
        TEXT_COLOR,
        SCORE_TEXT_SCALE
    );

    // 最大コンボ表示
    Text_Draw(
        L"MAX COMBO",
        BASE_X * MAX_COMBO_LABEL_X_RATE,
        BASE_Y * MAX_COMBO_Y_RATE,
        TEXT_COLOR,
        MAX_COMBO_TEXT_SCALE
    );

    wchar_t text[TEXT_NUM];

    swprintf_s(
        text,
        L"%d",
        g_Result_MaxCombo
    );

    Text_Draw(
        text,
        BASE_X * MAX_COMBO_VALUE_X_RATE,
        BASE_Y * MAX_COMBO_Y_RATE,
        TEXT_COLOR,
        MAX_COMBO_TEXT_SCALE
    );

    // ゲーム結果から表示内容を決定
    const wchar_t* resultText = L"";
    const wchar_t* comment = L"";
    switch (g_ResultType) {
    case ResultType::FAIL:
        resultText = L"MISSION  FAIL";
        comment = L"TRY AGAIN!";
        break;

    case ResultType::CLEAR:
        resultText = L"MISSION  CLEAR";
        comment = L"GOOD JOB!";
        break;
    }
    // コメントの表示
    Text_Draw(
        comment,
        BASE_X * RESULT_MESSAGE_X_RATE,
        BASE_Y * RESULT_MESSAGE_Y_RATE,
        TEXT_COLOR,
        RESULT_MESSAGE_SCALE
    );

    // ゲーム結果の表示
    // ミッション結果をウィンドウ左側へ表示
    Text_Draw(
        resultText,
        BASE_X * MISSION_RESULT_X_RATE,
        BASE_Y * MISSION_RESULT_Y_RATE,
        TEXT_COLOR,
        MISSION_RESULT_SCALE
    );

    // 特別結果をウィンドウ右側へ表示
    const wchar_t* specialResultText = L"";

    if (g_Is_AllPerfect){
        specialResultText = L"ALL PERFECT!!";

    }else if (g_Is_FullCombo){
        specialResultText = L"FULL COMBO!!";
    }

    if (specialResultText[0] != L'\0'){
        Text_Draw(
            specialResultText,
            BASE_X * SPECIAL_RESULT_X_RATE,
            BASE_Y * SPECIAL_RESULT_Y_RATE,
            GOLD,
            SPECIAL_RESULT_SCALE
        );
    }

    // Perfect数
    swprintf_s(text, L"%d", g_Result_Perfect);
    Text_Draw(
        text,
        BASE_X * JUDGE_VALUE_X_RATE,
        BASE_Y * (PERFECT_Y_RATE + JUDGE_VALUE_Y_OFFSET_RATE),
        TEXT_COLOR,
        JUDGE_VALUE_SCALE
    );

    // Great数
    swprintf_s(text, L"%d", g_Result_Great);
    Text_Draw(
        text,
        BASE_X * JUDGE_VALUE_X_RATE,
        BASE_Y * (GREAT_Y_RATE + JUDGE_VALUE_Y_OFFSET_RATE),
        TEXT_COLOR,
        JUDGE_VALUE_SCALE
    );

    // Miss数
    swprintf_s(text, L"%d", g_Result_Miss);
    Text_Draw(
        text,
        BASE_X * JUDGE_VALUE_X_RATE,
        BASE_Y * (MISS_Y_RATE + JUDGE_VALUE_Y_OFFSET_RATE),
        TEXT_COLOR,
        JUDGE_VALUE_SCALE
    );
    
    Text_End();
}

void Result_SetData(const GameResultData& data){
    g_ResultData = data;

    switch (g_ResultData.rank)
    {
    case Rank_S:
        g_Result_RankTexId = Resouce_Manager_GetTexId(Rank_IMG_S);
        break;

    case Rank_A:
        g_Result_RankTexId = Resouce_Manager_GetTexId(Rank_IMG_A);
        break;

    case Rank_B:
        g_Result_RankTexId = Resouce_Manager_GetTexId(Rank_IMG_B);
        break;

    case Rank_C:
        g_Result_RankTexId = Resouce_Manager_GetTexId(Rank_IMG_C);
        break;

    case Rank_D:
    default:
        g_Result_RankTexId = Resouce_Manager_GetTexId(Rank_IMG_D);
        break;
    }

    g_ResultType = data.resultType;
    g_Result_MaxCombo = data.maxCombo;
    g_Result_Perfect = data.perfectCount;
    g_Result_Great = data.greatCount;
    g_Result_Miss = data.missCount;
    g_Is_AllPerfect = data.isAllPerfect;
    g_Is_FullCombo = data.isFullCombo;

    switch (data.character)
    {
    case PlayerCharacter::Riff:
        g_Result_PlayerTexId = Resouce_Manager_GetTexId(Riff_Smile);
        break;

    case PlayerCharacter::Luna:
        g_Result_PlayerTexId = Resouce_Manager_GetTexId(Luna_Smile);
        break;

    case PlayerCharacter::Melody:
        g_Result_PlayerTexId = Resouce_Manager_GetTexId(Melody_Smile);
        break;
    default:
        g_Result_PlayerTexId = Resouce_Manager_GetTexId(Riff_Smile);
        break;

    }
}