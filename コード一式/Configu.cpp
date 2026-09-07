/*
    コンフィグ画面の制御：Configu.cpp

    2026/07/09    hibiki sakuma
*/

#include "Configu.h"

#include "config_demo.h"
#include "resource_manager.h"
#include "text_manager.h"
#include "ui_text_manager.h"
#include "Audio.h"
#include "sprite.h"
#include "Key_logger.h"
#include "pad_logger.h"
#include "direct3d.h"
#include "scene.h"
#include "game_window.h"

#include <cwchar>
#include <fstream>

#include <DirectXMath.h>

using namespace DirectX;


// 定数宣言
// 共通
static constexpr int INVALID_ID = -1;

static constexpr int INITIAL_ITEM_INDEX = 0;

// ノーツ速度
static constexpr float NOTE_SPEED_DEFAULT = 1.0f;

static constexpr float NOTE_SPEED_MIN = 0.5f;
static constexpr float NOTE_SPEED_MAX = 3.0f;

static constexpr float NOTE_SPEED_STEP = 0.1f;

// 音量
static constexpr int AUDIO_VOLUME_DEFAULT = 100;

static constexpr int AUDIO_VOLUME_MIN = 0;
static constexpr int AUDIO_VOLUME_MAX = 100;

static constexpr int AUDIO_VOLUME_STEP = 10;

// 初期画面サイズ
static constexpr ScreenSize SCREEN_SIZE_DEFAULT = ScreenSize::Size1600x900;

// 入力リピート
static constexpr float REPEAT_START = 0.4f;
static constexpr float REPEAT_SPEED = 0.1f;

// 文字列バッファ
static constexpr int TEXT_SIZE = 32;

// 画面全体
static constexpr float CONFIG_WINDOW_X_RATE = 0.05f;
static constexpr float CONFIG_WINDOW_Y_RATE = 0.05f;

static constexpr float CONFIG_WINDOW_WIDTH_RATE = 0.90f;
static constexpr float CONFIG_WINDOW_HEIGHT_RATE = 0.90f;

// タイトル
static constexpr float CONFIG_TITLE_X_RATE = 0.10f;
static constexpr float CONFIG_TITLE_Y_RATE = 0.08f;

static constexpr float CONFIG_TITLE_SCALE = 1.50f;

// 設定項目
static constexpr float CONFIG_ITEM_X_RATE = 0.12f;
static constexpr float CONFIG_VALUE_X_RATE = 0.38f;

static constexpr float CONFIG_ITEM_START_Y_RATE = 0.15f;
static constexpr float CONFIG_ITEM_SPACING_RATE = 0.055f;

static constexpr float CONFIG_ITEM_TEXT_SCALE = 0.85f;
static constexpr float CONFIG_VALUE_TEXT_SCALE = 0.80f;

// 設定バー
static constexpr float CONFIG_BAR_X_RATE = 0.38f;

static constexpr float CONFIG_BAR_WIDTH_RATE = 0.22f;
static constexpr float CONFIG_BAR_HEIGHT_RATE = 0.022f;

static constexpr float CONFIG_BAR_VALUE_X_RATE = 0.62f;

static constexpr float CONFIG_BAR_BACKGROUND_ALPHA = 0.60f;

// ノーツ速度確認エリア
static constexpr float CONFIG_DEMO_X_RATE = 0.68f;
static constexpr float CONFIG_DEMO_Y_RATE = 0.18f;

static constexpr float CONFIG_DEMO_WIDTH_RATE = 0.22f;
static constexpr float CONFIG_DEMO_HEIGHT_RATE = 0.62f;

// 操作ガイド
static constexpr float GUIDE_TEXT_X_RATE = 0.10f;
static constexpr float GUIDE_TEXT_Y_RATE = 0.91f;
static constexpr float GUIDE_TEXT_SCALE = 0.70f;

// フェード
static constexpr float FADE_OUT_DURATION = 0.5f;

// 画像用
// 背景
static constexpr float BACKGROUND_X = 0.0f;
static constexpr float BACKGROUND_Y = 0.0f;

// バーの行内位置
static constexpr float CONFIG_BAR_Y_OFFSET_RATE = 0.012f;

// ボタン項目の横位置
static constexpr float CONFIG_ACTION_X_RATE = 0.38f;

// 値表示形式
static constexpr int VOLUME_TEXT_DIGITS = 3;

// 色
static constexpr XMVECTORF32 CONFIG_WINDOW_COLOR{ 1.0f, 1.0f, 1.0f, 1.0f };
static constexpr XMVECTORF32 CONFIG_TEXT_COLOR{ 1.0f, 1.0f, 1.0f, 1.0f };
static constexpr XMVECTORF32 CONFIG_SELECTED_COLOR{ 1.0f, 1.0f, 0.0f, 1.0f };
static constexpr XMFLOAT4 CONFIG_BAR_BACKGROUND_COLOR{ 0.15f, 0.15f, 0.15f, CONFIG_BAR_BACKGROUND_ALPHA };
static constexpr XMFLOAT4 CONFIG_BAR_COLOR{ 0.20f,  0.75f,  1.00f,  1.0f };
static constexpr XMFLOAT4 CONFIG_BAR_SELECTED_COLOR{ 1.0f, 1.0f, 0.0f, 1.0f };
static constexpr XMVECTORF32 GUIDE_TEXT_COLOR{ 0.0f, 0.0f, 0.0f, 1.0f };

// 保存場所
static constexpr const char* CONFIG_SAVE_FILE_PATH = "save/config.dat";

// 設定項目
enum class ConfigItem
{
    MasterVolume,
    BGMVolume,
    SEVolume,
    VoiceVolume,

    Speed,
    Language,
    ScreenSize,

    Reset,
    Save,
    Back,

    Max
};

// 変数宣言
// 設定データ
static Config g_Config
{
    AUDIO_VOLUME_DEFAULT,
    AUDIO_VOLUME_DEFAULT,
    AUDIO_VOLUME_DEFAULT,
    AUDIO_VOLUME_DEFAULT,

    NOTE_SPEED_DEFAULT,

    Language::Japanese,

    SCREEN_SIZE_DEFAULT
};

// キャンセル時に元の設定へ戻すためのデータ
static Config g_OriginalConfig
{
    AUDIO_VOLUME_DEFAULT,
    AUDIO_VOLUME_DEFAULT,
    AUDIO_VOLUME_DEFAULT,
    AUDIO_VOLUME_DEFAULT,

    NOTE_SPEED_DEFAULT,

    Language::Japanese,

    SCREEN_SIZE_DEFAULT
};

// 選択状態
static ConfigItem g_SelectItem = ConfigItem::MasterVolume;

// 入力
static bool g_MoveLeftTrigger = false;
static bool g_MoveRightTrigger = false;

static bool g_MoveUpTrigger = false;
static bool g_MoveDownTrigger = false;

static bool g_MoveLeftPressed = false;
static bool g_MoveRightPressed = false;

static bool g_Decide = false;
static bool g_Cancel = false;

// 長押し入力
static float g_LeftRepeatTimer = 0.0f;
static float g_RightRepeatTimer = 0.0f;

// 画面サイズ
static float g_ScreenWidth = 0.0f;
static float g_ScreenHeight = 0.0f;

// 画面遷移
static bool g_IsTransitioning = false;

// 内部関数宣言
// 入力
static void UpdateInput();
static bool IsRepeatInput(bool trigger, bool pressed, float& timer, double elapsed_time);

// 項目選択
static void MoveSelection(int direction);
static void ChangeCurrentValue(int direction);
static void ExecuteCurrentItem();

// 設定処理
static void SetDefaultConfig();
static void ResetConfig();
static void CancelConfig();
static void ApplyCurrentVolume();
static void ChangeLanguage(int direction);
static void ChangeScreenSize(int direction);
static void ValidateConfig();

// 値の範囲制限
static int ClampVolume(int volume);
static float ClampNoteSpeed(float noteSpeed);

// テスト音声
static void PlaySETest();
static void PlayVoiceTest();

// 画面サイズの設定
static int GetScreenWidth(ScreenSize screenSize);
static int GetScreenHeight(ScreenSize screenSize);
static const wchar_t* GetScreenSizeDisplayText(ScreenSize screenSize);

// 表示文字
static const char* GetConfigItemTextId(ConfigItem item);
static const wchar_t* GetConfigItemValueText(ConfigItem item, wchar_t* buffer, int bufferSize);

// 描画
static void DrawConfigWindow();
static void DrawConfigItems();
static void DrawConfigItem(ConfigItem item, int rowIndex);
static void DrawValueBar(ConfigItem item, float valueRate, float rowY);
static void DrawGuideText();

// 値取得
static float GetConfigValueRate(ConfigItem item); 

void Configu_Initialize(){
    // 前回保存した設定を読み込む
    Config_Load();

    ValidateConfig();

    // キャンセル時に戻すため画面を開いた時点の値を保持する
    g_OriginalConfig = g_Config;

    // 言語に対応するフォントへ更新
    Text_ReloadFont();

    // ノーツ速度確認デモ
    Configu_Demo_Initialize();

    // 先頭項目を選択
    g_SelectItem = ConfigItem::MasterVolume;

    // 入力状態
    g_MoveLeftTrigger = false;
    g_MoveRightTrigger = false;
    g_MoveUpTrigger = false;
    g_MoveDownTrigger = false;

    g_MoveLeftPressed = false;
    g_MoveRightPressed = false;

    g_Decide = false;
    g_Cancel = false;

    g_LeftRepeatTimer = 0.0f;
    g_RightRepeatTimer = 0.0f;

    // 画面サイズ
    g_ScreenWidth = static_cast<float>(Direct3D_GetBackBufferWidth());
    g_ScreenHeight = static_cast<float>(Direct3D_GetBackBufferHeight());

    g_IsTransitioning = false;

    // 読み込んだ値を音量へ反映
    ApplyCurrentVolume();

    // 動作確認用BGM
    PlayAudio(Resouce_Manager_GetAudioId(Game_BGM), true);
}

void Configu_Finalize(){
    StopAudio(Resouce_Manager_GetAudioId(Game_BGM));
    Configu_Demo_Finalize();
}

void Configu_Update(double elapsed_time){
    if (g_IsTransitioning)
        return;

    UpdateInput();

    Configu_Demo_Update(elapsed_time);

    // 項目移動
    if (g_MoveUpTrigger){
        MoveSelection(-1);
    }else if (g_MoveDownTrigger){
        MoveSelection(1);
    }

    // 左右入力
    const bool moveLeft = IsRepeatInput(
        g_MoveLeftTrigger, g_MoveLeftPressed, g_LeftRepeatTimer, elapsed_time);
    const bool moveRight = IsRepeatInput(
        g_MoveRightTrigger, g_MoveRightPressed, g_RightRepeatTimer, elapsed_time);

    if (moveLeft){
        ChangeCurrentValue(-1);
    } else if (moveRight){
        ChangeCurrentValue(1);
    }

    // RESET／SAVE／BACKなどを実行
    if (g_Decide){
        ExecuteCurrentItem();
    }

    // 設定を破棄して戻る
    if (g_Cancel){
        CancelConfig();
        return;
    }

    // 変更値を即時反映
    ApplyCurrentVolume();
}

void Configu_Draw(){
    DrawConfigWindow();

    Configu_Demo_Draw();

    DrawConfigItems();

    DrawGuideText();
}

void Config_Save(){
    std::ofstream file(CONFIG_SAVE_FILE_PATH, std::ios::binary | std::ios::trunc);

    if (!file){
        return;
    }

    file.write(reinterpret_cast<const char*>(&g_Config), sizeof(Config));
}

void Config_Load(){
    std::ifstream file(CONFIG_SAVE_FILE_PATH, std::ios::binary);

    if (!file){
        SetDefaultConfig();
        return;
    }

    file.read(reinterpret_cast<char*>(&g_Config), sizeof(Config));

    if (!file){
        SetDefaultConfig();
        return;
    }

    ValidateConfig();
}

float Configu_GetNoteSpeed() {
    return g_Config.NoteSpeed;
}

int Configu_GetVolume() {
    return g_Config.MasterVolume;
}

Language Configu_GetLanguage() {
    return g_Config.Lang;
}

const char* Configu_GetLanguageCode(){
    switch (Configu_GetLanguage())
    {
    case Language::Japanese:
        return "JP";

    case Language::English:
        return "EN";

    case Language::Korean:
        return "KR";

    case Language::Chinese:
        return "CN";

        // 今後追加
        
    default:
        return "JP";
    }
}

const wchar_t* Configu_GetLanguageDisplayText()
{
    switch (Configu_GetLanguage())
    {
    case Language::Japanese:
        return L"< JP >";

    case Language::English:
        return L"< EN >";

    case Language::Korean:
        return L"< KR >";

    case Language::Chinese:
        return L"< CN >";
    
    default:
        return L"< JP >";
    }
}

static void UpdateInput(){
    const bool padConnected = PadLogger_IsConnected();

    g_MoveLeftTrigger =
        KeyLogger_IsTrigger(KK_A) ||
        (padConnected && PadLogger_IsTrigger( 0,SDL_CONTROLLER_BUTTON_DPAD_LEFT));

    g_MoveRightTrigger =
        KeyLogger_IsTrigger(KK_D) ||
        (padConnected && PadLogger_IsTrigger( 0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));

    g_MoveUpTrigger =
        KeyLogger_IsTrigger(KK_W) ||
        (padConnected && PadLogger_IsTrigger( 0, SDL_CONTROLLER_BUTTON_DPAD_UP));

    g_MoveDownTrigger =
        KeyLogger_IsTrigger(KK_S) ||
        (padConnected && PadLogger_IsTrigger( 0, SDL_CONTROLLER_BUTTON_DPAD_DOWN));

    g_MoveLeftPressed =
        KeyLogger_IsPressed(KK_A) ||
        (padConnected && PadLogger_IsPressed( 0, SDL_CONTROLLER_BUTTON_DPAD_LEFT));

    g_MoveRightPressed =
        KeyLogger_IsPressed(KK_D) ||
        (padConnected && PadLogger_IsPressed( 0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));

    g_Decide =
        KeyLogger_IsTrigger(KK_ENTER) ||
        (padConnected && PadLogger_IsTrigger( 0, SDL_CONTROLLER_BUTTON_A));

    g_Cancel =
        KeyLogger_IsTrigger(KK_BACK) ||
        (padConnected && PadLogger_IsTrigger( 0, SDL_CONTROLLER_BUTTON_B));
}

static bool IsRepeatInput(bool trigger, bool pressed, float& timer, double elapsed_time){
    if (trigger){
        timer = 0.0f;
        return true;
    }

    if (!pressed){
        timer = 0.0f;
        return false;
    }

    timer += static_cast<float>(elapsed_time);

    if (timer >= REPEAT_START){
        timer -= REPEAT_SPEED;
        return true;
    }

    return false;
}

static void MoveSelection(int direction) {
    int index = static_cast<int>(g_SelectItem);

    index += direction;

    const int itemCount = static_cast<int>(ConfigItem::Max);

    if (index < INITIAL_ITEM_INDEX) {
        index = itemCount - 1;
    }
    else if (index >= itemCount) {
        index = INITIAL_ITEM_INDEX;
    }

    g_SelectItem = static_cast<ConfigItem>(index);

    PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
}

static void ChangeCurrentValue(int direction) {
    switch (g_SelectItem)
    {
    case ConfigItem::MasterVolume:
        g_Config.MasterVolume =
            ClampVolume(g_Config.MasterVolume + direction * AUDIO_VOLUME_STEP);
        break;

    case ConfigItem::BGMVolume:
        g_Config.BGMVolume =
            ClampVolume(g_Config.BGMVolume + direction * AUDIO_VOLUME_STEP);
        break;

    case ConfigItem::SEVolume:
        g_Config.SEVolume =
            ClampVolume(g_Config.SEVolume + direction * AUDIO_VOLUME_STEP);
        break;

    case ConfigItem::VoiceVolume:
        g_Config.VoiceVolume =
            ClampVolume(g_Config.VoiceVolume + direction * AUDIO_VOLUME_STEP);
        break;

    case ConfigItem::Speed:
        g_Config.NoteSpeed =
            ClampNoteSpeed(g_Config.NoteSpeed + static_cast<float>(direction) * NOTE_SPEED_STEP);
        break;

    case ConfigItem::Language:
        ChangeLanguage(direction);
        break;

    case ConfigItem::ScreenSize:
        ChangeScreenSize(direction);
        break;

    case ConfigItem::Reset:
    case ConfigItem::Save:
    case ConfigItem::Back:
    case ConfigItem::Max:
    default:
        return;
    }

    // 音量変更を先に反映する
    ApplyCurrentVolume();
}

static void ExecuteCurrentItem(){
    switch (g_SelectItem)
    {
    case ConfigItem::SEVolume:
        PlaySETest();
        break;

    case ConfigItem::VoiceVolume:
        PlayVoiceTest();
        break;

    case ConfigItem::Reset:
        ResetConfig();
        break;

    case ConfigItem::Save:
    {
        Config_Save();

        g_OriginalConfig = g_Config;

        const int width = GetScreenWidth(g_Config.Screen);
        const int height = GetScreenHeight(g_Config.Screen);

        GameWindow_SetClientSize(width, height);

        PlayAudio(Resouce_Manager_GetAudioId(Check_SE));

        g_IsTransitioning = true;

        Scene_Change(SCENE_MUSIC_SELEC);
        break;
    }
    case ConfigItem::Back:
        CancelConfig();
        break;

    case ConfigItem::MasterVolume:
    case ConfigItem::BGMVolume:
    case ConfigItem::Speed:
    case ConfigItem::Language:
    case ConfigItem::ScreenSize:
    case ConfigItem::Max:
    default:
        break;
    }
}

static void SetDefaultConfig(){
    g_Config.MasterVolume = AUDIO_VOLUME_DEFAULT;
    g_Config.BGMVolume = AUDIO_VOLUME_DEFAULT;
    g_Config.SEVolume = AUDIO_VOLUME_DEFAULT;
    g_Config.VoiceVolume = AUDIO_VOLUME_DEFAULT;
    g_Config.NoteSpeed =  NOTE_SPEED_DEFAULT;
    g_Config.Lang = Language::Japanese;
    g_Config.Screen = SCREEN_SIZE_DEFAULT;
}
static void ResetConfig(){
    SetDefaultConfig();

    Text_ReloadFont();

    ApplyCurrentVolume();

    PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
}

static void CancelConfig(){

    g_Config = g_OriginalConfig;

    Text_ReloadFont();

    ApplyCurrentVolume();

    g_IsTransitioning = true;

    Scene_Change(SCENE_MUSIC_SELEC);
}

static void ApplyCurrentVolume(){
    ApplyVolumeSetting(
        g_Config.MasterVolume,
        g_Config.BGMVolume,
        g_Config.SEVolume,
        g_Config.VoiceVolume
    );
}

static void ChangeLanguage(int direction){
    int index = static_cast<int>(g_Config.Lang);

    index += direction;

    const int languageCount = static_cast<int>(Language::MAX);

    if (index < 0){
        index = languageCount - 1;
    }else if (index >= languageCount){
        index = 0;
    }

    g_Config.Lang = static_cast<Language>(index);

    Text_ReloadFont();
}

static void ChangeScreenSize(int direction){
    int index = static_cast<int>(g_Config.Screen);

    index += direction;

    const int count = static_cast<int>(ScreenSize::Max);

    if (index < 0){
        index = count - 1;
    } else if (index >= count){
        index = 0;
    }

    g_Config.Screen = static_cast<ScreenSize>(index);
}

static void ValidateConfig(){
    const int languageIndex = static_cast<int>(g_Config.Lang);

    if (languageIndex < 0 ||
        languageIndex >= static_cast<int>(Language::MAX))
    {
        g_Config.Lang = Language::Japanese;
    }

    const int screenSizeIndex = static_cast<int>(g_Config.Screen);
    if (screenSizeIndex < 0 ||
        screenSizeIndex >= static_cast<int>(ScreenSize::Max))
    {
            g_Config.Screen = SCREEN_SIZE_DEFAULT;
    }

    if (g_Config.NoteSpeed < NOTE_SPEED_MIN ||
        g_Config.NoteSpeed >  NOTE_SPEED_MAX)
    {
        g_Config.NoteSpeed = NOTE_SPEED_DEFAULT;
    }

    if (g_Config.MasterVolume < AUDIO_VOLUME_MIN ||
        g_Config.MasterVolume > AUDIO_VOLUME_MAX)
    {
        g_Config.MasterVolume = AUDIO_VOLUME_DEFAULT;
    }

    if (g_Config.BGMVolume < AUDIO_VOLUME_MIN ||
        g_Config.BGMVolume > AUDIO_VOLUME_MAX)
    {
        g_Config.BGMVolume = AUDIO_VOLUME_DEFAULT;
    }

    if (g_Config.SEVolume < AUDIO_VOLUME_MIN ||
        g_Config.SEVolume > AUDIO_VOLUME_MAX)
    {
        g_Config.SEVolume = AUDIO_VOLUME_DEFAULT;
    }

    if (g_Config.VoiceVolume < AUDIO_VOLUME_MIN ||
        g_Config.VoiceVolume > AUDIO_VOLUME_MAX)
    {
        g_Config.VoiceVolume = AUDIO_VOLUME_DEFAULT;
    }
}

static int ClampVolume(int volume){
    if (volume < AUDIO_VOLUME_MIN) return AUDIO_VOLUME_MIN;
    
    if (volume > AUDIO_VOLUME_MAX) return AUDIO_VOLUME_MAX;

    return volume;
}

static float ClampNoteSpeed(float noteSpeed){
    if (noteSpeed < NOTE_SPEED_MIN) return NOTE_SPEED_MIN;

    if (noteSpeed > NOTE_SPEED_MAX) return NOTE_SPEED_MAX;

    return noteSpeed;
}

static void PlaySETest(){
    PlayAudio(Resouce_Manager_GetAudioId(Check_SE));
}

static void PlayVoiceTest(){
    PlayAudio(Resouce_Manager_GetAudioId(TestCV));
}

static int GetScreenWidth(ScreenSize screenSize){
    switch (screenSize)
    {
    case ScreenSize::Size1280x720:
        return 1280;

    case ScreenSize::Size1600x900:
        return 1600;

    case ScreenSize::Size1920x1080:
        return 1920;

    case ScreenSize::Max:
    default:
        return 1600;
    }
}

static int GetScreenHeight(ScreenSize screenSize){
    switch (screenSize)
    {
    case ScreenSize::Size1280x720:
        return 720;

    case ScreenSize::Size1600x900:
        return 900;

    case ScreenSize::Size1920x1080:
        return 1080;

    case ScreenSize::Max:
    default:
        return 900;
    }
}

static const wchar_t* GetScreenSizeDisplayText(ScreenSize screenSize){
    switch (screenSize)
    {
    case ScreenSize::Size1280x720:
        return L"< 1280 x 720 >";

    case ScreenSize::Size1600x900:
        return L"< 1600 x 900 >";

    case ScreenSize::Size1920x1080:
        return L"< 1920 x 1080 >";

    case ScreenSize::Max:
    default:
        return L"< 1600 x 900 >";
    }
}

static const char* GetConfigItemTextId(ConfigItem item){
    switch (item)
    {
    case ConfigItem::MasterVolume:
        return "CONFIG_MASTER_VOLUME";

    case ConfigItem::BGMVolume:
        return "CONFIG_BGM_VOLUME";

    case ConfigItem::SEVolume:
        return "CONFIG_SE_VOLUME";

    case ConfigItem::VoiceVolume:
        return "CONFIG_VOICE_VOLUME";

    case ConfigItem::Speed:
        return "CONFIG_NOTE_SPEED";

    case ConfigItem::Language:
        return "CONFIG_LANGUAGE";

    case ConfigItem::ScreenSize:
        return "CONFIG_SCREEN_SIZE";
    
    case ConfigItem::Reset:
        return "CONFIG_RESET";

    case ConfigItem::Save:
        return "CONFIG_SAVE";

    case ConfigItem::Back:
        return "CONFIG_BACK";

    case ConfigItem::Max:
    default:
        return "";
    }
}

static const wchar_t* GetConfigItemValueText(ConfigItem item, wchar_t* buffer, int bufferSize){
    if (buffer == nullptr || bufferSize <= 0){
        return L"";
    }

    buffer[0] = L'\0';

    switch (item)
    {
    case ConfigItem::MasterVolume:
        swprintf_s(buffer, bufferSize, L"%03d", g_Config.MasterVolume);
        break;

    case ConfigItem::BGMVolume:
        swprintf_s(buffer, bufferSize, L"%03d", g_Config.BGMVolume);
        break;

    case ConfigItem::SEVolume:
        swprintf_s(buffer, bufferSize, L"%03d", g_Config.SEVolume);
        break;

    case ConfigItem::VoiceVolume:
        swprintf_s(buffer, bufferSize, L"%03d", g_Config.VoiceVolume);
        break;

    case ConfigItem::Speed:
        swprintf_s(buffer, bufferSize, L"%.1fx", g_Config.NoteSpeed);
        break;

    case ConfigItem::Language:
        return Configu_GetLanguageDisplayText();

    case ConfigItem::ScreenSize:
        return GetScreenSizeDisplayText(g_Config.Screen);

    case ConfigItem::Reset:
    case ConfigItem::Save:
    case ConfigItem::Back:
    case ConfigItem::Max:
    default:
        return L"";
    }

    return buffer;
}

static void DrawConfigWindow()
{
    // 背景
    Sprite_Draw(
        Resouce_Manager_GetTexId(Select_Back), // 背景は継続
        BACKGROUND_X, BACKGROUND_Y,
        g_ScreenWidth, g_ScreenHeight
    );

    // 大きな設定ウィンドウ
    Sprite_Draw(
        Resouce_Manager_GetTexId(TextBox),
        g_ScreenWidth * CONFIG_WINDOW_X_RATE, g_ScreenHeight * CONFIG_WINDOW_Y_RATE,
        g_ScreenWidth * CONFIG_WINDOW_WIDTH_RATE, g_ScreenHeight * CONFIG_WINDOW_HEIGHT_RATE
    );
}

static void DrawConfigItem(ConfigItem item, int rowIndex){
    const float rowY = g_ScreenHeight *(CONFIG_ITEM_START_Y_RATE + CONFIG_ITEM_SPACING_RATE * static_cast<float>(rowIndex));

    const bool isSelected = g_SelectItem == item;

    const XMVECTORF32 textColor = isSelected
        ? CONFIG_SELECTED_COLOR
        : CONFIG_TEXT_COLOR;

    // 項目名
    Text_Draw(
        UI_Text_Get(GetConfigItemTextId(item)).c_str(),
        g_ScreenWidth * CONFIG_ITEM_X_RATE,
        rowY,
        textColor,
        CONFIG_ITEM_TEXT_SCALE
    );

    wchar_t valueText[TEXT_SIZE]{};

        const wchar_t* value =
            GetConfigItemValueText(
                item,
                valueText,
                TEXT_SIZE
            );

        // 音量
        switch (item)
        {
        case ConfigItem::MasterVolume:
        case ConfigItem::BGMVolume:
        case ConfigItem::SEVolume:
        case ConfigItem::VoiceVolume:
        case ConfigItem::Speed:

            Text_Draw(
                value,
                g_ScreenWidth * CONFIG_BAR_VALUE_X_RATE,
                rowY,
                textColor,
                CONFIG_VALUE_TEXT_SCALE
            );
            break;

        case ConfigItem::Language:
        case ConfigItem::ScreenSize:
            Text_Draw(
                value,
                g_ScreenWidth * CONFIG_VALUE_X_RATE,
                rowY,
                textColor,
                CONFIG_VALUE_TEXT_SCALE
            );
            break;

        case ConfigItem::Reset:
        case ConfigItem::Save:
        case ConfigItem::Back:
        case ConfigItem::Max:
        default:
            break;
        }
}

static void DrawConfigItems(){
    const int itemCount = static_cast<int>(ConfigItem::Max);

    // バーを先に描画
    for (int index = 0; index < itemCount; ++index){
        const ConfigItem item = static_cast<ConfigItem>(index);

        switch (item)
        {
        case ConfigItem::MasterVolume:
        case ConfigItem::BGMVolume:
        case ConfigItem::SEVolume:
        case ConfigItem::VoiceVolume:
        case ConfigItem::Speed:
        {
            const float rowY = g_ScreenHeight * (CONFIG_ITEM_START_Y_RATE + CONFIG_ITEM_SPACING_RATE * static_cast<float>(index));

            DrawValueBar(
                item,
                GetConfigValueRate(item),
                rowY
            );
            break;
        }

        default:
            break;
        }
    }

    // 文字を後から描画

    Text_Begin();

    Text_Draw(
        UI_Text_Get("CONFIG_TITLE").c_str(),
        g_ScreenWidth * CONFIG_TITLE_X_RATE,
        g_ScreenHeight * CONFIG_TITLE_Y_RATE,
        CONFIG_SELECTED_COLOR,
        CONFIG_TITLE_SCALE
    );

    for (int index = 0; index < itemCount; ++index){
        DrawConfigItem(
            static_cast<ConfigItem>(index),
            index
        );
    }

    Text_End();
}

static void DrawValueBar(ConfigItem item, float valueRate, float rowY){
    if (valueRate < 0.0f) valueRate = 0.0f;

    if (valueRate > 1.0f) valueRate = 1.0f;

    const float barX = g_ScreenWidth * CONFIG_BAR_X_RATE;
    const float barY = rowY +  g_ScreenHeight * CONFIG_BAR_Y_OFFSET_RATE;

    const float barWidth = g_ScreenWidth * CONFIG_BAR_WIDTH_RATE;
    const float barHeight = g_ScreenHeight *  CONFIG_BAR_HEIGHT_RATE;

    const int whiteTexId =Resouce_Manager_GetTexId(Color_White);

    // 背景
    Sprite_Draw(
        whiteTexId,
        barX,  barY,
        barWidth, barHeight,
        CONFIG_BAR_BACKGROUND_COLOR
    );

    const bool isSelected = g_SelectItem == item;

    const XMFLOAT4 barColor = isSelected
        ? XMFLOAT4 {1.0f,1.0f,1.0f,1.0f}
        : XMFLOAT4 {0.0f,1.0f,1.0f,1.0f};

    // 現在値
    Sprite_Draw(
        whiteTexId,
        barX, barY,
        barWidth * valueRate, barHeight,
        barColor
    );
}

static void DrawGuideText(){
    const char* guideTextId = "CONFIG_GUIDE_KEYBOARD";

    if (PadLogger_IsConnected()){
        switch (PadLogger_GetControllerDisplayType())
        {
            case ControllerDisplayType::PlayStation:
                guideTextId = "CONFIG_GUIDE_PS";
                break;

            case ControllerDisplayType::Xbox:
            case ControllerDisplayType::Unknown:
            default:
                guideTextId = "CONFIG_GUIDE_XBOX";
                break;
        }
    }

    Text_Begin();

    Text_Draw(
        UI_Text_Get(guideTextId).c_str(),
        g_ScreenWidth * GUIDE_TEXT_X_RATE,
        g_ScreenHeight * GUIDE_TEXT_Y_RATE,
        GUIDE_TEXT_COLOR,
        GUIDE_TEXT_SCALE
    );

    Text_End();
}

static float GetConfigValueRate(ConfigItem item){
    switch (item)
    {
    case ConfigItem::MasterVolume:
        return
            static_cast<float>(g_Config.MasterVolume - AUDIO_VOLUME_MIN) /
            static_cast<float>(AUDIO_VOLUME_MAX - AUDIO_VOLUME_MIN);

    case ConfigItem::BGMVolume:
        return
            static_cast<float>(g_Config.BGMVolume - AUDIO_VOLUME_MIN) /
            static_cast<float>(AUDIO_VOLUME_MAX - AUDIO_VOLUME_MIN);

    case ConfigItem::SEVolume:
        return
            static_cast<float>(g_Config.SEVolume - AUDIO_VOLUME_MIN) /
            static_cast<float>(AUDIO_VOLUME_MAX - AUDIO_VOLUME_MIN);

    case ConfigItem::VoiceVolume:
        return
            static_cast<float>(g_Config.VoiceVolume - AUDIO_VOLUME_MIN) /
            static_cast<float>(AUDIO_VOLUME_MAX - AUDIO_VOLUME_MIN);

    case ConfigItem::Speed:
        return
            (g_Config.NoteSpeed - NOTE_SPEED_MIN) /
            (NOTE_SPEED_MAX - NOTE_SPEED_MIN);

    case ConfigItem::Language:
    case ConfigItem::ScreenSize:
    case ConfigItem::Reset:
    case ConfigItem::Save:
    case ConfigItem::Back:
    case ConfigItem::Max:
    default:
        return 0.0f;
    }
}