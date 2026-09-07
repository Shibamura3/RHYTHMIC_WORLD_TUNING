/*
	エディットモードの制御：Edit.cpp

	2026/05/22	hibiki sakuma
*/

#include "Edit.h"
#include "resource_manager.h"
#include "text_manager.h"
#include "sprite.h"
#include "Audio.h"
#include "pad_logger.h"
#include "Key_logger.h"
#include "fade.h"
#include "scene.h"
#include "direct3d.h"

#include <algorithm>
#include <vector>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <DirectXMath.h>
using namespace DirectX;

// データ型
// エディット画面のレーン
enum class EditLane
{
    Up = 0,
    Down = 1,

    Max
};
// エディットで配置するノーツ
enum class EditNoteType
{
    Normal = 0,
    Long = 1,
    Rapid = 2,

    Max
};
// エディット画面の操作状態
enum class EditState
{
    Timeline,
    PlacingRangeNote
};

// エディット画面上のノーツデータ
struct EditNoteData
{
    float time = 0.0f;
    EditLane lane = EditLane::Up;
    EditNoteType type = EditNoteType::Normal;
    float param = 0.0f;
};
// 配置中のLong・Rapidノーツ
struct PendingRangeNote
{
    float startTime = 0.0f;
    float endTime = 0.0f;

    EditLane lane = EditLane::Up;
    EditNoteType type = EditNoteType::Long;
};
// 保存状態
enum class SaveResult
{
    None,
    Success,
    Failed
};
// 入力リピート用
struct InputRepeatState
{
    float timer = 0.0f;
};
// 定数宣言
// フェード
static constexpr float FADE_IN_DURATION = 1.0f;
static constexpr float FADE_OUT_DURATION = 0.5f;
// 無効ID
static constexpr int INVALID_ID = -1;
static constexpr int INVALID_NOTE_INDEX = -1;
// タイムライン
static constexpr float TIMELINE_X_RATE = 0.05f;
static constexpr float TIMELINE_Y_RATE = 0.35f;
static constexpr float TIMELINE_WIDTH_RATE = 0.90f;
static constexpr float TIMELINE_HEIGHT_RATE = 0.30f;
// 中央カーソル
static constexpr float TIMELINE_CURSOR_WIDTH_RATE = 0.004f;
// タイムライン表示範囲
static constexpr float TIMELINE_VISIBLE_SECONDS = 10.0f;
static constexpr float TIMELINE_HALF_SECONDS = TIMELINE_VISIBLE_SECONDS * 0.5f;

// 時間操作
static constexpr float TIMELINE_TIME_STEP = 0.10f;
static constexpr float TIMELINE_MIN_TIME = 0.0f;
// スティック入力
static constexpr float STICK_INPUT_THRESHOLD = 0.60f;
// 入力リピート
static constexpr float INPUT_REPEAT_START_DELAY = 0.35f;
static constexpr float INPUT_REPEAT_INTERVAL = 0.08f;

// 時刻表示
static constexpr float TIME_TEXT_Y_OFFSET_RATE = 0.06f;
static constexpr float TIME_TEXT_X_OFFSET_RATE = 0.035f;
static constexpr float TIME_TEXT_SCALE = 0.80f;
static constexpr int TIME_TEXT_BUFFER_SIZE = 32;
// 時間目盛り
static constexpr float TIMELINE_MARK_INTERVAL = 1.0f;
static constexpr float TIMELINE_MARK_WIDTH_RATE = 0.002f;
static constexpr float TIMELINE_MARK_HEIGHT_RATE = 0.04f;
static constexpr float MARK_TEXT_Y_OFFSET_RATE = 0.035f;
static constexpr float MARK_TEXT_X_OFFSET_RATE = 0.012f;
static constexpr float MARK_TEXT_SCALE = 0.45f;
static constexpr int MARK_TEXT_BUFFER_SIZE = 16;

// レーン
static constexpr float LANE_DIVIDER_HEIGHT_RATE = 0.005f;
static constexpr int LANE_COUNT = static_cast<int>(EditLane::Max);
// 選択レーン背景
static constexpr float SELECTED_LANE_ALPHA = 0.28f;
// レーン名
static constexpr float LANE_TEXT_X_OFFSET_RATE = 0.04f;
static constexpr float LANE_TEXT_Y_OFFSET_RATE = 0.02f;

static constexpr float LANE_TEXT_SCALE = 0.60f;
// ノーツ種類表示ウィンドウ
static constexpr float NOTE_TYPE_BOX_X_RATE = 0.73f;
static constexpr float NOTE_TYPE_BOX_Y_RATE = 0.05f;
static constexpr float NOTE_TYPE_BOX_WIDTH_RATE = 0.22f;
static constexpr float NOTE_TYPE_BOX_HEIGHT_RATE = 0.12f;
// ノーツ種類表示文字
static constexpr float NOTE_TYPE_LABEL_X_RATE = 0.75f;
static constexpr float NOTE_TYPE_LABEL_Y_RATE = 0.065f;
static constexpr float NOTE_TYPE_NAME_X_RATE = 0.75f;
static constexpr float NOTE_TYPE_NAME_Y_RATE = 0.11f;
static constexpr float NOTE_TYPE_LABEL_SCALE = 0.50f;
static constexpr float NOTE_TYPE_NAME_SCALE = 0.80f;
// ノーツ種類
static constexpr int NOTE_TYPE_COUNT = static_cast<int>(EditNoteType::Max);
// 範囲ノーツ
static constexpr float RANGE_NOTE_MIN_DURATION = TIMELINE_TIME_STEP;
// ノーツ表示
static constexpr float EDIT_NOTE_WIDTH_RATE = 0.025f;
static constexpr float EDIT_NOTE_HEIGHT_RATE = 0.10f;
// 配置予定ノーツ
static constexpr float NOTE_PREVIEW_ALPHA = 0.55f;
// 同一位置判定
static constexpr float NOTE_TIME_EPSILON = 0.001f;
// CSV保存
static constexpr const char* EDIT_SCORE_OUTPUT_PATH = "resource/data/score_data/score_edit.csv";
static constexpr int SCORE_VALUE_DECIMAL_DIGITS = 1;
// 保存結果表示
static constexpr float SAVE_MESSAGE_DISPLAY_TIME = 2.0f;
static constexpr float SAVE_MESSAGE_X_RATE = 0.42f;
static constexpr float SAVE_MESSAGE_Y_RATE = 0.22f;
static constexpr float SAVE_MESSAGE_SCALE = 0.80f;
// 操作説明
static constexpr float GUIDE_TEXT_X_RATE = 0.02f;
static constexpr float GUIDE_TEXT_LINE_1_Y_RATE = 0.80f;
static constexpr float GUIDE_TEXT_LINE_2_Y_RATE = 0.85f;
static constexpr float GUIDE_TEXT_LINE_3_Y_RATE = 0.90f;
static constexpr float GUIDE_TEXT_SCALE = 0.50f;
// 再生確認
static constexpr float PLAY_STATE_X_RATE = 0.05f;
static constexpr float PLAY_STATE_Y_RATE = 0.05f;
static constexpr float PLAY_STATE_SCALE = 0.70f;

// 色
static constexpr XMFLOAT4 TIMELINE_BACKGROUND_COLOR{0.05f, 0.05f, 0.10f, 0.85f};
static constexpr XMFLOAT4 TIMELINE_DIVIDER_COLOR{1.0f, 1.0f, 1.0f, 0.40f};
static constexpr XMFLOAT4 TIMELINE_CURSOR_COLOR{1.0f, 0.85f, 0.05f, 1.0f};
static constexpr XMVECTORF32 TIME_TEXT_COLOR{ 0.0f, 0.0f, 0.0f, 1.0f};
static constexpr XMFLOAT4 TIMELINE_MARK_COLOR{ 1.0f, 1.0f, 1.0f, 0.35f};
static constexpr XMVECTORF32 MARK_TEXT_COLOR{ 0.0f, 0.0f, 0.0f, 1.0f};
static constexpr XMFLOAT4 SELECTED_LANE_COLOR{ 1.0f, 0.85f, 0.05f, SELECTED_LANE_ALPHA };
static constexpr XMVECTORF32 SELECTED_LANE_TEXT_COLOR{ 1.0f, 0.85f, 0.05f,1.0f };
static constexpr XMVECTORF32 UNSELECTED_LANE_TEXT_COLOR{ 1.0f, 1.0f, 1.0f,0.5f };
static constexpr XMVECTORF32 NOTE_TYPE_LABEL_COLOR{ 1.0f,1.0f,1.0f,1.0f };
static constexpr XMVECTORF32 NOTE_TYPE_NORMAL_COLOR{ 0.25f,0.75f,1.0f,1.0f };
static constexpr XMVECTORF32 NOTE_TYPE_LONG_COLOR{ 0.25f,1.0f,0.25f,1.0f };
static constexpr XMVECTORF32 NOTE_TYPE_RAPID_COLOR{ 1.0f,0.80f,0.10f,1.0f };
static constexpr XMFLOAT4 NORMAL_NOTE_COLOR{ 0.25f, 0.75f, 1.0f, 1.0f };
static constexpr XMFLOAT4 LONG_NOTE_COLOR{ 0.25f,1.0f,0.25f,1.0f };
static constexpr XMFLOAT4 RAPID_NOTE_COLOR{ 1.0f, 0.80f, 0.10f, 1.0f };
static constexpr XMFLOAT4 NOTE_PREVIEW_COLOR{ 1.0f,1.0f,1.0f,NOTE_PREVIEW_ALPHA };
static constexpr XMFLOAT4 INVALID_NOTE_PREVIEW_COLOR{ 1.0f, 0.20f, 0.20f, 0.65f };
static constexpr XMVECTORF32 SAVE_SUCCESS_COLOR{ 0.10f, 0.80f, 0.25f, 1.0f };
static constexpr XMVECTORF32 SAVE_FAILED_COLOR{ 1.0f,0.20f,0.20f,1.0f };
static constexpr XMVECTORF32 GUIDE_TEXT_COLOR{ 0.0f, 0.0f, 0.0f, 1.0f };
static constexpr XMVECTORF32 PLAYING_TEXT_COLOR{ 0.20f, 0.90f, 0.30f, 1.0f };
static constexpr XMVECTORF32 STOPPED_TEXT_COLOR{ 0.50f, 0.50f, 0.50f, 1.0f };

// 変数宣言
// 画面サイズ
static float g_ScreenX = 0.0f;
static float g_ScreenY = 0.0f;
// カーソル
static float g_CurrentTime = 0.0f;
// スティック入力状態
static bool g_StickLeftPrev = false;
static bool g_StickRightPrev = false;
static bool g_StickUpPrev = false;
static bool g_StickDownPrev = false;
// 入力リピート状態
static InputRepeatState g_LeftRepeatState;
static InputRepeatState g_RightRepeatState;
// 選択中レーン
static EditLane g_SelectedLane = EditLane::Up;
// 選択中のノーツ
static EditNoteType g_SelectedNoteType = EditNoteType::Normal;
// 編集中のノーツ
static std::vector<EditNoteData> g_EditNotes;
// エディット操作状態
static EditState g_EditState = EditState::Timeline;
// 配置中の範囲ノーツ
static PendingRangeNote g_PendingRangeNote;
// 現在の保存状態
static SaveResult g_SaveResult = SaveResult::None;
static float g_SaveMessageTimer = 0.0f;
// 画面遷移
static bool g_EditFadeOutStarted = false;
// 楽曲再生
static bool g_IsMusicPlaying = false;
static int g_EditMusicAudioId = INVALID_ID;

// 画像用変数
static int g_BackgroundTexId = INVALID_ID;
static int g_WhiteTexId = INVALID_ID;
static int g_TextBoxTexId = INVALID_ID;

// 内部関数
// // 入力リピート
static bool UpdateInputRepeat(bool isTriggered, bool isPressed, InputRepeatState& repeatState, float elapsedTime);
// タイムラインの時間移動
static void MoveTimeline(float amount);
// レーン選択
static void MoveLaneSelection(int direction);
// レーンの描画
static void DrawSelectedLane();
static void DrawLaneLabels();
// ノーツ種類選択
static void ChangeNoteType(int direction);
static void SetNoteType(EditNoteType noteType);
// ノーツ種類表示
static void DrawNoteTypeWindow();
static void DrawNoteTypeText();
static const wchar_t* GetNoteTypeName(EditNoteType noteType);
static XMVECTORF32 GetNoteTypeColor(EditNoteType noteType);
// ノーツ配置
static void PlaceSelectedNote();
static void PlaceNormalNote();
// 再生中のNormalノーツ仮配置
static void PlacePlaybackNormalNote();
// ノーツ削除
static int FindNoteIndexAtCurrentPosition();
static void RemoveNoteAtCurrentPosition();
// ノーツ描画
static void DrawPlacedNotes();
static void DrawNotePreview();
// 範囲ノーツ配置
static void StartRangeNotePlacement();
static void ConfirmRangeNotePlacement();
static void CancelRangeNotePlacement();
static void PlaceRangeNote();
// 範囲ノーツ描画
static void DrawRangeNote(float startTime, float duration, EditLane lane, const XMFLOAT4& color);
static void DrawPendingRangeNote();
// ノーツ重複判定
static float GetNoteEndTime(const EditNoteData& note);
static bool DoNoteRangesOverlap(float firstStartTime, float firstEndTime, float secondStartTime, float secondEndTime);
static bool CanPlaceNote(float startTime, float duration, EditLane lane);
// ノーツ色
static XMFLOAT4 GetPlacedNoteColor(EditNoteType noteType);
// 画面遷移
static void StartReturnToMusicSelect();
// 楽曲再生
static void ToggleMusicPlayback();
static void StartMusicPlayback();
static void StopMusicPlayback();
static void DrawPlaybackState();
// 座標計算
static float GetTimelineXFromTime(float noteTime);
static float GetLaneCenterY(EditLane lane);
// タイムラインの描画
static void DrawCurrentTime();
static void DrawTimelineBase();
static void DrawTimelineCursor(); 
static void DrawTimelineMarks();
static void DrawTimelineMarkLabels();
// 操作説明
static void DrawControlGuide();
// CSV保存
static bool SaveChartToCsv(const char* filePath);
static void SaveCurrentChart();
static void DrawSaveResultText();

void Edit_Initialize(){
	Fade_Start(FADE_IN_DURATION, false);
	// 変数初期化
	g_ScreenX = (float)Direct3D_GetBackBufferWidth();
	g_ScreenY = (float)Direct3D_GetBackBufferHeight();

    g_CurrentTime = TIMELINE_MIN_TIME;

    g_SelectedLane = EditLane::Up;
    g_SelectedNoteType = EditNoteType::Normal;

    g_EditNotes.clear();

    g_EditState = EditState::Timeline;
    g_PendingRangeNote = {};
    g_SaveResult = SaveResult::None;
    g_SaveMessageTimer = 0.0f;

    g_StickLeftPrev = false;
    g_StickRightPrev = false;
    g_StickUpPrev = false;
    g_StickDownPrev = false;

    g_LeftRepeatState = {};
    g_RightRepeatState = {};

    g_EditFadeOutStarted = false;

    g_IsMusicPlaying = false;
    g_EditMusicAudioId = Resouce_Manager_GetAudioId(Edit_Game_BGM);

	g_BackgroundTexId = Resouce_Manager_GetTexId(Select_Back);
	g_WhiteTexId = Resouce_Manager_GetTexId(Color_White);
    g_TextBoxTexId = Resouce_Manager_GetTexId(TextBox);
}

void Edit_Finalize(){
    StopMusicPlayback();

    g_EditNotes.clear();

    g_BackgroundTexId = INVALID_ID;
    g_WhiteTexId = INVALID_ID;
    g_TextBoxTexId = INVALID_ID;

    g_EditMusicAudioId = INVALID_ID;
}

void Edit_Update(double elapsed_time){
    const float elapsedTime = static_cast<float>(elapsed_time);

    // フェードアウト完了後に選曲画面へ戻る
    if (g_EditFadeOutStarted && Fade_GetState() == FADE_STATE_FINISHED_OUT){
        Scene_Change(SCENE_MUSIC_SELEC);

        return;
    }

    // フェードアウト中は入力を受け付けない
    if (g_EditFadeOutStarted) return;

    if (g_SaveMessageTimer > 0.0f){
        g_SaveMessageTimer -= elapsedTime;

        if (g_SaveMessageTimer <= 0.0f){
            g_SaveMessageTimer = 0.0f;
            g_SaveResult = SaveResult::None;
        }
    }

    // 楽曲再生中はタイムラインを進める
    if (g_IsMusicPlaying){
        if (IsPlaying(g_EditMusicAudioId)){
            g_CurrentTime = GetAudioPlaybackTime(g_EditMusicAudioId);

        }else{
            // 曲末尾へ到達
            g_CurrentTime = GetAudioDuration(g_EditMusicAudioId);

            g_IsMusicPlaying = false;
        }
    }

    const XMFLOAT2 leftStick = PadLogger_GetLeftThumbStick(0);

    // スティックの現在状態
    const bool stickLeftNow =leftStick.x < -STICK_INPUT_THRESHOLD;
    const bool stickRightNow =leftStick.x > STICK_INPUT_THRESHOLD;
    const bool stickUpNow = leftStick.y > STICK_INPUT_THRESHOLD;
    const bool stickDownNow = leftStick.y < -STICK_INPUT_THRESHOLD;

    // スティックのトリガー
    const bool stickLeftTrigger = stickLeftNow && !g_StickLeftPrev;
    const bool stickRightTrigger = stickRightNow && !g_StickRightPrev;
    const bool stickUpTrigger = stickUpNow && !g_StickUpPrev;
    const bool stickDownTrigger = stickDownNow && !g_StickDownPrev;

    // 前フレーム状態を更新
    g_StickLeftPrev = stickLeftNow;
    g_StickRightPrev = stickRightNow;
    g_StickUpPrev = stickUpNow;
    g_StickDownPrev = stickDownNow;

    // 決定入力
    const bool decide =
        KeyLogger_IsTrigger(KK_ENTER) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_A);
    // キャンセル入力
    const bool cancel =
        KeyLogger_IsTrigger(KK_TAB) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_B);
    // 削除入力
    const bool removeNote =
        KeyLogger_IsTrigger(KK_DELETE) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_Y);
    // 作業の保存
    const bool saveChart =
        KeyLogger_IsTrigger(KK_LEFTSHIFT) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_START);
    // 曲の再生
    const bool togglePlayback =
        KeyLogger_IsTrigger(KK_RIGHTSHIFT) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_X);

    // 左入力
    const bool leftTriggered =
        KeyLogger_IsTrigger(KK_A) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ||
        stickLeftTrigger;
    const bool leftPressed =
        KeyLogger_IsPressed(KK_A) ||
        PadLogger_IsPressed(0, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ||
        stickLeftNow;

    // 右入力
    const bool rightTriggered =
        KeyLogger_IsTrigger(KK_D) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ||
        stickRightTrigger;
    const bool rightPressed =
        KeyLogger_IsPressed(KK_D) ||
        PadLogger_IsPressed(0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ||
        stickRightNow;

    // 左右は長押しリピート対応
    const bool moveLeft =
        UpdateInputRepeat(leftTriggered, leftPressed, g_LeftRepeatState, elapsedTime);
    const bool moveRight =
        UpdateInputRepeat(rightTriggered, rightPressed, g_RightRepeatState, elapsedTime);

    // 上下は1回押すごとに1回移動
    const bool moveUp =
        KeyLogger_IsTrigger(KK_W) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_DPAD_UP) ||
        stickUpTrigger;
    const bool moveDown =
        KeyLogger_IsTrigger(KK_S) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ||
        stickDownTrigger;

    // keyboardのノーツ種類の直接選択
    const bool selectNormal = KeyLogger_IsTrigger(KK_D1);
    const bool selectLong = KeyLogger_IsTrigger(KK_D2);
    const bool selectRapid = KeyLogger_IsTrigger(KK_D3);

    // padのノーツ種類の前後切り替え
    const bool previousNoteType = PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    const bool nextNoteType = PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

    // Long / Rapid配置中
    if (g_EditState == EditState::PlacingRangeNote){
        if (cancel){
            CancelRangeNotePlacement();

        }else if (decide){
            ConfirmRangeNotePlacement();

        }else if (moveLeft && !moveRight){
            MoveTimeline(-TIMELINE_TIME_STEP);

        }else if (moveRight && !moveLeft){
            MoveTimeline(TIMELINE_TIME_STEP);

        }

        return;
    }

    // 楽曲再生中
    if (g_IsMusicPlaying){
        if (cancel){
            StartReturnToMusicSelect();

        }else if (togglePlayback){
            StopMusicPlayback();

        }else if (decide){
            PlacePlaybackNormalNote();

        }else if (moveUp){
            MoveLaneSelection(-1);

        }else if (moveDown){
            MoveLaneSelection(1);

        }

        return;
    }

    // 通常編集状態
    if (cancel){
        StartReturnToMusicSelect();

    }else if (togglePlayback){
        ToggleMusicPlayback();

    }else if (saveChart){
        SaveCurrentChart();

    }else if (removeNote){
        RemoveNoteAtCurrentPosition();

    }else if (decide){
        PlaceSelectedNote();

    }else if (selectNormal){
        SetNoteType(EditNoteType::Normal);

    }else if (selectLong){
        SetNoteType(EditNoteType::Long);

    }else if (selectRapid){
        SetNoteType(EditNoteType::Rapid);

    }else if (previousNoteType){
        ChangeNoteType(-1);

    }else if (nextNoteType){
        ChangeNoteType(1);

    }else if (moveLeft && !moveRight){
        MoveTimeline(-TIMELINE_TIME_STEP);

    }else if (moveRight && !moveLeft){
        MoveTimeline(TIMELINE_TIME_STEP);

    }else if (moveUp){
        MoveLaneSelection(-1);

    }else if (moveDown){
        MoveLaneSelection(1);

    }
}

void Edit_Draw() {
    // 背景表示
    Sprite_Draw(
        g_BackgroundTexId,
        0.0f, 0.0f,
        g_ScreenX, g_ScreenY
    );

    // ノーツ表示ウィンドウの描画
    DrawNoteTypeWindow();

    // タイムラインの描画
    // 画像
    DrawTimelineBase();
    DrawSelectedLane();
    DrawTimelineMarks();
    // ノーツ
    DrawPlacedNotes();
    DrawNotePreview();
    DrawPendingRangeNote();
    // 中央カーソル
    DrawTimelineCursor();
    // 文字
    DrawNoteTypeText();
    DrawLaneLabels();
    DrawTimelineMarkLabels();
    DrawCurrentTime();
    //  再生中表示
    DrawPlaybackState();
    // 操作説明
    DrawControlGuide();
    // 保存結果
    DrawSaveResultText();
}

static bool UpdateInputRepeat(bool isTriggered, bool isPressed, InputRepeatState& repeatState, float elapsedTime){
    // 押した瞬間は即座に1回入力
    if (isTriggered){
        repeatState.timer = INPUT_REPEAT_START_DELAY;

        return true;
    }

    // 離している場合はリピート状態を解除
    if (!isPressed){
        repeatState.timer = 0.0f;

        return false;
    }

    // 長押し中の待機時間を減らす
    repeatState.timer -= elapsedTime;
    if (repeatState.timer > 0.0f){
        return false;
    }

    // 次のリピートまでの時間を設定
    repeatState.timer += INPUT_REPEAT_INTERVAL;

    return true;
}

static void MoveTimeline(float amount){
    g_CurrentTime += amount;

    g_CurrentTime = std::round(g_CurrentTime / TIMELINE_TIME_STEP) * TIMELINE_TIME_STEP;

    float minimumTime = TIMELINE_MIN_TIME;

    if (g_EditState == EditState::PlacingRangeNote){
        minimumTime = g_PendingRangeNote.startTime + RANGE_NOTE_MIN_DURATION;
    }

    if (g_CurrentTime < minimumTime){
        g_CurrentTime = minimumTime;
    }
}

static void MoveLaneSelection(int direction){
    int laneIndex = static_cast<int>(g_SelectedLane);

    laneIndex += direction;

    if (laneIndex < 0){
        laneIndex = LANE_COUNT - 1;
    } else if (laneIndex >= LANE_COUNT){
        laneIndex = 0;
    }

    g_SelectedLane = static_cast<EditLane>(laneIndex);

    PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));
}

static void DrawSelectedLane(){
    if (g_WhiteTexId == INVALID_ID) return;

    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;

    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;
    const float timelineHeight = g_ScreenY * TIMELINE_HEIGHT_RATE;

    const float laneHeight = timelineHeight / static_cast<float>(LANE_COUNT);

    const int selectedLaneIndex = static_cast<int>(g_SelectedLane);

    const float selectedLaneY = timelineY + laneHeight * static_cast<float>(selectedLaneIndex);

    Sprite_Draw(
        g_WhiteTexId,
        timelineX, selectedLaneY,
        timelineWidth,laneHeight,
        SELECTED_LANE_COLOR
    );
}

static void DrawLaneLabels(){
    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;

    const float timelineHeight = g_ScreenY * TIMELINE_HEIGHT_RATE;

    const float laneHeight = timelineHeight / static_cast<float>(LANE_COUNT);

    const wchar_t* laneNames[LANE_COUNT] =
    {
        L"UP",
        L"DOWN"
    };

    Text_Begin();

    for (int laneIndex = 0;laneIndex < LANE_COUNT;++laneIndex){
        const bool isSelected = laneIndex == static_cast<int>(g_SelectedLane);

        const XMVECTORF32& color =isSelected
            ? SELECTED_LANE_TEXT_COLOR
            : UNSELECTED_LANE_TEXT_COLOR;

        const float textX = timelineX + g_ScreenX * LANE_TEXT_X_OFFSET_RATE;
        const float textY = timelineY + laneHeight * static_cast<float>(laneIndex) +g_ScreenY * LANE_TEXT_Y_OFFSET_RATE;

        Text_Draw(
            laneNames[laneIndex],
            textX, textY,
            color,
            LANE_TEXT_SCALE
        );
    }

    Text_End();
}

static void ChangeNoteType(int direction){
    int noteTypeIndex = static_cast<int>(g_SelectedNoteType);

    noteTypeIndex += direction;

    if (noteTypeIndex < 0){
        noteTypeIndex = NOTE_TYPE_COUNT - 1;
    }else if (noteTypeIndex >= NOTE_TYPE_COUNT){
        noteTypeIndex = 0;
    }

    SetNoteType(static_cast<EditNoteType>(noteTypeIndex)
    );
}

static void SetNoteType(EditNoteType noteType){
    if (g_SelectedNoteType == noteType) return;

    g_SelectedNoteType = noteType;

    PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));
}

static void DrawNoteTypeWindow(){
    if (g_TextBoxTexId == INVALID_ID) return;

    Sprite_Draw(
        g_TextBoxTexId,
        g_ScreenX * NOTE_TYPE_BOX_X_RATE, g_ScreenY * NOTE_TYPE_BOX_Y_RATE,
        g_ScreenX * NOTE_TYPE_BOX_WIDTH_RATE, g_ScreenY * NOTE_TYPE_BOX_HEIGHT_RATE
    );
}

static void DrawNoteTypeText(){
    const wchar_t* noteTypeName = GetNoteTypeName(g_SelectedNoteType);

    const XMVECTORF32 noteTypeColor = GetNoteTypeColor( g_SelectedNoteType);

    Text_Begin();

    Text_Draw(
        L"NOTE TYPE",
        g_ScreenX * NOTE_TYPE_LABEL_X_RATE,
        g_ScreenY * NOTE_TYPE_LABEL_Y_RATE,
        NOTE_TYPE_LABEL_COLOR,
        NOTE_TYPE_LABEL_SCALE
    );

    Text_Draw(
        noteTypeName,
        g_ScreenX * NOTE_TYPE_NAME_X_RATE,
        g_ScreenY * NOTE_TYPE_NAME_Y_RATE,
        noteTypeColor,
        NOTE_TYPE_NAME_SCALE
    );

    Text_End();
}

static const wchar_t* GetNoteTypeName(EditNoteType noteType){
    switch (noteType)
    {
    case EditNoteType::Normal:
        return L"Normal";

    case EditNoteType::Long:
        return L"Long";

    case EditNoteType::Rapid:
        return L"Rapid";

    default:
        return L"Unknown";
    }
}

static XMVECTORF32 GetNoteTypeColor(EditNoteType noteType){
    switch (noteType)
    {
    case EditNoteType::Normal:
        return NOTE_TYPE_NORMAL_COLOR;

    case EditNoteType::Long:
        return NOTE_TYPE_LONG_COLOR;

    case EditNoteType::Rapid:
        return NOTE_TYPE_RAPID_COLOR;

    default:
        return NOTE_TYPE_LABEL_COLOR;
    }
}

static void PlaceSelectedNote(){
    if (g_EditState == EditState::PlacingRangeNote){
        ConfirmRangeNotePlacement();
        return;
    }

    switch (g_SelectedNoteType)
    {
    case EditNoteType::Normal:
        PlaceNormalNote();
        break;

    case EditNoteType::Long:
    case EditNoteType::Rapid:
        StartRangeNotePlacement();
        break;

    default:
        break;
    }
}

static void PlaceNormalNote(){
    const bool canPlace = CanPlaceNote(g_CurrentTime, 0.0f, g_SelectedLane);

    if (!canPlace){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    EditNoteData note;

    note.time = g_CurrentTime;
    note.lane = g_SelectedLane;
    note.type = EditNoteType::Normal;
    note.param = 0.0f;

    g_EditNotes.push_back(note);

    PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
}

static void PlacePlaybackNormalNote(){
    // 音声側から入力時点の最新時刻を取得
    const float playbackTime = GetAudioPlaybackTime(g_EditMusicAudioId);

    // 0.1秒単位へ丸める
    const float noteTime = std::round(playbackTime / TIMELINE_TIME_STEP) * TIMELINE_TIME_STEP;

    const bool canPlace = CanPlaceNote(noteTime, 0.0f, g_SelectedLane);

    if (!canPlace){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    EditNoteData note;

    note.time = noteTime;
    note.lane = g_SelectedLane;
    note.type = EditNoteType::Normal;
    note.param = 0.0f;

    g_EditNotes.push_back(note);
}

static int FindNoteIndexAtCurrentPosition(){
    for (int noteIndex = 0; noteIndex < static_cast<int>(g_EditNotes.size()); ++noteIndex){
        const EditNoteData& note = g_EditNotes[noteIndex];

        if (note.lane != g_SelectedLane){
            continue;
        }

        if (note.type == EditNoteType::Normal){
            const bool sameTime = std::fabs(note.time - g_CurrentTime) <= NOTE_TIME_EPSILON;

            if (sameTime){
                return noteIndex;
            }

            continue;
        }

        const float noteEndTime = note.time + note.param;

        const bool isInsideRange =
            g_CurrentTime >= note.time - NOTE_TIME_EPSILON &&
            g_CurrentTime <= noteEndTime + NOTE_TIME_EPSILON;

        if (isInsideRange){
            return noteIndex;
        }
    }

    return INVALID_NOTE_INDEX;
}

static void RemoveNoteAtCurrentPosition(){
    const int noteIndex = FindNoteIndexAtCurrentPosition();

    if (noteIndex == INVALID_NOTE_INDEX){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    g_EditNotes.erase(g_EditNotes.begin() + noteIndex);

    PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
}

static void DrawPlacedNotes(){
    if (g_WhiteTexId == INVALID_ID) return;

    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;

    const float noteWidth = g_ScreenX * EDIT_NOTE_WIDTH_RATE;
    const float noteHeight = g_ScreenY * EDIT_NOTE_HEIGHT_RATE;

    for (const EditNoteData& note : g_EditNotes){
        if (note.type == EditNoteType::Long || note.type == EditNoteType::Rapid){
            DrawRangeNote(note.time, note.param, note.lane, GetPlacedNoteColor(note.type));

            continue;
        }

        const float noteX = GetTimelineXFromTime(note.time);

        if (noteX + noteWidth * 0.5f < timelineX || noteX - noteWidth * 0.5f > timelineX + timelineWidth){
            continue;
        }

        const float noteY = GetLaneCenterY(note.lane);

        Sprite_Draw(
            g_WhiteTexId,
            noteX - noteWidth * 0.5f, noteY - noteHeight * 0.5f,
            noteWidth, noteHeight,
            NORMAL_NOTE_COLOR
        );
    }
}

static void DrawNotePreview(){
    if (g_WhiteTexId == INVALID_ID) return;

    if (g_EditState == EditState::PlacingRangeNote) return;

    const float noteX = GetTimelineXFromTime( g_CurrentTime);
    const float noteY = GetLaneCenterY(g_SelectedLane);

    const float noteWidth = g_ScreenX * EDIT_NOTE_WIDTH_RATE;
    const float noteHeight = g_ScreenY * EDIT_NOTE_HEIGHT_RATE;

    // 再生中は必ずNormalとして扱う
    const EditNoteType previewType =
        g_IsMusicPlaying
        ? EditNoteType::Normal
        : g_SelectedNoteType;

    float previewDuration = 0.0f;

    if (previewType == EditNoteType::Long || previewType == EditNoteType::Rapid){
        previewDuration = RANGE_NOTE_MIN_DURATION;
    }

    const bool canPlace = CanPlaceNote(g_CurrentTime, previewDuration, g_SelectedLane);

    XMFLOAT4 previewColor = canPlace
        ? GetPlacedNoteColor(previewType)
        : INVALID_NOTE_PREVIEW_COLOR;

    if (canPlace){
        previewColor.w = NOTE_PREVIEW_ALPHA;
    }

    Sprite_Draw(
        g_WhiteTexId,
        noteX - noteWidth * 0.5f, noteY - noteHeight * 0.5f,
        noteWidth, noteHeight,
        previewColor
    );
}

static void StartRangeNotePlacement(){
    const float startTime = g_CurrentTime;
    const bool canStart = CanPlaceNote(startTime, RANGE_NOTE_MIN_DURATION, g_SelectedLane);

    if (!canStart){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    g_PendingRangeNote.startTime = startTime;
    g_PendingRangeNote.endTime = startTime + RANGE_NOTE_MIN_DURATION;
    g_PendingRangeNote.lane = g_SelectedLane;
    g_PendingRangeNote.type = g_SelectedNoteType;
    g_EditState = EditState::PlacingRangeNote;

    g_CurrentTime = g_PendingRangeNote.endTime;

    PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
}

static void ConfirmRangeNotePlacement(){
    const float duration = g_CurrentTime - g_PendingRangeNote.startTime;

    if (duration < RANGE_NOTE_MIN_DURATION){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    const bool canPlace = CanPlaceNote(g_PendingRangeNote.startTime, duration, g_PendingRangeNote.lane);

    if (!canPlace){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    g_PendingRangeNote.endTime = g_CurrentTime;

    PlaceRangeNote();

    g_PendingRangeNote = {};
    g_EditState = EditState::Timeline;
}

static void CancelRangeNotePlacement(){
    g_CurrentTime = g_PendingRangeNote.startTime;

    g_PendingRangeNote = {};

    g_EditState = EditState::Timeline;

    PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));
}

static void PlaceRangeNote(){
    EditNoteData note;

    note.time = g_PendingRangeNote.startTime;
    note.lane = g_PendingRangeNote.lane;
    note.type = g_PendingRangeNote.type;
    note.param = g_PendingRangeNote.endTime - g_PendingRangeNote.startTime;
    // 0.1秒単位へ丸める
    note.time = std::round(note.time / TIMELINE_TIME_STEP) * TIMELINE_TIME_STEP;
    note.param = std::round(note.param / TIMELINE_TIME_STEP) * TIMELINE_TIME_STEP;

    g_EditNotes.push_back(note);

    PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
}

static void DrawRangeNote(float startTime, float duration, EditLane lane, const XMFLOAT4& color){
    if (g_WhiteTexId == INVALID_ID) return;

    const float startX = GetTimelineXFromTime(startTime);
    const float endX = GetTimelineXFromTime(startTime + duration);

    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;
    const float timelineRight = timelineX + timelineWidth;

    const float visibleStartX = std::max(startX, timelineX);
    const float visibleEndX = std::min(endX, timelineRight);

    if (visibleEndX <= visibleStartX){
        return;
    }

    const float noteY = GetLaneCenterY(lane);
    const float noteHeight = g_ScreenY * EDIT_NOTE_HEIGHT_RATE;

    Sprite_Draw(
        g_WhiteTexId,
        visibleStartX, noteY - noteHeight * 0.5f,
        visibleEndX - visibleStartX, noteHeight,
        color
    );
}

static void DrawPendingRangeNote(){
    if (g_EditState != EditState::PlacingRangeNote) return;

    const float duration = g_CurrentTime - g_PendingRangeNote.startTime;

    const bool canPlace = CanPlaceNote(g_PendingRangeNote.startTime, duration, g_PendingRangeNote.lane);

    XMFLOAT4 color = canPlace
        ? GetPlacedNoteColor(g_PendingRangeNote.type)
        : INVALID_NOTE_PREVIEW_COLOR;

    if (canPlace){
        color.w = NOTE_PREVIEW_ALPHA;
    }

    DrawRangeNote(g_PendingRangeNote.startTime, duration, g_PendingRangeNote.lane, color);
}

static float GetNoteEndTime(const EditNoteData& note){
    if (note.type == EditNoteType::Normal){
        return note.time;
    }

    return note.time + note.param;
}

static bool DoNoteRangesOverlap(float firstStartTime, float firstEndTime, float secondStartTime, float secondEndTime){
    return 
        firstStartTime <= secondEndTime + NOTE_TIME_EPSILON &&
        firstEndTime >= secondStartTime - NOTE_TIME_EPSILON;
}

static bool CanPlaceNote(float startTime, float duration, EditLane lane){
    const float endTime = startTime + duration;

    for (const EditNoteData& note : g_EditNotes){
        if (note.lane != lane){
            continue;
        }

        const float existingStartTime = note.time;
        const float existingEndTime = GetNoteEndTime(note);

        if (DoNoteRangesOverlap(startTime, endTime, existingStartTime, existingEndTime)){
            return false;
        }
    }

    return true;
}

static XMFLOAT4 GetPlacedNoteColor(EditNoteType noteType)
{
    switch (noteType)
    {
    case EditNoteType::Normal:
        return NORMAL_NOTE_COLOR;

    case EditNoteType::Long:
        return LONG_NOTE_COLOR;

    case EditNoteType::Rapid:
        return RAPID_NOTE_COLOR;

    default:
        return NORMAL_NOTE_COLOR;
    }
}

static void StartReturnToMusicSelect(){
    if (g_EditFadeOutStarted) return;

    if (g_IsMusicPlaying){
        StopMusicPlayback();
    }

    g_EditFadeOutStarted = true;

    PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE), false);

    Fade_Start(FADE_OUT_DURATION, true);
}

static void ToggleMusicPlayback(){
    if (g_IsMusicPlaying){
        StopMusicPlayback();

    }else{
        StartMusicPlayback();

    }
}

static void StartMusicPlayback(){
    if (g_EditMusicAudioId == INVALID_ID){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    const float musicDuration = GetAudioDuration(g_EditMusicAudioId);

    if (musicDuration <= 0.0f){
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    // 曲末尾にいる場合は0秒へ戻す
    if (g_CurrentTime >= musicDuration){
        g_CurrentTime = TIMELINE_MIN_TIME;
    }

    const bool started = PlayAudioFromTime(g_EditMusicAudioId, g_CurrentTime, false);

    if (!started){
        g_IsMusicPlaying = false;

        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    g_IsMusicPlaying = true;
}

static void StopMusicPlayback(){
    if (g_EditMusicAudioId != INVALID_ID){
        if (g_IsMusicPlaying){
            g_CurrentTime = GetAudioPlaybackTime(g_EditMusicAudioId);
        }

        StopAudio(g_EditMusicAudioId);
    }

    g_IsMusicPlaying = false;
}

static void DrawPlaybackState(){
    Text_Begin();

    if (g_IsMusicPlaying){
        Text_Draw(
            L"PLAYING",
            g_ScreenX * PLAY_STATE_X_RATE,
            g_ScreenY * PLAY_STATE_Y_RATE,
            PLAYING_TEXT_COLOR,
            PLAY_STATE_SCALE
        );
    }else{
        Text_Draw(
            L"STOPPED",
            g_ScreenX * PLAY_STATE_X_RATE,
            g_ScreenY * PLAY_STATE_Y_RATE,
            STOPPED_TEXT_COLOR,
            PLAY_STATE_SCALE
        );
    }

    Text_End();
}

static float GetTimelineXFromTime(float noteTime){
    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;

    const float secondsToPixel = timelineWidth / TIMELINE_VISIBLE_SECONDS;
    const float relativeTime = noteTime - g_CurrentTime;

    return timelineX + timelineWidth * 0.5f + relativeTime * secondsToPixel;
}

static float GetLaneCenterY(EditLane lane){
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;
    const float timelineHeight = g_ScreenY * TIMELINE_HEIGHT_RATE;

    const float laneHeight = timelineHeight / static_cast<float>(LANE_COUNT);

    const int laneIndex = static_cast<int>(lane);

    return timelineY + laneHeight * static_cast<float>(laneIndex) + laneHeight * 0.5f;
}

static void DrawCurrentTime(){
    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;

    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;

    const float timelineCenterX = timelineX + timelineWidth * 0.5f;

    wchar_t timeText[TIME_TEXT_BUFFER_SIZE];
    swprintf_s(
        timeText,
        L"%.2f sec",
        g_CurrentTime
    );

    Text_Begin();

    Text_Draw(
        timeText,
        timelineCenterX - g_ScreenX * TIME_TEXT_X_OFFSET_RATE,
        timelineY - g_ScreenY * TIME_TEXT_Y_OFFSET_RATE,
        TIME_TEXT_COLOR,
        TIME_TEXT_SCALE
    );

    Text_End();
}

static void DrawTimelineBase(){
    if (g_WhiteTexId == INVALID_ID) return;

    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;

    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;
    const float timelineHeight = g_ScreenY * TIMELINE_HEIGHT_RATE;

    Sprite_Draw(
        g_WhiteTexId,
        timelineX, timelineY,
        timelineWidth, timelineHeight,
        TIMELINE_BACKGROUND_COLOR
    );

    const float dividerHeight = g_ScreenY * LANE_DIVIDER_HEIGHT_RATE;
    const float dividerY = timelineY + timelineHeight * 0.5f - dividerHeight * 0.5f;

    Sprite_Draw(
        g_WhiteTexId,
        timelineX, dividerY,
        timelineWidth, dividerHeight,
        TIMELINE_DIVIDER_COLOR
    );
}

static void DrawTimelineCursor(){
    if (g_WhiteTexId == INVALID_ID) return;

    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;

    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;
    const float timelineHeight = g_ScreenY * TIMELINE_HEIGHT_RATE;

    const float cursorWidth = g_ScreenX * TIMELINE_CURSOR_WIDTH_RATE;
    const float cursorX = timelineX + timelineWidth * 0.5f - cursorWidth * 0.5f;

    Sprite_Draw(
        g_WhiteTexId,
        cursorX, timelineY,
        cursorWidth, timelineHeight,
        TIMELINE_CURSOR_COLOR
    );
}

static void DrawTimelineMarks(){
    if (g_WhiteTexId == INVALID_ID) return;


    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;
    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;

    const float secondsToPixel = timelineWidth / TIMELINE_VISIBLE_SECONDS;

    const float visibleStartTime = g_CurrentTime - TIMELINE_HALF_SECONDS;
    const float visibleEndTime = g_CurrentTime + TIMELINE_HALF_SECONDS;

    const int firstMarkIndex = static_cast<int>(std::floor(visibleStartTime / TIMELINE_MARK_INTERVAL));
    const int lastMarkIndex = static_cast<int>(std::ceil(visibleEndTime / TIMELINE_MARK_INTERVAL));

    const float markWidth = g_ScreenX * TIMELINE_MARK_WIDTH_RATE;
    const float markHeight = g_ScreenY * TIMELINE_MARK_HEIGHT_RATE;

    for (int markIndex = firstMarkIndex; markIndex <= lastMarkIndex; ++markIndex){
        const float markTime = static_cast<float>(markIndex) * TIMELINE_MARK_INTERVAL;

        if (markTime < TIMELINE_MIN_TIME){
            continue;
        }

        const float relativeTime = markTime - g_CurrentTime;

        const float markX =  timelineX + timelineWidth * 0.5f + relativeTime * secondsToPixel;

        if (markX < timelineX || markX > timelineX + timelineWidth){
            continue;
        }

        Sprite_Draw(
            g_WhiteTexId,
            markX - markWidth * 0.5f, timelineY,
            markWidth, markHeight,
            TIMELINE_MARK_COLOR
        );
    }
}

static void DrawTimelineMarkLabels(){
    const float timelineX = g_ScreenX * TIMELINE_X_RATE;
    const float timelineY = g_ScreenY * TIMELINE_Y_RATE;
    const float timelineWidth = g_ScreenX * TIMELINE_WIDTH_RATE;

    const float secondsToPixel = timelineWidth / TIMELINE_VISIBLE_SECONDS;

    const float visibleStartTime = g_CurrentTime - TIMELINE_HALF_SECONDS;
    const float visibleEndTime = g_CurrentTime + TIMELINE_HALF_SECONDS;

    const int firstMarkIndex = static_cast<int>(std::floor(visibleStartTime / TIMELINE_MARK_INTERVAL));
    const int lastMarkIndex = static_cast<int>(std::ceil(visibleEndTime / TIMELINE_MARK_INTERVAL));

    Text_Begin();

    for (int markIndex = firstMarkIndex; markIndex <= lastMarkIndex; ++markIndex){
        const float markTime = static_cast<float>(markIndex) *TIMELINE_MARK_INTERVAL;

        if (markTime < TIMELINE_MIN_TIME){
            continue;
        }

        const float relativeTime = markTime - g_CurrentTime;
        const float markX = timelineX + timelineWidth * 0.5f + relativeTime * secondsToPixel;

        if (markX < timelineX || markX > timelineX + timelineWidth){
            continue;
        }

        wchar_t markText[MARK_TEXT_BUFFER_SIZE];

        swprintf_s(
            markText,
            L"%.0f",
            markTime
        );

        Text_Draw(
            markText,
            markX - g_ScreenX * MARK_TEXT_X_OFFSET_RATE,
            timelineY - g_ScreenY * MARK_TEXT_Y_OFFSET_RATE,
            MARK_TEXT_COLOR,
            MARK_TEXT_SCALE
        );
    }

    Text_End();
}

static void DrawControlGuide(){
    const wchar_t* guideLine1 = L"";
    const wchar_t* guideLine2 = L"";
    const wchar_t* guideLine3 = L"";

    if (!PadLogger_IsConnected()){
        // Keyboard
        guideLine1 =
            L"[A / D] MOVE TIME    "
            L"[W / S] SELECT LANE";
        guideLine2 =
            L"[1 / 2 / 3] NOTE TYPE    "
            L"[ENTER] PLACE / CONFIRM";
        guideLine3 =
            L"[DELETE] DELETE    "
            L"[R-SHIFT] PLAY / STOP    "
            L"[L-SHIFT] SAVE    "
            L"[TAB] BACK / CANCEL";
        if (g_IsMusicPlaying){
            guideLine1 =
                L"[W / S] SELECT LANE    "
                L"[ENTER] PLACE NORMAL";
            guideLine2 =
                L"[R-SHIFT] STOP PLAYBACK";
            guideLine3 =
                L"[TAB] BACK";
        }

    }else{
        switch (PadLogger_GetControllerDisplayType())
        {
        case ControllerDisplayType::PlayStation:
            guideLine1 =
                L"[L STICK / D-PAD LEFT-RIGHT] MOVE TIME    "
                L"[UP-DOWN] SELECT LANE";
            guideLine2 =
                L"[L1 / R1] NOTE TYPE    "
                L"[CROSS] PLACE / CONFIRM";
            guideLine3 =
                L"[TRIANGLE] DELETE    "
                L"[SQUARE] PLAY / STOP    "
                L"[OPTIONS] SAVE    "
                L"[CIRCLE] BACK / CANCEL";           
            if (g_IsMusicPlaying){
                guideLine1 =
                    L"[D-PAD UP / DOWN] SELECT LANE    "
                    L"[CROSS] PLACE NORMAL";
                guideLine2 = L"[SQUARE] STOP PLAYBACK";
                guideLine3 = L"[CIRCLE] BACK";
            }
            break;

        case ControllerDisplayType::Xbox:
        case ControllerDisplayType::Unknown:
        default:
            guideLine1 =
                L"[L STICK / D-PAD LEFT-RIGHT] MOVE TIME    "
                L"[UP-DOWN] SELECT LANE";
            guideLine2 =
                L"[LB / RB] NOTE TYPE    "
                L"[A] PLACE / CONFIRM";
            guideLine3 =
                L"[Y] DELETE    "
                L"[X] PLAY / STOP    "
                L"[MENU] SAVE    "
                L"[B] BACK / CANCEL";           
            if (g_IsMusicPlaying) {
                guideLine1 =
                    L"[D-PAD UP / DOWN] SELECT LANE    "
                    L"[A] PLACE NORMAL";
                guideLine2 = L"[X] STOP PLAYBACK";
                guideLine3 = L"[B] BACK";
            }


            break;
        }
    }

    Text_Begin();

    Text_Draw(
        guideLine1,
        g_ScreenX * GUIDE_TEXT_X_RATE,
        g_ScreenY * GUIDE_TEXT_LINE_1_Y_RATE,
        GUIDE_TEXT_COLOR,
        GUIDE_TEXT_SCALE
    );

    Text_Draw(
        guideLine2,
        g_ScreenX * GUIDE_TEXT_X_RATE,
        g_ScreenY * GUIDE_TEXT_LINE_2_Y_RATE,
        GUIDE_TEXT_COLOR,
        GUIDE_TEXT_SCALE
    );

    Text_Draw(
        guideLine3,
        g_ScreenX * GUIDE_TEXT_X_RATE,
        g_ScreenY * GUIDE_TEXT_LINE_3_Y_RATE,
        GUIDE_TEXT_COLOR,
        GUIDE_TEXT_SCALE
    );

    Text_End();
}

static bool SaveChartToCsv(const char* filePath)
{
    std::vector<EditNoteData> sortedNotes =
        g_EditNotes;

    std::sort(
        sortedNotes.begin(),
        sortedNotes.end(),
        [](const EditNoteData & left, const EditNoteData & right)
        {
            if (std::fabs(left.time - right.time) > NOTE_TIME_EPSILON){
                return left.time < right.time;
            }

            return static_cast<int>(left.lane) < static_cast<int>(right.lane);
        }
    );

    std::ofstream outputFile(filePath);

    if (!outputFile.is_open()){
        return false;
    }

    outputFile
        << "time,lane,type,param\n";

    outputFile
        << std::fixed
        << std::setprecision(SCORE_VALUE_DECIMAL_DIGITS);

    for (const EditNoteData& note : sortedNotes)
    {
        outputFile
            << note.time
            << ","
            << static_cast<int>(note.lane)
            << ","
            << static_cast<int>(note.type)
            << ","
            << note.param
            << "\n";
    }

    outputFile.close();

    return !outputFile.fail();
}

static void SaveCurrentChart(){
    if (g_EditState == EditState::PlacingRangeNote){
        g_SaveResult = SaveResult::Failed;

        g_SaveMessageTimer = SAVE_MESSAGE_DISPLAY_TIME;

        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

        return;
    }

    const bool saved = SaveChartToCsv(EDIT_SCORE_OUTPUT_PATH);

    if (saved){
        g_SaveResult = SaveResult::Success;

        PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
    }else{
        g_SaveResult = SaveResult::Failed;

        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));
    }

    g_SaveMessageTimer = SAVE_MESSAGE_DISPLAY_TIME;
}

static void DrawSaveResultText(){
    if (g_SaveResult == SaveResult::None || g_SaveMessageTimer <= 0.0f) return;

    const wchar_t* message = L"";
    XMVECTORF32 color = SAVE_SUCCESS_COLOR;

    switch (g_SaveResult)
    {
    case SaveResult::Success:
        message = L"CHART SAVED";
        color = SAVE_SUCCESS_COLOR;
        break;

    case SaveResult::Failed:
        message = L"SAVE FAILED";
        color = SAVE_FAILED_COLOR;
        break;

    case SaveResult::None:
    default:
        return;
    }

    Text_Begin();

    Text_Draw(
        message,
        g_ScreenX * SAVE_MESSAGE_X_RATE, g_ScreenY * SAVE_MESSAGE_Y_RATE,
        color,
        SAVE_MESSAGE_SCALE
    );

    Text_End();
}