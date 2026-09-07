/*
    選曲画面の制御：Music_select.cpp

    2026/05/27	hibiki sakuma
*/
#include "Music_select.h"
#include "resource_manager.h"
#include "text_manager.h"
#include "ui_text_manager.h"
#include "play_record_manager.h"
#include "sprite.h"
#include "Key_logger.h"
#include "pad_logger.h"
#include "fade.h"
#include "scene.h"
#include "Audio.h"
#include "ui.h"
#include "direct3d.h"
#include "Configu.h"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <locale>
#include <cwchar>
#include <cmath>
#include <string>
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

// 定数宣言
// 共通値
static constexpr int INVALID_ID = -1;

static constexpr int INITIAL_SELECT_INDEX = 0;

static constexpr float COLOR_MAX = 1.0f;

// フェード
static constexpr float FADE_IN_DURATION = 1.0f;
static constexpr float FADE_OUT_DURATION = 0.5f;

// 入力
static constexpr float STICK_MENU_THRESHOLD = 0.60f;

// 選曲リール
static constexpr float SCROLL_DURATION = 0.18f;

static constexpr int DRAW_ORDER[] = { -2,-1,0,1,2 };

static constexpr int ORDER_MAX =
static_cast<int>(sizeof(DRAW_ORDER) / sizeof(DRAW_ORDER[0]));

static constexpr float REEL_CENTER_X_RATE = 0.50f;
static constexpr float REEL_CENTER_Y_RATE = 0.35f;

static constexpr float REEL_STEP_X_RATE = 0.07f;
static constexpr float REEL_STEP_Y_RATE = 0.13f;

static constexpr float REEL_BOX_WIDTH_RATE = 0.80f;
static constexpr float REEL_BOX_HEIGHT_RATE = 0.15f;

static constexpr float REEL_SCALE_REDUCTION = 0.12f;
static constexpr float REEL_MIN_SCALE = 0.55f;

static constexpr float REEL_ALPHA_REDUCTION = 0.25f;
static constexpr float REEL_MIN_ALPHA = 0.35f;

static constexpr float REEL_TEXT_OFFSET_X = 100.0f;
static constexpr float REEL_TEXT_OFFSET_Y = 15.0f;

// 説明欄
static constexpr float INFORMATION_BOX_X_RATE = 0.075f;
static constexpr float INFORMATION_BOX_Y_RATE = 0.68f;
static constexpr float INFORMATION_BOX_WIDTH_RATE = 0.85f;
static constexpr float INFORMATION_BOX_HEIGHT_RATE = 0.30f;

static constexpr float MENU_TITLE_X_RATE = 0.10f;
static constexpr float MENU_TITLE_Y_RATE = 0.71f;
static constexpr float MENU_TITLE_SCALE = 1.10f;

static constexpr float MENU_DESCRIPTION_X_RATE = 0.10f;
static constexpr float MENU_DESCRIPTION_Y_RATE = 0.77f;
static constexpr float MENU_DESCRIPTION_SCALE = 0.90f;

static constexpr float SONG_TITLE_X_RATE = 0.10f;
static constexpr float SONG_TITLE_Y_RATE = 0.70f;
static constexpr float SONG_TITLE_SCALE = 1.10f;

static constexpr float SONG_DESCRIPTION_X_RATE = 0.10f;
static constexpr float SONG_DESCRIPTION_Y_RATE = 0.755f;
static constexpr float SONG_DESCRIPTION_SCALE = 0.80f;

// 非楽曲項目のアイコン
static constexpr float MENU_ICON_CENTER_X_RATE = 0.825f;
static constexpr float MENU_ICON_CENTER_Y_RATE = 0.825f;
static constexpr float MENU_ICON_SIZE_RATE = 0.24f;

// 難易度別情報
static constexpr float ROW_START_Y_RATE = 0.81f;
static constexpr float ROW_SPACING_RATE = 0.041f;

static constexpr float DIFFICULTY_X_RATE = 0.10f;
static constexpr float LEVEL_LABEL_X_RATE = 0.19f;
static constexpr float SCORE_LABEL_X_RATE = 0.29f;
static constexpr float SCORE_VALUE_X_RATE = 0.39f;
static constexpr float RANK_LABEL_X_RATE = 0.53f;
static constexpr float RANK_VALUE_X_RATE = 0.61f;

static constexpr float DIFFICULTY_TEXT_SCALE = 0.75f;

static constexpr int LEVEL_TEXT_BUFFER_SIZE = 16;
static constexpr int SCORE_TEXT_BUFFER_SIZE = 32;

// 難易度選択カーソル
static constexpr float DIFFICULTY_CURSOR_X_RATE = 0.10f;
static constexpr float DIFFICULTY_CURSOR_WIDTH_RATE = 0.58f;
static constexpr float DIFFICULTY_CURSOR_Y_OFFSET_RATE = 0.006f;
static constexpr float DIFFICULTY_CURSOR_HEIGHT_RATE = 0.036f;
static constexpr float DIFFICULTY_CURSOR_ALPHA = 0.35f;

static constexpr float DIFFICULTY_CURSOR_RED = 0.20f;
static constexpr float DIFFICULTY_CURSOR_GREEN = 0.75f;
static constexpr float DIFFICULTY_CURSOR_BLUE = 1.00f;

// エディットモード選択
static constexpr float EDIT_SELECT_START_Y_RATE = 0.82f;
static constexpr float EDIT_SELECT_ROW_SPACING_RATE = 0.06f;

static constexpr float EDIT_SELECT_TEXT_X_RATE = 0.12f;
static constexpr float EDIT_SELECT_CURSOR_X_RATE = 0.10f;
static constexpr float EDIT_SELECT_CURSOR_WIDTH_RATE = 0.42f;
static constexpr float EDIT_SELECT_CURSOR_HEIGHT_RATE = 0.05f;

static constexpr float EDIT_SELECT_TEXT_SCALE = 0.85f;
static constexpr float EDIT_SELECT_CURSOR_ALPHA = 0.35f;
static constexpr int EDIT_SONG_MAX_SCORE = 10000;

// 操作ガイド
static constexpr float GUIDE_TEXT_X_RATE = 0.08f;
static constexpr float GUIDE_TEXT_Y_RATE = 0.94f;
static constexpr float GUIDE_TEXT_SCALE = 0.70f;

// プレビュー再生
static constexpr float PREVIEW_WAIT_DURATION = 0.35f;

// 色
static constexpr XMVECTORF32 DIFFICULTY_COLOR_NORMAL{ 1.0f,1.0f,1.0f,1.0f };
static constexpr XMVECTORF32 DIFFICULTY_COLOR_SELECTED{ 1.0f, 1.0f, 0.0f, 1.0f };
static constexpr XMVECTORF32 DIFFICULTY_COLOR_NOT_CLEARED{ 0.45f, 0.45f, 0.45f, 1.0f };
static constexpr XMVECTORF32 MENU_TITLE_COLOR{ 1.0f,1.0f,0.0f,1.0f };
static constexpr XMVECTORF32 NORMAL_TEXT_COLOR{ 1.0f,1.0f,1.0f,1.0f };
static constexpr XMVECTORF32 GUIDE_TEXT_COLOR{ 0.0f, 0.0f, 0.0f, 1.0f };


// CSV共通
static constexpr int CSV_HEADER_LINE = 1;
static constexpr int SONG_TEXT_DESCRIPTION_LINE = 2;

static constexpr int SONG_MASTER_COLUMN_COUNT = 5;
static constexpr int SONG_TEXT_COLUMN_COUNT = 3;
static constexpr int SONG_DIFFICULTY_COLUMN_COUNT = 6;

// song_master.csv列
static constexpr int SONG_MASTER_SONG_ID_COLUMN = 0;
static constexpr int SONG_MASTER_TITLE_COLUMN = 1;
static constexpr int SONG_MASTER_JACKET_COLUMN = 2;
static constexpr int SONG_MASTER_BGM_COLUMN = 3;
static constexpr int SONG_MASTER_ENEMY_COLUMN = 4;

// song_text.csv列
static constexpr int SONG_TEXT_SONG_ID_COLUMN = 0;
static constexpr int SONG_TEXT_LANGUAGE_COLUMN = 1;
static constexpr int SONG_TEXT_DESCRIPTION_COLUMN = 2;

// song_difficulty.csv列
static constexpr int SONG_DIFFICULTY_SONG_ID_COLUMN = 0;
static constexpr int SONG_DIFFICULTY_NAME_COLUMN = 1;
static constexpr int SONG_DIFFICULTY_SCORE_ID_COLUMN = 2;
static constexpr int SONG_DIFFICULTY_LEVEL_COLUMN = 3;
static constexpr int SONG_DIFFICULTY_AVAILABLE_COLUMN = 4;
static constexpr int SONG_DIFFICULTY_MAX_SCORE_COLUMN = 5;

// データ型
struct SongDifficultyData
{
    GameDifficulty difficulty = GameDifficulty::Easy;

    int scoreId = INVALID_ID;

    int level = 0;

    bool isAvailable = false;

    int maxScore = 0;
};


struct SongData
{
    std::string songId;

    // 曲名は全言語で英語固定
    std::wstring title;

    // 現在言語に対応する説明文
    std::wstring description;

    // 将来のジャケット差し替え用
    int jacketTexId = INVALID_ID;

    // 将来の曲別BGM・プレビュー用
    int bgmAudioId = INVALID_ID;

    EnemyType enemyType = EnemyType::None;

    SongDifficultyData difficulties[ static_cast<int>(GameDifficulty::Max)];
};

enum class SelectItemType
{
    Tutorial,
    Song,
    Setting,
    Edit
};

struct SelectItem
{
    SelectItemType type =  SelectItemType::Song;

    // Songの場合だけ使用
    int songIndex = INVALID_ID;

    int texId = INVALID_ID;
    int iconTexId = INVALID_ID;

    // Tutorial、Setting、Edit用
    std::wstring displayName;
    std::wstring description;
};

enum class EditSelectChoice
{
    PlayChart = 0,
    EditChart,

    Max
};

enum class MusicSelectState
{
    ItemSelect,
    DifficultySelect,
    EditSelect,
    Transition
};

// 変数宣言
// リール表示
static float g_SelectBox_SizeX = static_cast<float>(INVALID_ID);
static float g_SelectBox_SizeY = static_cast<float>(INVALID_ID);
static int g_SelectIndex = INITIAL_SELECT_INDEX;

static bool g_IsScrolling = false;
static float g_ScrollTimer = 0.0f;

static float g_ReelOffset = 0.0f;
static float g_StartOffset = 0.0f;

// プレビュー再生
static int g_PreviewAudioId = INVALID_ID;
static int g_PreviewSongIndex = INVALID_ID;
static float g_PreviewWaitTimer = 0.0f;

// 入力状態
static bool g_StickUpPrev = false;
static bool g_StickDownPrev = false;

// 選択状態
static MusicSelectState g_SelectState = MusicSelectState::ItemSelect;
static GameDifficulty g_SelectedDifficulty = GameDifficulty::Easy;
static int g_SelectedSongIndex = INVALID_ID;

// エディット項目内の選択
static EditSelectChoice g_SelectedEditChoice = EditSelectChoice::PlayChart;
// エディット譜面
static constexpr const char* EDIT_SCORE_PATH = "resource/data/score_data/score_edit.csv";

// 画面間の引き渡し
// 画面間の引き渡し
static int g_SelectedScoreId = INVALID_ID;
static int g_SelectedBgmAudioId = INVALID_ID;
static EnemyType g_Select_EnemyType = EnemyType::None;

// 読込データ
static std::vector<SongData> g_Songs;
static std::vector<SelectItem> g_SelectItems;

// 内部関数
// リール・入力制御
static int WrapIndex(int index);

static void MoveNext();
static void MovePrev();

static float EaseOutCubic(float t);

// 決定処理
static void DecideCurrentItem();
static void DecideDifficulty();

// 表示文字取得
static const wchar_t* GetItemDisplayName(const SelectItem& item);
static const wchar_t* GetDifficultyName(GameDifficulty difficulty);
static const wchar_t* GetRankName(Rank_Type rank);

// プレビュー再生
static void StopSongPreview();

// エディット選択
static void DecideEditChoice();
static bool DoesEditChartExist();
static const wchar_t* GetEditChoiceName(EditSelectChoice choice);

// エディット選択描画
static void DrawEditSelectInformation(const SelectItem& item, float screenWidth, float screenHeight);
static void DrawEditSelectCursor(float screenWidth, float screenHeight);

// 描画
static void DrawSongInformation(const SelectItem& item, float screenWidth, float screenHeight);
static void DrawMenuInformation(const SelectItem& item, float screenWidth, float screenHeight);
static void DrawDifficultyCursor(float screenWidth, float screenHeight);

// CSV共通
static std::vector<std::string> SplitCSVLine(const std::string& line);
static void TrimLineEnd(std::string& value);
static std::wstring Utf8ToWstring(const std::string& value);
static void RemoveUtf8Bom(std::string& value);

// CSV読込
static bool LoadSongMasterCSV();
static bool LoadSongTextCSV();
static bool LoadSongDifficultyCSV();

// 項目生成
static void BuildSelectItems();

// 検索・変換
static int FindSongIndex(const std::string& songId);
static GameDifficulty ParseDifficulty(const std::string& value);
static EnemyType ParseEnemyType(const std::string& value);
static int ParseScoreId(const std::string& value);
static int ParseJacketTextureId(const std::string& value);
static int ParseBgmAudioId(const std::string& value);

void Music_Select_Initialize(){
    // 画面遷移のフェードイン
    Fade_Start(FADE_IN_DURATION, false);

    const float h = static_cast<float>(Direct3D_GetBackBufferHeight());
    const float w = static_cast<float>(Direct3D_GetBackBufferWidth());

    // リール項目サイズ
    g_SelectBox_SizeX = w * REEL_BOX_WIDTH_RATE;

    g_SelectBox_SizeY = h * REEL_BOX_HEIGHT_RATE;

    // リール状態
    g_SelectIndex = INITIAL_SELECT_INDEX;

    g_IsScrolling = false;
    g_ScrollTimer = 0.0f;

    g_ReelOffset = 0.0f;
    g_StartOffset = 0.0f;

    // 入力状態
    g_StickUpPrev = false;
    g_StickDownPrev = false;

    // データ
    g_Songs.clear();
    g_SelectItems.clear();

    // 選択状態
    g_SelectState = MusicSelectState::ItemSelect;
    g_SelectedDifficulty = GameDifficulty::Easy;
    g_SelectedSongIndex = INVALID_ID;
    
    // エディット項目の初期化
    g_SelectedEditChoice = EditSelectChoice::PlayChart;
    
    // 引き渡し情報
    g_SelectedScoreId = INVALID_ID;
    g_SelectedBgmAudioId = INVALID_ID;
    g_Select_EnemyType = EnemyType::None;

    // 楽曲CSV読込
    const bool masterLoaded = LoadSongMasterCSV();

    if (masterLoaded){
        LoadSongTextCSV();
        LoadSongDifficultyCSV();
    }

    // CSVからリール項目を生成
    BuildSelectItems();

    // 選曲BGM
    PlayAudio(Resouce_Manager_GetAudioId(Select_BGM),true);
}

void Music_Select_Finalize(){
    StopSongPreview();

    StopAudio(Resouce_Manager_GetAudioId(Select_BGM));
}

void Music_Select_Update(double elapsed_time){
    if (g_SelectItems.empty())
        return;

    const bool decide = 
        KeyLogger_IsTrigger(KK_ENTER) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_A);
    const bool cancel =
        KeyLogger_IsTrigger(KK_TAB) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_B);

    const XMFLOAT2 leftStick = PadLogger_GetLeftThumbStick(0);

    const bool stickUpNow = leftStick.y > STICK_MENU_THRESHOLD;
    const bool stickDownNow = leftStick.y < -STICK_MENU_THRESHOLD;

    const bool stickUpTrigger = stickUpNow && !g_StickUpPrev;
    const bool stickDownTrigger = stickDownNow && !g_StickDownPrev;

    g_StickUpPrev = stickUpNow;
    g_StickDownPrev = stickDownNow;

    const bool moveUp = KeyLogger_IsTrigger(KK_W) || stickUpTrigger;
    const bool moveDown = KeyLogger_IsTrigger(KK_S) || stickDownTrigger;

    switch (g_SelectState)
    {
    case MusicSelectState::ItemSelect:
        if (moveUp) { MovePrev(); }
        else if (moveDown) { MoveNext(); }
        else if (decide && !g_IsScrolling) { DecideCurrentItem(); }
        
        if (cancel){
            PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));
            Scene_Change(SCENE_TITLE);
            return;
        }
        break;

        case MusicSelectState::DifficultySelect:
            if (moveUp){
                PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));
                const int current = static_cast<int>(g_SelectedDifficulty);

                int next = current - 1;

                if (next < 0){
                    next = static_cast<int>(GameDifficulty::Max) - 1;
                }

                g_SelectedDifficulty = static_cast<GameDifficulty >(next);
            }

            if (moveDown){
                PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));
                const int count = static_cast<int>(GameDifficulty::Max);

                const int next = (static_cast<int>(g_SelectedDifficulty) + 1) % count;

                g_SelectedDifficulty = static_cast<GameDifficulty >(next);
            }

            if (decide){
                DecideDifficulty();
            }

            if (cancel){
                PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));
                g_SelectState = MusicSelectState::ItemSelect;
                g_SelectedSongIndex = INVALID_ID;
            }
            break;

        case MusicSelectState::EditSelect:
        {
            if (moveUp){
                PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));

                int current = static_cast<int>(g_SelectedEditChoice);

                --current;

                if (current < 0){
                    current = static_cast<int>(EditSelectChoice::Max) - 1;
                }

                g_SelectedEditChoice = static_cast<EditSelectChoice>(current);

            }else if (moveDown){
                PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));

                const int count = static_cast<int>(EditSelectChoice::Max);
                const int next = (static_cast<int>(g_SelectedEditChoice) + 1) % count;

                g_SelectedEditChoice = static_cast<EditSelectChoice>(next);

            }else if (decide){
                DecideEditChoice();

            }else if (cancel){
                PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

                g_SelectState = MusicSelectState::ItemSelect;
            }

            break;
        }

        case MusicSelectState::Transition:
            if (Fade_GetState() == FADE_STATE_FINISHED_OUT){
                Scene_Change(SCENE_CHARACTER_SELECT);

                return;
            }
            break;
    }

    if (g_IsScrolling){
        g_ScrollTimer += static_cast<float>(elapsed_time);

        float t = g_ScrollTimer / SCROLL_DURATION;

        if (t >= 1.0f){
            g_IsScrolling = false;
            g_ReelOffset = 0.0f;
        } else {
            const float ease = EaseOutCubic(t);

            g_ReelOffset = g_StartOffset * (1.0f - ease);
        }
    }

    if (!g_IsScrolling && g_SelectState != MusicSelectState::Transition){
        const SelectItem& current = g_SelectItems[g_SelectIndex];

        if (current.type == SelectItemType::Song &&
            current.songIndex >= 0 &&
            current.songIndex < static_cast<int>(g_Songs.size())){

            if (g_PreviewSongIndex != current.songIndex){
                g_PreviewWaitTimer += static_cast<float>(elapsed_time);

                if (g_PreviewWaitTimer >= PREVIEW_WAIT_DURATION) {
                    const SongData& song = g_Songs[current.songIndex];

                    if (song.bgmAudioId != INVALID_ID){
                        StopAudio(Resouce_Manager_GetAudioId(Select_BGM));

                        PlayAudio(song.bgmAudioId, true);

                        g_PreviewAudioId = song.bgmAudioId;
                        g_PreviewSongIndex = current.songIndex;
                    }

                    g_PreviewWaitTimer = 0.0f;
                }
            }
        }else{
            if (g_PreviewAudioId != INVALID_ID){
                StopSongPreview();

                PlayAudio(Resouce_Manager_GetAudioId(Select_BGM), true);
            }
        }
    }
}

void Music_Select_Draw() {
    if (g_SelectItems.empty())
        return;

    float h = (float)Direct3D_GetBackBufferHeight();
    float w = (float)Direct3D_GetBackBufferWidth();

    // 背景
    Sprite_Draw(
        Resouce_Manager_GetTexId(Select_Back),
        0.0f, 0.0f,
        w, h
    );

    // 選曲リール表示
    const float centerX = w * REEL_CENTER_X_RATE;
    const float centerY = h * REEL_CENTER_Y_RATE;

    const float stepX = w * REEL_STEP_X_RATE;
    const float stepY = h * REEL_STEP_Y_RATE;

    for (int order = 0; order < ORDER_MAX; order++) {
        int relIndex = DRAW_ORDER[order];

        int index = WrapIndex(g_SelectIndex + relIndex);

        float visualOffset = (float)relIndex + g_ReelOffset;

        float absOffset = fabsf(visualOffset);

        const float scale =
            std::max(REEL_MIN_SCALE, COLOR_MAX - absOffset * REEL_SCALE_REDUCTION);

        const float alpha =
            std::max(REEL_MIN_ALPHA, COLOR_MAX - absOffset * REEL_ALPHA_REDUCTION);

        float x = centerX - visualOffset * stepX;
        float y = centerY + visualOffset * stepY;

        float sizeX = g_SelectBox_SizeX * scale;
        float sizeY = g_SelectBox_SizeY * scale;

        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, alpha };

        Sprite_Draw(
            g_SelectItems[index].texId,
            x - sizeX * 0.5f, y - sizeY * 0.5f,
            sizeX, sizeY,
            color
        );
    }

    // 文字描画開始
    Text_Begin();

    for (int order = 0; order < ORDER_MAX; order++) {
        int relIndex = DRAW_ORDER[order];

        int index = WrapIndex(g_SelectIndex + relIndex);

        float visualOffset = (float)relIndex + g_ReelOffset;

        float x = centerX - visualOffset * stepX;
        float y = centerY + visualOffset * stepY;

        const SelectItem& item = g_SelectItems[index];

        Text_Draw(
            GetItemDisplayName(item),
            x - REEL_TEXT_OFFSET_X, y - REEL_TEXT_OFFSET_Y
        );
    }

    Text_End();

    // 説明欄
    const SelectItem& current = g_SelectItems[g_SelectIndex];
    // 説明用ウィンドウ
    Sprite_Draw(
        Resouce_Manager_GetTexId(TextBox),
        w * INFORMATION_BOX_X_RATE ,    h * INFORMATION_BOX_Y_RATE,
        w * INFORMATION_BOX_WIDTH_RATE, h * INFORMATION_BOX_HEIGHT_RATE
    );

    // 楽曲以外の項目だけアイコンを表示
    if (current.type != SelectItemType::Song && current.iconTexId >= 0){
        const float iconX = w * MENU_ICON_CENTER_X_RATE;
        const float iconY = h * MENU_ICON_CENTER_Y_RATE;
        const float iconSize = h * MENU_ICON_SIZE_RATE;

        Sprite_Draw(
            current.iconTexId,
            iconX - iconSize * 0.5f, iconY - iconSize * 0.5f,
            iconSize, iconSize,
            XMFLOAT4{1.0f,1.0f,1.0f,1.0f
            }
        );
    }

    // 楽曲項目ではジャケット画像を表示
    if (current.type == SelectItemType::Song){
        if (current.songIndex >= 0 && current.songIndex < static_cast<int>(g_Songs.size())){
            const SongData& song = g_Songs[current.songIndex];

            if (song.jacketTexId != INVALID_ID){
                const float jacketCenterX = w * MENU_ICON_CENTER_X_RATE;
                const float jacketCenterY = h * MENU_ICON_CENTER_Y_RATE;

                const float jacketSize = h * MENU_ICON_SIZE_RATE;

                Sprite_Draw(
                    song.jacketTexId,
                    jacketCenterX - jacketSize * 0.5f, jacketCenterY - jacketSize * 0.5f,
                    jacketSize, jacketSize
                );
            }
        }
    }

    // カーソル画像の描画
    if (current.type == SelectItemType::Song) {
        DrawDifficultyCursor(w, h);
    
    }else if (current.type == SelectItemType::Edit) {
        DrawEditSelectCursor(w, h);

    }

    //文字描画
    Text_Begin();

    if (current.type == SelectItemType::Song){
        DrawSongInformation(current, w, h);

    }else if (current.type == SelectItemType::Edit && g_SelectState == MusicSelectState::EditSelect){
        DrawEditSelectInformation(current, w, h);

    }else{
        DrawMenuInformation(current, w, h);
    }

    // 操作方法
    const wchar_t* guideText = nullptr;
    if (PadLogger_IsConnected()){
        switch (PadLogger_GetControllerDisplayType())
        {
        case ControllerDisplayType::PlayStation:
            guideText = g_SelectState == MusicSelectState::DifficultySelect
                ? L"[L STICK] DIFFICULTY  [X] DECIDE  [O] BACK"
                : L"[L STICK] SELECT  [X] DECIDE  [O] BACK";
            break;

        case ControllerDisplayType::Xbox:
        case ControllerDisplayType::Unknown:
        default:
            guideText = g_SelectState == MusicSelectState::DifficultySelect
                ? L"[L STICK] DIFFICULTY  [A] DECIDE  [B] BACK"
                : L"[L STICK] SELECT  [A] DECIDE  [B] BACK";
            break;
        }
    }else{
        guideText = g_SelectState == MusicSelectState::
            DifficultySelect
            ? L"[W/S] DIFFICULTY  [ENTER] DECIDE  [TAB] BACK"
            : L"[W/S] SELECT  [ENTER] DECIDE  [TAB] BACK";
    }
    Text_Draw(
        guideText,
        w* GUIDE_TEXT_X_RATE, h* GUIDE_TEXT_Y_RATE,
        GUIDE_TEXT_COLOR,
        GUIDE_TEXT_SCALE
    );

    Text_End();
}

int Music_Select_GetId(){
    return g_SelectedScoreId;
}

int Music_Select_GetMaxScore(){
    if (g_SelectedSongIndex < 0 ||
        g_SelectedSongIndex >= static_cast<int>(g_Songs.size())){
        return 0;
    }

    if (g_SelectedDifficulty == GameDifficulty::Edit) { // エディットモードの際は仮の最大スコアを入れる
        return EDIT_SONG_MAX_SCORE;
    }

    const int difficultyIndex = static_cast<int>(g_SelectedDifficulty);

    return g_Songs[g_SelectedSongIndex].difficulties[difficultyIndex].maxScore;
}

int Music_Select_GetBgmAudioId(){
    return g_SelectedBgmAudioId;
}

EnemyType Music_Select_GetEnemyId() {
    return g_Select_EnemyType;
}

GameDifficulty Music_Select_GetDifficulty(){
    return g_SelectedDifficulty;
}

const std::string& Music_Select_GetSongId(){
    static const std::string EMPTY_SONG_ID;

    if (g_SelectedSongIndex < 0 || g_SelectedSongIndex >= static_cast<int>(g_Songs.size())){
        return EMPTY_SONG_ID;
    }

    return g_Songs[g_SelectedSongIndex].songId;
}

static int WrapIndex(int index){
    const int size = static_cast<int>(g_SelectItems.size());

    if (size <= 0) return 0;

    index %= size;

    if (index < 0) index += size;

    return index;
}

static void MoveNext(){
    if (g_IsScrolling) return;

    StopSongPreview(); // 移動中はプレビュー停止

    PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));
    g_SelectIndex = WrapIndex(g_SelectIndex + 1);

    g_StartOffset = 1.0f;
    g_ReelOffset = g_StartOffset;

    g_ScrollTimer = 0.0f;
    g_IsScrolling = true;
}

static void MovePrev(){
    if (g_IsScrolling) return;

    StopSongPreview(); // 移動中はプレビュー停止


    PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));
    g_SelectIndex = WrapIndex(g_SelectIndex - 1);

    g_StartOffset = -1.0f;
    g_ReelOffset = g_StartOffset;

    g_ScrollTimer = 0.0f;
    g_IsScrolling = true;
}

static float EaseOutCubic(float t){
    float p = 1.0f - t;
    return 1.0f - p * p * p;
}

static void DecideCurrentItem(){
    const SelectItem& current = g_SelectItems[g_SelectIndex];
    PlayAudio(Resouce_Manager_GetAudioId(MusicSelect_Decide_SE));

    switch (current.type)
    {
    case SelectItemType::Song:
        g_SelectedSongIndex = current.songIndex;

        g_SelectedDifficulty = GameDifficulty::Easy;

        g_SelectState = MusicSelectState::DifficultySelect;
        break;

    case SelectItemType::Tutorial:
        Scene_Change(SCENE_TUTORIAL);
        break;

    case SelectItemType::Setting:
        Scene_Change(SCENE_CONFIGU);
        break;

    case SelectItemType::Edit:
        g_SelectedEditChoice = EditSelectChoice::PlayChart;

        g_SelectState = MusicSelectState::EditSelect;
        break;
    }
}

static void DecideDifficulty() {
    if (g_SelectedSongIndex < 0 || g_SelectedSongIndex >= static_cast<int>(g_Songs.size()))
        return;

    const SongData& song = g_Songs[g_SelectedSongIndex];

    const int difficultyIndex = static_cast<int>(g_SelectedDifficulty);

    const SongDifficultyData& difficulty = song.difficulties[difficultyIndex];

    if (!difficulty.isAvailable) {
     //失敗音　予定
        return;
    }

    StopSongPreview();

    PlayAudio(Resouce_Manager_GetAudioId(MusicSelect_Decide_SE));

    g_SelectedScoreId = difficulty.scoreId;
    g_SelectedBgmAudioId = song.bgmAudioId;
    g_Select_EnemyType = song.enemyType;
    g_SelectState = MusicSelectState::Transition;

    Fade_Start(FADE_OUT_DURATION, true);
}

static const wchar_t* GetItemDisplayName(const SelectItem& item){
    if (item.type == SelectItemType::Song){
        if (item.songIndex >= 0 && item.songIndex < static_cast<int>(g_Songs.size())){
            return g_Songs[item.songIndex].title.c_str();
        }

        return L"UNKNOWN SONG";
    }
    return item.displayName.c_str();
}

static const wchar_t* GetDifficultyName(GameDifficulty difficulty){
    switch (difficulty)
    {
    case GameDifficulty::Easy:
        return L"EASY";

    case GameDifficulty::Normal:
        return L"NORMAL";

    case GameDifficulty::Hard:
        return L"HARD";

    case GameDifficulty::Max:
    default:
        return L"UNKNOWN";
    }
}

static const wchar_t* GetRankName(Rank_Type rank){
    switch (rank)
    {
    case Rank_S:
        return L"S";

    case Rank_A:
        return L"A";

    case Rank_B:
        return L"B";

    case Rank_C:
        return L"C";

    case Rank_D:
        return L"D";

    default:
        return L"-";
    }
}

static void StopSongPreview(){
    if (g_PreviewAudioId != INVALID_ID){
        StopAudio(g_PreviewAudioId);
    }

    g_PreviewAudioId = INVALID_ID;
    g_PreviewSongIndex = INVALID_ID;
    g_PreviewWaitTimer = 0.0f;
}

static void DecideEditChoice(){
    switch (g_SelectedEditChoice)
    {
    case EditSelectChoice::PlayChart:
    {
        if (!DoesEditChartExist()){
            PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

            return;
        }

        PlayAudio(Resouce_Manager_GetAudioId(MusicSelect_Decide_SE));

        // エディット譜面用ID
        g_SelectedScoreId = SCORE_EDIT;

        // ゲーム側で必要になる値を設定
        g_SelectedDifficulty = GameDifficulty::Edit;

        // エディットモード用の曲
        g_SelectedBgmAudioId = Resouce_Manager_GetAudioId(Edit_Game_BGM);

        // エディット用に使用する敵
        g_Select_EnemyType = EnemyType::Tutorial;

        g_SelectState = MusicSelectState::Transition;

        // resource_manager側で再読み込み
        Resouce_Manager_Reload_EditScoreData();

        Fade_Start(FADE_OUT_DURATION, true);

        break;
    }

    case EditSelectChoice::EditChart:
        PlayAudio(Resouce_Manager_GetAudioId(MusicSelect_Decide_SE));

        // 選曲BGMを停止
        StopAudio(Resouce_Manager_GetAudioId(Select_BGM));

        Scene_Change(SCENE_EDIT);
        break;

    case EditSelectChoice::Max:
    default:
        PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));
        break;
    }
}

static bool DoesEditChartExist(){
    std::ifstream file(EDIT_SCORE_PATH, std::ios::binary);

    if (!file){
        return false;
    }

    std::string line;
    int validLineCount = 0;

    while (std::getline(file, line)){
        TrimLineEnd(line);

        if (line.empty()){
            continue;
        }

        ++validLineCount;
    }

    // 1行目はヘッダーなので、2行以上あればノーツデータが存在する
    return validLineCount >= 2;
}

static const wchar_t* GetEditChoiceName(EditSelectChoice choice){
    switch (choice)
    {
    case EditSelectChoice::PlayChart:
        return L"PLAY CHART";

    case EditSelectChoice::EditChart:
        return L"EDIT CHART";

    case EditSelectChoice::Max:
    default:
        return L"UNKNOWN";
    }
}

static void DrawEditSelectCursor(float screenWidth, float screenHeight){
    if (g_SelectState != MusicSelectState::EditSelect){
        return;
    }

    const int choiceIndex = static_cast<int>(g_SelectedEditChoice);
    const float rowY =  screenHeight * (EDIT_SELECT_START_Y_RATE + EDIT_SELECT_ROW_SPACING_RATE * static_cast<float>(choiceIndex));

    Sprite_Draw(
        Resouce_Manager_GetTexId(Color_White),
        screenWidth * EDIT_SELECT_CURSOR_X_RATE    , rowY,
        screenWidth * EDIT_SELECT_CURSOR_WIDTH_RATE, screenHeight *  EDIT_SELECT_CURSOR_HEIGHT_RATE,
        XMFLOAT4{
            DIFFICULTY_CURSOR_RED,
            DIFFICULTY_CURSOR_GREEN,
            DIFFICULTY_CURSOR_BLUE,
            EDIT_SELECT_CURSOR_ALPHA
        }
    );
}

static void DrawEditSelectInformation(const SelectItem& item, float screenWidth, float screenHeight){
    // 項目名
    Text_Draw(
        item.displayName.c_str(),
        screenWidth *
        MENU_TITLE_X_RATE,
        screenHeight *
        MENU_TITLE_Y_RATE,
        MENU_TITLE_COLOR,
        MENU_TITLE_SCALE
    );

    // 短い説明
    Text_Draw(
        item.description.c_str(),
        screenWidth * MENU_DESCRIPTION_X_RATE,
        screenHeight * MENU_DESCRIPTION_Y_RATE,
        NORMAL_TEXT_COLOR,
        MENU_DESCRIPTION_SCALE
    );

    const bool chartExists = DoesEditChartExist();

    for (int index = 0; index < static_cast<int>(EditSelectChoice::Max); ++index){
        const EditSelectChoice choice = static_cast<EditSelectChoice>(index);

        const bool isSelected = choice == g_SelectedEditChoice;

        XMVECTORF32 color = isSelected
            ? DIFFICULTY_COLOR_SELECTED
            : DIFFICULTY_COLOR_NORMAL;

        // 譜面がない場合はPLAY CHARTだけ暗く表示
        if (choice == EditSelectChoice::PlayChart && !chartExists && !isSelected){
            color = DIFFICULTY_COLOR_NOT_CLEARED;
        }

        const float rowY = screenHeight * (EDIT_SELECT_START_Y_RATE + EDIT_SELECT_ROW_SPACING_RATE * static_cast<float>(index));

        Text_Draw(
            GetEditChoiceName(choice),
            screenWidth * EDIT_SELECT_TEXT_X_RATE, 
            rowY,
            color,
            EDIT_SELECT_TEXT_SCALE
        );
    }
}

static void DrawMenuInformation(const SelectItem& item, float screenWidth, float screenHeight){
    // 項目名
    Text_Draw(
        item.displayName.c_str(),
        screenWidth * MENU_TITLE_X_RATE,
        screenHeight * MENU_TITLE_Y_RATE,
        MENU_TITLE_COLOR,
        MENU_TITLE_SCALE
    );

    // 説明文
    Text_Draw(
        item.description.c_str(),
        screenWidth * MENU_DESCRIPTION_X_RATE,
        screenHeight * MENU_DESCRIPTION_Y_RATE,
        NORMAL_TEXT_COLOR,
        MENU_DESCRIPTION_SCALE
    );
}

static void DrawDifficultyCursor(float screenWidth, float screenHeight){
    // 難易度選択中だけ表示する
    if (g_SelectState != MusicSelectState::DifficultySelect)
        return;


    const int difficultyIndex = static_cast<int>(g_SelectedDifficulty);

    // 選択難易度に対応する行のY座標
    const float rowY = 
        screenHeight * (ROW_START_Y_RATE + ROW_SPACING_RATE * static_cast<float>(difficultyIndex));

    const float cursorX = screenWidth * DIFFICULTY_CURSOR_X_RATE;
    const float cursorY = rowY - screenHeight * DIFFICULTY_CURSOR_Y_OFFSET_RATE;

    const float cursorWidth = screenWidth * DIFFICULTY_CURSOR_WIDTH_RATE;
    const float cursorHeight = screenHeight * DIFFICULTY_CURSOR_HEIGHT_RATE;

    const XMFLOAT4 cursorColor = {
        DIFFICULTY_CURSOR_RED,
        DIFFICULTY_CURSOR_GREEN,
        DIFFICULTY_CURSOR_BLUE,
        DIFFICULTY_CURSOR_ALPHA
    };

    Sprite_Draw(
        Resouce_Manager_GetTexId(Color_White),
        cursorX, cursorY,
        cursorWidth, cursorHeight,
        cursorColor
    );
}


static void DrawSongInformation(const SelectItem& item, float screenWidth, float screenHeight){
    const SongData& song = g_Songs[item.songIndex];

    // 曲名
    Text_Draw(
        song.title.c_str(),
        screenWidth *  SONG_TITLE_X_RATE,
        screenHeight * SONG_TITLE_Y_RATE,
        MENU_TITLE_COLOR,
        SONG_TITLE_SCALE
    );

    // 曲説明
    Text_Draw(
        song.description.c_str(),
        screenWidth * SONG_DESCRIPTION_X_RATE,
        screenHeight * SONG_DESCRIPTION_Y_RATE,
        NORMAL_TEXT_COLOR,
        SONG_DESCRIPTION_SCALE
    );

    // 難易度別記録
    for (int i = 0; i < static_cast<int>(GameDifficulty::Max); ++i){
        const GameDifficulty difficulty = static_cast<GameDifficulty>(i);

        const SongDifficultyData& difficultyData = song.difficulties[i];

        const float rowY = screenHeight *( ROW_START_Y_RATE + ROW_SPACING_RATE * static_cast<float>(i));

        const bool isSelected =
            g_SelectState == MusicSelectState::DifficultySelect &&
            g_SelectedDifficulty == difficulty;

        const DifficultyRecord* record = PlayRecord_Get(song.songId, difficulty);

        const bool hasCleared = record != nullptr && record->hasCleared;

        XMVECTORF32 rowColor = DIFFICULTY_COLOR_NORMAL;

        if (!hasCleared){
            rowColor = DIFFICULTY_COLOR_NOT_CLEARED;
        }

        if (isSelected){
            rowColor = DIFFICULTY_COLOR_SELECTED;
        }

        // 難易度
        Text_Draw(
            GetDifficultyName(difficulty),
            screenWidth * DIFFICULTY_X_RATE,
            rowY,
            rowColor,
            DIFFICULTY_TEXT_SCALE
        );

        // レベル
        wchar_t levelText[LEVEL_TEXT_BUFFER_SIZE]{};
            swprintf_s(
                levelText,
                L"Lv.%d",
                difficultyData.level
            );

            Text_Draw(
                levelText,
                screenWidth * LEVEL_LABEL_X_RATE,
                rowY,
                rowColor,
                DIFFICULTY_TEXT_SCALE
            );

            // SCORE
            Text_Draw(
                L"SCORE",
                screenWidth * SCORE_LABEL_X_RATE,
                rowY,
                rowColor,
                DIFFICULTY_TEXT_SCALE
            );

            // RANK
            Text_Draw(
                L"RANK",
                screenWidth * RANK_LABEL_X_RATE,
                rowY,
                rowColor,
                DIFFICULTY_TEXT_SCALE
            );

            if (record != nullptr && record->hasPlayed){
                wchar_t scoreText[SCORE_TEXT_BUFFER_SIZE]{};

                    swprintf_s(
                        scoreText,
                        L"%d",
                        record->highScore
                    );

                    Text_Draw(
                        scoreText,
                        screenWidth * SCORE_VALUE_X_RATE,
                        rowY,
                        rowColor,
                        DIFFICULTY_TEXT_SCALE
                    );

                    Text_Draw(
                        GetRankName(record->highRank),
                        screenWidth * RANK_VALUE_X_RATE,
                        rowY,
                        rowColor,
                        DIFFICULTY_TEXT_SCALE
                    );
            } else {
                Text_Draw(
                    L"------",
                    screenWidth * SCORE_VALUE_X_RATE,
                    rowY,
                    rowColor,
                    DIFFICULTY_TEXT_SCALE
                );

                Text_Draw(
                    L"-",
                    screenWidth *  RANK_VALUE_X_RATE,
                    rowY,
                    rowColor,
                    DIFFICULTY_TEXT_SCALE
                );
            }
    }
}
static std::vector<std::string> SplitCSVLine(const std::string& line){
    std::vector<std::string> result;
    std::string field;

    bool insideQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i){
        const char character = line[i];

        if (character == '"'){
            // ""はフィールド内の引用符
            if (insideQuotes && i + 1 < line.size() && line[i + 1] == '"'){
                field.push_back('"');
                ++i;
            }else{
                insideQuotes = !insideQuotes;
            }
        } else if (character == ',' && !insideQuotes){
            result.push_back(field);
            field.clear();
        }else{
            field.push_back(character);
        }
    }

    result.push_back(field);

    return result;
}

static void TrimLineEnd(std::string& value){
    while (!value.empty())
    {
        const char last = value.back();

        if (last == '\r' ||
            last == '\n' ||
            last == ' ' ||
            last == '\t')
        {
            value.pop_back();
        } else {
            break;
        }
    }
}

static std::wstring Utf8ToWstring(const std::string& value) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(value);
}

static void RemoveUtf8Bom(std::string& value){
    if (value.size() >= 3 && 
        static_cast<unsigned char>(value[0]) == 0xEF &&
        static_cast<unsigned char>(value[1]) == 0xBB &&
        static_cast<unsigned char>(value[2]) == 0xBF)
    {
        value.erase(0, 3);
    }
}

static bool LoadSongMasterCSV(){
    const char* path =
        Resouce_Manager_GetDataFilePath(DataFileID::Music_Select_song_master);

    if (path == nullptr) return false;

    std::ifstream file(path, std::ios::binary);

    if (!file) return false;

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line))
    {
        ++lineNumber;

        // 1行目はヘッダー
        if (lineNumber == CSV_HEADER_LINE){
            RemoveUtf8Bom(line);
            continue;
        }

        if (line.empty()){
            continue;
        }

        std::vector<std::string> columns = SplitCSVLine(line);

        for (std::string& column : columns){
            TrimLineEnd(column);
        }

        if (columns.size() != SONG_MASTER_COLUMN_COUNT){
            continue;
        }

        SongData song;

        song.songId = columns[SONG_MASTER_SONG_ID_COLUMN];
        song.title = Utf8ToWstring(columns[SONG_MASTER_TITLE_COLUMN]);
        song.jacketTexId = ParseJacketTextureId(columns[SONG_MASTER_JACKET_COLUMN]);
        song.bgmAudioId = ParseBgmAudioId(columns[SONG_MASTER_BGM_COLUMN]);
        song.enemyType = ParseEnemyType(columns[SONG_MASTER_ENEMY_COLUMN]);

        g_Songs.push_back(song);
    }

    return !g_Songs.empty();
}

static bool LoadSongTextCSV(){
    const char* path =
        Resouce_Manager_GetDataFilePath(DataFileID::Music_Select_song_text);

    if (path == nullptr) return false;

    std::ifstream file(path, std::ios::binary);

    if (!file) return false;

    const std::string currentLanguage = Configu_GetLanguageCode();

    std::string line;
    int lineNumber = 0;
    bool textFound = false;

    while (std::getline(file, line)){
        ++lineNumber;

        // 1行目はヘッダー
        if (lineNumber == CSV_HEADER_LINE){
            RemoveUtf8Bom(line);
            continue;
        }

        // 2行目に日本語の列説明をスキップ
        if (lineNumber == SONG_TEXT_DESCRIPTION_LINE){
            continue;
        }

        if (line.empty()){
            continue;
        }

        std::vector<std::string> columns = SplitCSVLine(line);

        for (std::string& column : columns){
            TrimLineEnd(column);
        }

        if (columns.size() != SONG_TEXT_COLUMN_COUNT){
            continue;
        }

        const std::string& songId = columns[SONG_TEXT_SONG_ID_COLUMN];
        const std::string& language = columns[SONG_TEXT_LANGUAGE_COLUMN];

        if (language != currentLanguage){
            continue;
        }

        const int songIndex = FindSongIndex(songId);

        if (songIndex < 0){
            continue;
        }

        g_Songs[songIndex].description = Utf8ToWstring(columns[SONG_TEXT_DESCRIPTION_COLUMN]);

        textFound = true;
    }

    return textFound;
}

static bool LoadSongDifficultyCSV(){
    const char* path =
        Resouce_Manager_GetDataFilePath(DataFileID::Music_Select_song_difficulty);

    if (path == nullptr) return false;

    std::ifstream file(path, std::ios::binary);

    if (!file) return false;

    std::string line;
    int lineNumber = 0;
    bool difficultyFound = false;

    while (std::getline(file, line)){
        ++lineNumber;

        // 1行目はヘッダー
        if (lineNumber == CSV_HEADER_LINE){
            RemoveUtf8Bom(line);
            continue;
        }

        if (line.empty()){
            continue;
        }

        std::vector<std::string> columns = SplitCSVLine(line);

        for (std::string& column : columns){
            TrimLineEnd(column);
        }

        if (columns.size() != SONG_DIFFICULTY_COLUMN_COUNT){
            continue;
        }

        const int songIndex = FindSongIndex(columns[SONG_DIFFICULTY_SONG_ID_COLUMN]);

        if (songIndex < 0){
            continue;
        }

        const GameDifficulty difficulty = ParseDifficulty(columns[SONG_DIFFICULTY_NAME_COLUMN]);

        if (difficulty == GameDifficulty::Max){
            continue;
        }

        const int difficultyIndex = static_cast<int>(difficulty);

        SongDifficultyData data;

        data.difficulty = difficulty;
        data.scoreId = ParseScoreId(columns[SONG_DIFFICULTY_SCORE_ID_COLUMN]);
        try
        {
            data.level = std::stoi(columns[SONG_DIFFICULTY_LEVEL_COLUMN]);
            data.isAvailable = std::stoi(columns[SONG_DIFFICULTY_AVAILABLE_COLUMN]) != 0;
            data.maxScore = std::stoi(columns[SONG_DIFFICULTY_MAX_SCORE_COLUMN]);
        }
        catch (const std::exception&)
        {
            continue;
        }

        // 譜面IDを取得できなかった場合は選択不能
        if (data.scoreId < 0){
            data.isAvailable = false;
        }

        g_Songs[songIndex].difficulties[difficultyIndex] = data;

        difficultyFound = true;
    }

    return difficultyFound;
}

static void BuildSelectItems(){
    g_SelectItems.clear();

    // Tutorial
    g_SelectItems.push_back(
        {
            SelectItemType::Tutorial,
            INVALID_ID,

            Resouce_Manager_GetTexId(TextBox),
            Resouce_Manager_GetTexId(Enemy_Tutorial),

            UI_Text_Get("MUSIC_SELECT_TUTORIAL_TITLE"),
            UI_Text_Get("MUSIC_SELECT_TUTORIAL_DESC")
        });

    // Setting
    g_SelectItems.push_back(
        {
            SelectItemType::Setting,
            INVALID_ID,

            Resouce_Manager_GetTexId(TextBox),
            Resouce_Manager_GetTexId(Configu_Icon),

            UI_Text_Get("MUSIC_SELECT_SETTING_TITLE"),

            UI_Text_Get("MUSIC_SELECT_SETTING_DESC")
        });

    // Edit
    g_SelectItems.push_back(
        {
            SelectItemType::Edit,
            INVALID_ID,

            Resouce_Manager_GetTexId(TextBox),
            Resouce_Manager_GetTexId(Edit_Icon),

            UI_Text_Get("MUSIC_SELECT_EDIT_TITLE"),
            UI_Text_Get("MUSIC_SELECT_EDIT_DESC")
        });

    // 楽曲
    for (int songIndex = 0; songIndex <static_cast<int>(g_Songs.size()); ++songIndex){
        SelectItem songItem;

        songItem.type = SelectItemType::Song;
        songItem.songIndex = songIndex;
        songItem.texId = Resouce_Manager_GetTexId(TextBox);
        songItem.iconTexId = INVALID_ID;
        g_SelectItems.push_back(songItem);
    }
}

static int FindSongIndex(const std::string& songId){
    for (int i = 0; i < static_cast<int>(g_Songs.size()); ++i){
        if (g_Songs[i].songId == songId){
            return i;
        }
    }

    return INVALID_ID;
}

static GameDifficulty ParseDifficulty(const std::string& value) {
    if (value == "EASY") return GameDifficulty::Easy;

    if (value == "NORMAL") return GameDifficulty::Normal;

    if (value == "HARD")  return GameDifficulty::Hard;

    return GameDifficulty::Max;
}

static EnemyType ParseEnemyType(const std::string& value){
    if (value == "Enemy_Song001") return EnemyType::Song001;

    if (value == "Enemy_Song002") return EnemyType::Song002;

    if (value == "Enemy_Song003") return EnemyType::Song003;

    if (value == "Tutorial") return EnemyType::Tutorial;

    return EnemyType::None;
}

static int ParseScoreId(const std::string& value){
    if (value == "SCORE_SONG001_EASY"){
        return SCORE_SONG001_EASY;
    }

    if (value == "SCORE_SONG001_NORMAL"){
        return SCORE_SONG001_NORMAL;
    }

    if (value == "SCORE_SONG001_HARD"){
        return SCORE_SONG001_HARD;
    }

    if (value == "SCORE_SONG002_EASY") {
        return SCORE_SONG002_EASY;
    }

    if (value == "SCORE_SONG002_NORMAL") {
        return SCORE_SONG002_NORMAL;
    }

    if (value == "SCORE_SONG002_HARD") {
        return SCORE_SONG002_HARD;
    }

    if (value == "SCORE_SONG003_EASY") {
        return SCORE_SONG003_EASY;
    }

    if (value == "SCORE_SONG003_NORMAL") {
        return SCORE_SONG003_NORMAL;
    }

    if (value == "SCORE_SONG003_HARD") {
        return SCORE_SONG003_HARD;
    }


    return INVALID_ID;
}

static int ParseJacketTextureId( const std::string& value){
    if (value == "Jacket_Song001"){
        return Resouce_Manager_GetTexId(Song001_jacket);
    }

    if (value == "Jacket_Song002"){
        return Resouce_Manager_GetTexId(Song002_jacket);
    }

    if (value == "Jacket_Song003"){
        return Resouce_Manager_GetTexId(Song003_jacket);
    }

    return INVALID_ID;
}

static int ParseBgmAudioId(const std::string& value){
    if (value == "Song_BGM_001"){
        return Resouce_Manager_GetAudioId(Game_Song001_BGM);
    }

    if (value == "Song_BGM_002"){
        return Resouce_Manager_GetAudioId(Game_Song002_BGM);
    }

    if (value == "Song_BGM_003"){
        return Resouce_Manager_GetAudioId(Game_Song003_BGM);
    }

    return INVALID_ID;
}