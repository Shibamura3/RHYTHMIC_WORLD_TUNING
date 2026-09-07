/*
    キャラクター選択画面の制御：Character_select.cpp

    2026/08/27	hibiki sakuma
*/

#include "Character_select.h"
#include "resource_manager.h"
#include "text_manager.h"
#include "character_text_manager.h"
#include "Configu.h"
#include "Audio.h"
#include "sprite.h"
#include "direct3d.h"
#include "fade.h"
#include "Key_logger.h"
#include "pad_logger.h"
#include "scene.h"
#include <string>

#include <DirectXMath.h>
using namespace DirectX;

// 定数
// 共通
static constexpr int INVALID_ID = -1;
static constexpr int CHARACTER_COUNT = static_cast<int>(PlayerCharacter::Max);
static constexpr int INITIAL_CHARACTER_INDEX = 0;
// フェード
static constexpr float FADE_IN_DURATION = 1.0f;
static constexpr float FADE_OUT_DURATION = 0.5f;
// 入力
static constexpr float STICK_MENU_THRESHOLD = 0.60f;
// 三分割背景
static constexpr float CHARACTER_DIVIDER_WIDTH_RATE = 0.003f;
// キャラクター画像
static constexpr float CHARACTER_Y_RATE = 0.10f;
static constexpr float CHARACTER_WIDTH_SCALE = 1.25f;
static constexpr float CHARACTER_HEIGHT_RATE = 1.25f;
// 立ち絵位置補正
static constexpr float FIRST_CHARACTER_X_OFFSET_RATE = -0.03f;
static constexpr float SECOND_CHARACTER_X_OFFSET_RATE =-0.10f;
static constexpr float THIRD_CHARACTER_X_OFFSET_RATE = -0.05f;
// 説明ウィンドウ
static constexpr float DESCRIPTION_MARGIN_RATE = 0.03f;
static constexpr float DESCRIPTION_Y_RATE = 0.55f;
static constexpr float DESCRIPTION_HEIGHT_RATE = 0.4f;
// 説明文字
static constexpr float DESCRIPTION_TEXT_MARGIN_X_RATE = 0.06f;
static constexpr float CHARACTER_NAME_Y_RATE = 0.08f;
static constexpr float SKILL_NAME_Y_RATE = 0.30f;
static constexpr float DESCRIPTION_TEXT_Y_RATE = 0.45f;
static constexpr float CHARACTER_NAME_SCALE = 0.90f;
static constexpr float SKILL_NAME_SCALE = 0.75f;
static constexpr float DESCRIPTION_TEXT_SCALE = 0.65f;
// 操作ガイド
static constexpr float GUIDE_TEXT_X_RATE = 0.02f;
static constexpr float GUIDE_TEXT_Y_RATE = 0.95f;
static constexpr float GUIDE_TEXT_SCALE = 0.70f;
// 色
static constexpr XMFLOAT4 CHARACTER_SELECTED_COLOR{1.0f, 1.0f, 1.0f, 1.0f};
static constexpr XMFLOAT4 CHARACTER_UNSELECTED_COLOR{0.35f, 0.35f, 0.35f, 1.0f};
static constexpr XMVECTORF32 CHARACTER_NAME_COLOR{1.0f, 1.0f, 0.0f, 1.0f};
static constexpr XMVECTORF32 SKILL_NAME_COLOR{0.30f, 0.30f, 0.30f, 1.0f};
static constexpr XMVECTORF32 DESCRIPTION_TEXT_COLOR{1.0f, 1.0f, 1.0f, 1.0f};
static constexpr XMVECTORF32 GUIDE_TEXT_COLOR{1.0f, 1.0f, 1.0f, 1.0f};
static constexpr XMFLOAT4 CHARACTER_BACKGROUND_COLORS[CHARACTER_COUNT]{
    { 0.70f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.70f, 1.0f },
    { 0.70f, 0.70f, 0.0f, 1.0f }
};

// データ型
enum class CharacterSelectState
{
    Selecting,
    TransitionToGame,
    TransitionToMusicSelect
};

struct CharacterSelectData
{
    PlayerCharacter character = PlayerCharacter::Riff;

    int textureId = INVALID_ID;

    const char* characterTextId = "";

    float xOffsetRate = 0.0f;
    float yRate = CHARACTER_Y_RATE;

    float widthScale = CHARACTER_WIDTH_SCALE;
    float heightRate = CHARACTER_HEIGHT_RATE;
};

// 変数宣言
// 画面情報
static float g_ScreenX = 0.0f;
static float g_ScreenY = 0.0f;
// 選択状態
static int g_SelectedCharacterIndex = INITIAL_CHARACTER_INDEX;
static PlayerCharacter g_SelectedCharacter = PlayerCharacter::Riff;
static CharacterSelectState g_SelectState = CharacterSelectState::Selecting;
// スティック入力状態
static bool g_StickLeftPrev = false;
static bool g_StickRightPrev = false;
// キャラクターデータ
static CharacterSelectData g_CharacterData[CHARACTER_COUNT];

// 内部関数
// 入力・選択
static void MoveCharacterSelection(int direction);
static void DecideCharacter();
static void CancelCharacterSelection();
// レイアウト計算
static float GetCharacterAreaWidth();
static float GetCharacterAreaX(int index);
// 描画
static void DrawBackground();
static void DrawCharacters();
static void DrawCharacter(int index);
static void DrawDescriptionWindow();
static void DrawCharacterInformation();
static void DrawControlGuide();

void Character_Select_Initialize(){
    Fade_Start(FADE_IN_DURATION, false);

    g_ScreenX = static_cast<float>(Direct3D_GetBackBufferWidth());
    g_ScreenY = static_cast<float>(Direct3D_GetBackBufferHeight());

    g_SelectedCharacterIndex = INITIAL_CHARACTER_INDEX;
    g_SelectState = CharacterSelectState::Selecting;

    g_StickLeftPrev = false;
    g_StickRightPrev = false;

    Character_Text_Initialize(Resouce_Manager_GetDataFilePath(DataFileID::Character_Select_text));


    g_CharacterData[static_cast<int>(PlayerCharacter::Riff)] =
        {
            PlayerCharacter::Riff,
            Resouce_Manager_GetTexId(Riff_Smile),
            "RIFF",
            FIRST_CHARACTER_X_OFFSET_RATE,
            CHARACTER_Y_RATE,
            CHARACTER_WIDTH_SCALE,
            CHARACTER_HEIGHT_RATE
        };

    g_CharacterData[static_cast<int>(PlayerCharacter::Luna)] =
        {
            PlayerCharacter::Luna,
            Resouce_Manager_GetTexId(Luna_Smile),
            "LUNA",
            SECOND_CHARACTER_X_OFFSET_RATE,
            CHARACTER_Y_RATE,
            CHARACTER_WIDTH_SCALE,
            CHARACTER_HEIGHT_RATE
        };

    g_CharacterData[static_cast<int>(PlayerCharacter::Melody)] =
    {
        PlayerCharacter::Melody,
        Resouce_Manager_GetTexId(Melody_Smile),
        "MELODY",
        THIRD_CHARACTER_X_OFFSET_RATE,
        CHARACTER_Y_RATE,
        CHARACTER_WIDTH_SCALE,
        CHARACTER_HEIGHT_RATE
    };

    g_SelectedCharacter = g_CharacterData[g_SelectedCharacterIndex].character;
    PlayAudio(Resouce_Manager_GetAudioId(Select_BGM));
}

void Character_Select_Finalize() {
    StopAudio(Resouce_Manager_GetAudioId(Select_BGM));
    Character_Text_Finalize();
}

void Character_Select_Update(double elapsed_time){
    (void)elapsed_time;
    // 次画面
    if (g_SelectState == CharacterSelectState::TransitionToGame){
        if (Fade_GetState() == FADE_STATE_FINISHED_OUT){
            Scene_Change(SCENE_GAME);
        }
        return;
    }
    // 前画面
    if (g_SelectState == CharacterSelectState::TransitionToMusicSelect){
        if (Fade_GetState() == FADE_STATE_FINISHED_OUT) {
            Scene_Change(SCENE_MUSIC_SELEC);
        }
        return;
    }

    const bool decide = KeyLogger_IsTrigger(KK_ENTER) || PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_A);
    const bool cancel = KeyLogger_IsTrigger(KK_TAB) || PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_B);

    const XMFLOAT2 leftStick = PadLogger_GetLeftThumbStick(0);
    const bool stickLeftNow = leftStick.x < -STICK_MENU_THRESHOLD;
    const bool stickRightNow = leftStick.x > STICK_MENU_THRESHOLD;

    const bool stickLeftTrigger = stickLeftNow && !g_StickLeftPrev;
    const bool stickRightTrigger = stickRightNow && !g_StickRightPrev;

    g_StickLeftPrev = stickLeftNow;
    g_StickRightPrev = stickRightNow;

    const bool moveLeft = KeyLogger_IsTrigger(KK_A) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ||
        stickLeftTrigger;
    const bool moveRight = KeyLogger_IsTrigger(KK_D) ||
        PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ||
        stickRightTrigger;

    if (moveLeft){
        MoveCharacterSelection(-1);
    } else if (moveRight){
        MoveCharacterSelection(1);
    } else if (decide) {
        DecideCharacter();
    } else if (cancel) {
        CancelCharacterSelection();
    }
}

void Character_Select_Draw() {
    DrawBackground();

    DrawCharacters();

    DrawDescriptionWindow();

    DrawCharacterInformation();

    DrawControlGuide();
}

PlayerCharacter Character_Select_GetCharacter(){
    return g_SelectedCharacter;
}

static void MoveCharacterSelection(int direction){
    g_SelectedCharacterIndex += direction;

    if (g_SelectedCharacterIndex < 0){
        g_SelectedCharacterIndex = CHARACTER_COUNT - 1;
    } else if (g_SelectedCharacterIndex >= CHARACTER_COUNT){
        g_SelectedCharacterIndex = 0;
    }

    g_SelectedCharacter = g_CharacterData[g_SelectedCharacterIndex].character;

    PlayAudio(Resouce_Manager_GetAudioId(CursorMove_SE));
}

static void DecideCharacter(){
    g_SelectedCharacter = g_CharacterData[g_SelectedCharacterIndex].character;

    PlayAudio(Resouce_Manager_GetAudioId(MusicSelect_Decide_SE));

    g_SelectState = CharacterSelectState::TransitionToGame;

    Fade_Start(FADE_OUT_DURATION, true);
}

static void CancelCharacterSelection(){
    PlayAudio(Resouce_Manager_GetAudioId(Cancel_SE));

    g_SelectState = CharacterSelectState::TransitionToMusicSelect;

    Fade_Start(FADE_OUT_DURATION, true);
}

static float GetCharacterAreaWidth(){
    const float dividerWidth = g_ScreenX * CHARACTER_DIVIDER_WIDTH_RATE;

    const float totalDividerWidth = dividerWidth * static_cast<float>(CHARACTER_COUNT - 1);

    return (g_ScreenX - totalDividerWidth) / static_cast<float>(CHARACTER_COUNT);
}

static float GetCharacterAreaX(int index){
    const float dividerWidth = g_ScreenX * CHARACTER_DIVIDER_WIDTH_RATE;

    return (GetCharacterAreaWidth() + dividerWidth) * static_cast<float>(index);
}

static void DrawBackground(){
    Sprite_Draw(
        Resouce_Manager_GetTexId(Color_Black),
        0.0f, 0.0f,
        g_ScreenX, g_ScreenY
    );

    const float areaWidth = GetCharacterAreaWidth();

    for (int index = 0; index < CHARACTER_COUNT; ++index){
        Sprite_Draw(
            Resouce_Manager_GetTexId(Color_White),
            GetCharacterAreaX(index), 0.0f, 
            areaWidth, g_ScreenY,
            CHARACTER_BACKGROUND_COLORS[index]
        );
    }
}

static void DrawCharacters(){
    for (int index = 0; index < CHARACTER_COUNT; ++index){
        if (index == g_SelectedCharacterIndex){
            continue;
        }
        DrawCharacter(index);
    }

    DrawCharacter(g_SelectedCharacterIndex);
}

static void DrawCharacter(int index){
    if (index < 0 || index >= CHARACTER_COUNT){
        return;
    }

    const CharacterSelectData& data = g_CharacterData[index];

    if (data.textureId == INVALID_ID){
        return;
    }

    const float areaWidth = GetCharacterAreaWidth();
    const float areaX = GetCharacterAreaX(index);

    const float characterX = areaX + areaWidth * data.xOffsetRate;
    const float characterY = g_ScreenY * data.yRate;

    const float characterWidth = areaWidth * data.widthScale;
    const float characterHeight = g_ScreenY * data.heightRate;

    const bool isSelected = index == g_SelectedCharacterIndex;

    const XMFLOAT4 characterColor = isSelected
        ? CHARACTER_SELECTED_COLOR
        : CHARACTER_UNSELECTED_COLOR;

    Sprite_Draw(
        data.textureId,
        characterX, characterY,
        characterWidth, characterHeight,
        characterColor);
}

static void DrawDescriptionWindow(){
    const float areaWidth = GetCharacterAreaWidth();

    const float descriptionMargin = areaWidth * DESCRIPTION_MARGIN_RATE;

    const float descriptionX = GetCharacterAreaX(g_SelectedCharacterIndex) + descriptionMargin;

    const float descriptionWidth = areaWidth - descriptionMargin * 2.0f;

    Sprite_Draw(
        Resouce_Manager_GetTexId(TextBox),
        descriptionX, g_ScreenY * DESCRIPTION_Y_RATE,
        descriptionWidth, g_ScreenY * DESCRIPTION_HEIGHT_RATE
    );
}

static void DrawCharacterInformation() {
    const CharacterSelectData& data = g_CharacterData[g_SelectedCharacterIndex];
    const CharacterLocalizedText& text = Character_Text_Get(data.characterTextId);

    const float areaWidth = GetCharacterAreaWidth();
    const float descriptionMargin = areaWidth * DESCRIPTION_MARGIN_RATE;

    const float windowX = GetCharacterAreaX(g_SelectedCharacterIndex) + descriptionMargin;
    const float windowY = g_ScreenY * DESCRIPTION_Y_RATE;

    const float windowWidth = areaWidth - descriptionMargin * 2.0f;
    const float windowHeight = g_ScreenY * DESCRIPTION_HEIGHT_RATE;

    const float textX = windowX + windowWidth * DESCRIPTION_TEXT_MARGIN_X_RATE;

    // キャラクター説明とスキル説明をまとめる
    std::wstring combinedDescription = text.description;

    if (!text.skillDescription.empty()) {
        if (!combinedDescription.empty()) {
            combinedDescription += L"\n\n";
        }

        combinedDescription += text.skillDescription;
    }

    Text_Begin();

    Text_Draw(
        text.name.c_str(),
        textX, windowY + windowHeight * CHARACTER_NAME_Y_RATE,
        CHARACTER_NAME_COLOR,
        CHARACTER_NAME_SCALE
    );

    Text_Draw(
        text.skillName.c_str(),
        textX, windowY + windowHeight * SKILL_NAME_Y_RATE,
        SKILL_NAME_COLOR,
        SKILL_NAME_SCALE
    );

    Text_Draw(
        combinedDescription.c_str(),
        textX, windowY + windowHeight * DESCRIPTION_TEXT_Y_RATE,
        DESCRIPTION_TEXT_COLOR,
        DESCRIPTION_TEXT_SCALE
    );

    Text_End();
}

static void DrawControlGuide(){
    const wchar_t* guideText = nullptr;

    if (PadLogger_IsConnected()){
        switch (PadLogger_GetControllerDisplayType())
        {
        case ControllerDisplayType::PlayStation:
            guideText = L"[L STICK / D-PAD] SELECT  [X] DECIDE  [O] BACK";
            break;

        case ControllerDisplayType::Xbox:
        case ControllerDisplayType::Unknown:
        default:
            guideText = L"[L STICK / D-PAD] SELECT  [A] DECIDE  [B] BACK";
            break;
        }
    }else{
        guideText = L"[A/D] SELECT  [ENTER] DECIDE  [TAB] BACK";
    }

    Text_Begin();

    Text_Draw(
        guideText,
        g_ScreenX * GUIDE_TEXT_X_RATE, g_ScreenY * GUIDE_TEXT_Y_RATE,
        GUIDE_TEXT_COLOR,
        GUIDE_TEXT_SCALE
    );

    Text_End();
}