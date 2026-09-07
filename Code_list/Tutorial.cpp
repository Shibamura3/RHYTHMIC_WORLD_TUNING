/*
    チュートリアル画面の制御：Tutorial.h

    2026/08/17	hibiki sakuma
*/

#include "Tutorial.h"

#include "Audio.h"
#include "Configu.h"
#include "Key_logger.h"
#include "Music_select.h"
#include "Player.h"
#include "bg.h"
#include "combo.h"
#include "direct3d.h"
#include "enemy.h"
#include "fade.h"
#include "note.h"
#include "pad_logger.h"
#include "resource_manager.h"
#include "scene.h"
#include "sprite.h"
#include "text_manager.h"
#include "ui.h"

#include <cmath>
#include <codecvt>
#include <fstream>
#include <string>
#include <vector>

#include <DirectXMath.h>

using namespace DirectX;

// 定数宣言
// ゲームプレイ
static constexpr float NOTE_SPEED_DEFAULT = 500.0f;
static constexpr float BASE_DAMAGE = 10.0f;

static constexpr int GAMESCORE_PERFECT = 100;
static constexpr int GAMESCORE_GREAT = 50;
static constexpr int GAMESCORE_KILL = 1000;

// チュートリアル進行
static constexpr float TUTORIAL_END_WAIT_TIME = 1.0f;
static constexpr float TUTORIAL_SUCCESS_WAIT_TIME = 0.8f;
static constexpr float TUTORIAL_SKIP_HOLD_TIME = 0.7f;

static constexpr int TUTORIAL_CSV_COLUMN_COUNT = 10;

// 通しプレイ
static constexpr float FINAL_NOTE_INTERVAL = 1.0f;
static constexpr float FINAL_NOTE_LEAD_TIME = 2.0f;
static constexpr float FINAL_PRACTICE_END_TIME = 10.0f;
static constexpr float FINAL_PRACTICE_DAMAGE_MULTIPLIER = 3.0f; // 通しプレイ中の攻撃倍率

// 描画
static constexpr float FOCUS_RECT_THICKNESS_RATE = 0.006f; // 線の太さ比率
static constexpr float JUDGE_GUIDE_SIZE_RATE = 0.11f;
static constexpr float JUDGE_GUIDE_ALPHA = 0.5f;

// 型定義
// CSVデータ
enum class TutorialSection
{
	None,
	Intro,
	UI,
	Notes,
	Fever,
	FinalPractice,
	Ending
};

enum class TutorialFace
{
	Smile,
	Serious
};

struct TutorialMessage
{
	int id = 0;
	TutorialSection section = TutorialSection::None;

	std::string step;
	std::string speaker;
	std::string face;
	std::string voice;
	std::string lang;

	std::wstring text;

	std::string nextCondition;
	std::string command;
};

// 注目枠
enum class TutorialFocusTarget
{
	None,
	Player,
	Enemy,
	PlayerHP,
	EnemyHP,
	FeverGauge,
	ScoreRank,
	Combo,
	Lane,
	Judge
};

struct TutorialFocusRect
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
};

// ボイス対応表
struct TutorialVoiceEntry
{
	const char* voiceName;
	Audio_ID audioId;
};

// グローバル変数
// チュートリアル内の経過時間
static float g_currentTime = 0.0f;
// フェードイン完了後にBGM再生を開始したか
static bool g_GameStart = false;
// プレイヤー・エネミー
static Player* g_pPlayer = nullptr;
static Enemy* g_pEnemy = nullptr;

// 現在生成されているノーツ
static std::vector<Note> g_notes;

// チュートリアルメッセージ
// 選択言語に対応するCSVメッセージ一覧
static std::vector<TutorialMessage> g_TutorialMessages;
// 現在表示しているメッセージ番号
static int g_CurrentMessageIndex = 0;
// 決定入力が行われたか
static bool g_Decide = false;
// チュートリアル終了処理へ入ったか
static bool g_TutorialEnding = false;

// メッセージ自動進行待機
// 成功後など、次のメッセージへ進むまで待機中か
static bool g_MessageAdvanceWaiting = false;
// 次のメッセージへ進むまでの残り時間
static float g_MessageAdvanceWaitTimer = 0.0f;

// チュートリアル進行条件
// 各練習の成功状態
static bool g_NormalSuccess = false;
static bool g_LongSuccess = false;
static bool g_RapidSuccess = false;
// FEVER発動・敵撃破状態
static bool g_FeverActivated = false;
static bool g_EnemyKilled = false;
// ミス後に現在の練習を再生成する要求
static bool g_RetryRequested = false;

// ゲームプレイ状態
// 判定ラインのX座標
static float g_JudgeX = -1.0f;
// スコア・コンボ・討伐数
static int g_ComboCount_Now = 0;
static int g_ComboCount_Max = 0;
static int g_KillCount = 0;
static int g_GameScore = 0;
// 前フレームのFEVER状態
static bool g_PrevFeverState = false;
// チュートリアル中にFEVERゲージの加算を許可するか
static bool g_FeverGainEnabled = false;

// お手本プレイ
// お手本プレイの動作状態
static bool g_AutoDemoActive = false;
static bool g_AutoDemoComplete = false;
static bool g_AutoDemoTriggered = false;
// お手本で使用するノーツ情報
static int g_AutoDemoType = NOTE_NORMAL;
static int g_AutoDemoLane = 0;
// お手本の判定時刻・終了時刻
static float g_AutoDemoNoteTime = 0.0f;
static float g_AutoDemoEndTime = 0.0f;
// ラピッドノーツのお手本で次に入力する時刻
static float g_AutoDemoNextRapidTime = 0.0f;

// 通しプレイ
// 通しプレイ実行中か
static bool g_FinalPracticeActive = false;
// 固定譜面が終了する時刻
static float g_FinalPracticeFixedNotesEndTime = 0.0f;
// 敵を倒せなかった場合の補助ノーツ生成状態
static bool g_FinalSupportNotesActive = false;
// 次の補助ノーツ生成までの残り時間
static float g_FinalSupportNoteTimer = 0.0f;
// 次に補助ノーツを生成するレーン
static int g_FinalSupportNextLane = 0;

// SECTIONスキップ
// スキップ入力の長押し時間
static float g_SkipHoldTimer = 0.0f;
// 同じ長押し中に複数回スキップしないためのフラグ
static bool g_SkipExecuted = false;

// 表情・注目表示
// 現在表示している表情
static TutorialFace g_RiffFace = TutorialFace::Smile;

static TutorialFace g_RezzFace = TutorialFace::Smile;

static TutorialFocusTarget g_FocusTarget = TutorialFocusTarget::None;

// チュートリアル終了演出
// 終了後の待機状態
static bool g_TutorialEndWaiting = false;
// フェードアウトを開始したか
static bool g_TutorialFadeOutStarted = false;
// フェードアウト開始までの残り時間
static float g_TutorialEndWaitTimer = 0.0f;

// ボイス
// 現在再生中のボイスの音源ID
static int g_CurrentVoiceAudioId = -1;

// CSVのVOICE名とAudio_IDの対応表
static const TutorialVoiceEntry g_TutorialVoiceTable[] =
{
	{ "Rezz001", Rezz_Tutorial_001 },
	{ "Rezz002", Rezz_Tutorial_002 },
	{ "Rezz003", Rezz_Tutorial_003 },
	{ "Rezz004", Rezz_Tutorial_004 },
	{ "Rezz005", Rezz_Tutorial_005 },
	{ "Rezz006", Rezz_Tutorial_006 },
	{ "Rezz007", Rezz_Tutorial_007 },
	{ "Rezz008", Rezz_Tutorial_008 },
	{ "Rezz009", Rezz_Tutorial_009 },
	{ "Rezz010", Rezz_Tutorial_010 },
	{ "Rezz011", Rezz_Tutorial_011 },
	{ "Rezz012", Rezz_Tutorial_012 },
	{ "Rezz013", Rezz_Tutorial_013 },
	{ "Rezz014", Rezz_Tutorial_014 },
	{ "Rezz015", Rezz_Tutorial_015 },
	{ "Rezz016", Rezz_Tutorial_016 },
	{ "Rezz017", Rezz_Tutorial_017 },
	{ "Rezz018", Rezz_Tutorial_018 },

	{ "Riff001", Riff_Tutorial_001 },
	{ "Riff002", Riff_Tutorial_002 },
	{ "Riff003", Riff_Tutorial_003 }
};

// 内部関数
// メッセージ進行
static void Tutorial_NextMessage();
static void Tutorial_EnterCurrentMessage();
static void Tutorial_ExecuteCommand(const std::string& command);
static bool Tutorial_CheckCondition(const std::string& condition);
static void Tutorial_StartMessageAdvanceWait(float waitTime);
static float Tutorial_GetAdvanceWaitTime(const std::string& condition);

// 描画
static void Tutorial_DrawMessageWindow();
static void Tutorial_DrawConversationCharacters(const TutorialMessage& message);
static void Tutorial_DrawFocusTarget();
static void Tutorial_DrawFocusRect(float x, float y, float width, float height);
static TutorialFocusRect Tutorial_GetFocusRect(TutorialFocusTarget target);
static void Tutorial_DrawControlGuide();
static bool Tutorial_ShouldWrapByWord();

// ノーツ・実践
static void Tutorial_SetSuccessFlag(NoteJudgeResult judge);
static void Tutorial_RetryCurrentCommand();
static void Tutorial_UpdateAutoDemoInput(bool triggerLane[2], bool pressedLane[2]);
static bool Tutorial_IsCurrentAutoDemo();
static float Tutorial_GetBaseDamage(float judgeDamageRate);
static int Tutorial_GetRankFromScore(int gameScore);

// SECTIONスキップ
static TutorialSection Tutorial_GetCurrentSection();
static TutorialSection Tutorial_GetNextSection(TutorialSection currentSection);
static int Tutorial_FindSectionStart(TutorialSection section);
static void Tutorial_ResetRuntimeStateForSkip();
static void Tutorial_SkipToSection(TutorialSection section);
static void Tutorial_SkipToNextSection();
static bool Tutorial_UpdateSkipInput(double elapsedTime, bool skipPressed);
static bool Tutorial_CanManualAdvance();
static bool Tutorial_IsTipStep(const std::string& step);
static bool Tutorial_IsConversationStep(const std::string& step);

// 表情
static TutorialFace Tutorial_ParseFace(const std::string& face);
static void Tutorial_ApplyMessageFace(const TutorialMessage& message);
static int Tutorial_GetRiffFaceTexture(TutorialFace face);
static int Tutorial_GetNaviFaceTexture(TutorialFace face);

// ボイス
static Audio_ID Tutorial_GetVoiceAudioID(const std::string& voice);
static void Tutorial_StopVoice();
static void Tutorial_PlayVoice(const std::string& voice);

// CSV
static std::vector<std::string>
SplitCSVLine(const std::string& line);
static std::wstring Utf8ToWstring(const std::string& value);
static void TrimLineEnd(std::string& value);
static TutorialSection Tutorial_ParseSection(const std::string& section);
static void Tutorial_LoadCSV();

void Tutorial_Initialize(){
	// 各機能の初期化
	Bg_Initialize();
	UI_Initialize();

	g_pPlayer = new Player();
	g_pPlayer->Initialize(PlayerCharacter::Riff);

	g_pEnemy = new Enemy();
	g_pEnemy->Initialize(EnemyType::Tutorial);

	// シーン共通状態
	g_currentTime = 0.0f;
	g_GameStart = false;
	g_TutorialEnding = false;

	g_notes.clear();

	// メッセージ進行
	g_TutorialMessages.clear();
	g_CurrentMessageIndex = 0;
	g_Decide = false;

	g_MessageAdvanceWaiting = false;
	g_MessageAdvanceWaitTimer = 0.0f;

	// 進行条件
	g_NormalSuccess = false;
	g_LongSuccess = false;
	g_RapidSuccess = false;
	g_FeverActivated = false;
	g_EnemyKilled = false;
	g_RetryRequested = false;

	// ゲームプレイ状態
	g_JudgeX = Direct3D_GetBackBufferWidth() * 0.25f;

	g_ComboCount_Now = 0;
	g_ComboCount_Max = 0;
	g_KillCount = 0;
	g_GameScore = 0;

	g_PrevFeverState = false;
	g_FeverGainEnabled = false;


	// お手本プレイ
	g_AutoDemoActive = false;
	g_AutoDemoComplete = false;
	g_AutoDemoTriggered = false;

	g_AutoDemoType = NOTE_NORMAL;
	g_AutoDemoLane = 0;

	g_AutoDemoNoteTime = 0.0f;
	g_AutoDemoEndTime = 0.0f;
	g_AutoDemoNextRapidTime = 0.0f;

	// 通しプレイ
	g_FinalPracticeActive = false;
	g_FinalPracticeFixedNotesEndTime = 0.0f;

	g_FinalSupportNotesActive = false;
	g_FinalSupportNoteTimer = 0.0f;
	g_FinalSupportNextLane = 0;


	// SECTIONスキップ
	g_SkipHoldTimer = 0.0f;
	g_SkipExecuted = false;

	// 表情・注目表示
	g_RiffFace = TutorialFace::Smile;
	g_RezzFace = TutorialFace::Smile;

	g_FocusTarget = TutorialFocusTarget::None;

	// 終了演出
	g_TutorialEndWaiting = false;
	g_TutorialFadeOutStarted = false;
	g_TutorialEndWaitTimer = 0.0f;

	// ボイス
	g_CurrentVoiceAudioId = -1;

	// チュートリアルデータ読み込み
	Tutorial_LoadCSV();
	if (!g_TutorialMessages.empty())
		Tutorial_EnterCurrentMessage();

	// フェードイン
	Fade_Start(0.10, false);
}
void Tutorial_Finalize(){
	StopAudio(Resouce_Manager_GetAudioId(Tutorial_BGM));
	Tutorial_StopVoice();
	delete g_pPlayer;
	delete g_pEnemy;
	g_pPlayer = nullptr;
	g_pEnemy = nullptr;
	UI_Finalize();
	Bg_Finalize();
}

void Tutorial_Update(double elapsed_time){
	g_currentTime += (float)elapsed_time;

	if (!g_GameStart && Fade_GetState() == FADE_STATE_FINISHED_IN) {
		PlayAudio(Resouce_Manager_GetAudioId(Tutorial_BGM), true);
		g_GameStart = true;
	}

	if (g_FinalPracticeActive && g_pEnemy->IsDead() && !g_EnemyKilled) {
		g_KillCount++;
		g_GameScore += GAMESCORE_KILL;
		g_EnemyKilled = true;

		g_FinalSupportNotesActive = false;
		g_FinalSupportNoteTimer = 0.0f;

		g_notes.clear();
	}

	// 入力管理
	g_Decide = KeyLogger_IsTrigger(KK_ENTER) ||
		(PadLogger_IsConnected() && PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_A));

	bool skipPressed = KeyLogger_IsPressed(KK_J) ||
		(PadLogger_IsConnected() && PadLogger_IsPressed(0, SDL_CONTROLLER_BUTTON_B));

	bool triggerUp =
		KeyLogger_IsTrigger(KK_W) ||
		(PadLogger_IsConnected() && PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_Y));

	bool triggerDown =
		KeyLogger_IsTrigger(KK_S) ||
		(PadLogger_IsConnected() && PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_X));

	bool pressedUp =
		KeyLogger_IsPressed(KK_W) ||
		(PadLogger_IsConnected() && PadLogger_IsPressed(0, SDL_CONTROLLER_BUTTON_Y));

	bool pressedDown =
		KeyLogger_IsPressed(KK_S) ||
		(PadLogger_IsConnected() && PadLogger_IsPressed(0, SDL_CONTROLLER_BUTTON_X));

	bool triggerLane[2] = {triggerUp,triggerDown};
	bool pressedLane[2] = { pressedUp, pressedDown};

	Tutorial_UpdateAutoDemoInput(triggerLane,pressedLane); // お手本プレイ

	for (auto& n : g_notes) {
		NoteJudgeResult judge =
			n.UpdateJudge(g_currentTime, triggerLane, pressedLane);

		bool isAutoDemo = Tutorial_IsCurrentAutoDemo();

		Tutorial_SetSuccessFlag(judge);

		switch (judge) {
		case NoteJudgeResult::Note_Perfect:
			UI_SetJudge(Judge_Perfect);
			Combo_Add();
			if (!isAutoDemo) {
				g_ComboCount_Now++;
				g_GameScore += GAMESCORE_PERFECT;

				g_pPlayer->OnNoteSuccess(true, g_FeverGainEnabled);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(Tutorial_GetBaseDamage(1.0f)));
			}
			break;

		case NoteJudgeResult::Note_Great:
			UI_SetJudge(Judge_Great);
			Combo_Add();

			if (!isAutoDemo) {
				g_ComboCount_Now++;
				g_GameScore += GAMESCORE_GREAT;

				g_pPlayer->OnNoteSuccess(false, g_FeverGainEnabled);
				Combo_SetPerfectChain(false);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(Tutorial_GetBaseDamage(0.5f)));
			}
			
			break;

		case NoteJudgeResult::Note_LongTick:
			UI_SetJudge(Judge_Perfect);
			Combo_Add();

			if (!isAutoDemo) {
				g_ComboCount_Now++;
				g_GameScore += GAMESCORE_PERFECT;

				g_pPlayer->OnNoteSuccess(true, g_FeverGainEnabled);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(Tutorial_GetBaseDamage(0.5f)));
			}
		
			break;

		case NoteJudgeResult::Note_RapidTick:
			UI_SetJudge(Judge_Perfect);
			Combo_Add();

			if (!isAutoDemo) {
				g_ComboCount_Now++;
				g_GameScore += GAMESCORE_PERFECT;

				g_pPlayer->OnNoteSuccess(true, g_FeverGainEnabled);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(Tutorial_GetBaseDamage(0.5f)));
			}
			
			break;

		case NoteJudgeResult::Note_Miss:
			UI_SetJudge(Judge_Miss);

			if (!isAutoDemo) {
				if (g_ComboCount_Now > g_ComboCount_Max)
					g_ComboCount_Max = g_ComboCount_Now;

				g_ComboCount_Now = 0;

				Combo_Reset();
				Combo_SetPerfectChain(false);

				g_RetryRequested = true;
			}
			break;
		}
	}

	if (g_RetryRequested) {
		g_RetryRequested = false;
		Tutorial_RetryCurrentCommand();
	}

	// 通しプレイ用のノーツ再生成
	if (g_FinalPracticeActive && !g_EnemyKilled && g_currentTime >= g_FinalPracticeFixedNotesEndTime) {
		g_FinalSupportNotesActive = true;
	}

	if (g_FinalSupportNotesActive && !g_EnemyKilled) {
		g_FinalSupportNoteTimer -= static_cast<float>(elapsed_time);

		if (g_FinalSupportNoteTimer <= 0.0f){
			Note n;
			n.Initialize(
				g_currentTime + FINAL_NOTE_LEAD_TIME,
				g_FinalSupportNextLane,
				NOTE_NORMAL,
				0.0f
			);

			g_notes.push_back(n);

			// 上下を交互にする
			g_FinalSupportNextLane = 1 - g_FinalSupportNextLane;
			g_FinalSupportNoteTimer = FINAL_NOTE_INTERVAL;
		}
	}

	Bg_Update(elapsed_time);
	g_pPlayer->Update(elapsed_time);
	bool currentFever = g_pPlayer->IsFever();

	if (!g_PrevFeverState && currentFever) {
		UI_ShowSkillPopup(L"ATTACK UP!!");
		g_FeverActivated = true;
	}
	g_PrevFeverState = currentFever;

	g_pEnemy->Update(elapsed_time);

	// Note更新
	for (auto& n : g_notes) {
		n.SetNoteSpeed(NOTE_SPEED_DEFAULT* Configu_GetNoteSpeed());

		n.Update(g_currentTime, g_JudgeX);
	}

	// UI情報のセット
	UI_SetPlayerHpRate(g_pPlayer->GetHp() / g_pPlayer->GetMaxHP());
	UI_SetEnemyHpRate(g_pEnemy->GetHp() / g_pEnemy->GetMaxHp());
	UI_SetPlayerFeverRate(g_pPlayer->GetFeverGauge() / g_pPlayer->GetFeverGaugeMAX());

	UI_SetKillCount(g_KillCount);
	UI_SetScore(g_GameScore);
	UI_SetRank((Rank_Type)Tutorial_GetRankFromScore(g_GameScore));
	// セット後に更新
	UI_Update(elapsed_time);	

	// SECTIONスキップ
	bool skippedThisFrame = Tutorial_UpdateSkipInput(elapsed_time, skipPressed);
	
	if (skippedThisFrame && !g_TutorialEnding){
		Tutorial_SkipToNextSection();
	}

	if (!skippedThisFrame) {
		// メッセージ進行待ち中
		if (g_MessageAdvanceWaiting) {
			g_MessageAdvanceWaitTimer -= (float)elapsed_time;

			if (g_MessageAdvanceWaitTimer <= 0.0f) {
				g_MessageAdvanceWaiting = false;
				Tutorial_NextMessage();
			}
		}
		// 通常の進行判定
		else if (!g_TutorialEnding &&
			!g_TutorialMessages.empty() &&
			g_CurrentMessageIndex < (int)g_TutorialMessages.size())
		{
			const TutorialMessage& msg = g_TutorialMessages[g_CurrentMessageIndex];

			if (Tutorial_CheckCondition(msg.nextCondition)) {
				float waitTime = Tutorial_GetAdvanceWaitTime(msg.nextCondition);

				Tutorial_StartMessageAdvanceWait(waitTime);
			}
		}
	}
	// チュートリアル終了前の猶予時間
	if (g_TutorialEndWaiting){
		g_TutorialEndWaitTimer -= (float)elapsed_time;

		if (g_TutorialEndWaitTimer <= 0.0f){
			g_TutorialEndWaiting = false;
			g_TutorialFadeOutStarted = true;

			Fade_Start(0.5, true, { 0.0f, 0.0f, 0.0f });
		}
	}

	if (g_TutorialFadeOutStarted && Fade_GetState() == FADE_STATE_FINISHED_OUT) {
		Scene_Change(SCENE_MUSIC_SELEC);
	}

	if (g_GameStart && Fade_GetState() == FADE_STATE_FINISHED_OUT) {
		g_GameStart = false;

	}

}

void Tutorial_Draw(){
	// ゲーム画面の描画
	Bg_Draw();
	UI_Draw();
	// 判定位置の描画
	float w = (float)Direct3D_GetBackBufferWidth();
	float h = (float)Direct3D_GetBackBufferHeight();

	float judgeX = w * 0.25f;
	float noteSize = h * 0.15f;

	int Judge_Up_TexId = -1;
	int Judge_Down_TexId = -1;

	if (!PadLogger_IsConnected()) {
		Judge_Up_TexId = Resouce_Manager_GetTexId(Judge_W);
		Judge_Down_TexId = Resouce_Manager_GetTexId(Judge_S);

	}
	else {
		switch (PadLogger_GetControllerDisplayType())
		{
		case ControllerDisplayType::PlayStation:
			Judge_Up_TexId = Resouce_Manager_GetTexId(Judge_Triangle);
			Judge_Down_TexId = Resouce_Manager_GetTexId(Judge_Square);
			break;

		case ControllerDisplayType::Xbox:
		case ControllerDisplayType::Unknown:
		default:
			Judge_Up_TexId = Resouce_Manager_GetTexId(Judge_Y);
			Judge_Down_TexId = Resouce_Manager_GetTexId(Judge_X);

			break;
		}
	}
	float Judge_TexSize = h * 0.11f;
	float Judge_BasePoint = h * 0.5f;
	Sprite_Draw(
		Judge_Up_TexId,
		judgeX - (Judge_TexSize * 0.5f), Judge_BasePoint - h * 0.1f - (Judge_TexSize * 0.5f),
		Judge_TexSize, Judge_TexSize,
		{ 1.0f,1.0f,1.0f,0.5f }
	);
	Sprite_Draw(
		Judge_Down_TexId,
		judgeX - (Judge_TexSize * 0.5f), Judge_BasePoint + h * 0.1f - (Judge_TexSize * 0.5f),
		Judge_TexSize, Judge_TexSize,
		{ 1.0f,1.0f,1.0f,0.5f }
	);

	// ノーツの描画
	for (auto& n : g_notes) {
		n.Draw(g_currentTime);
	}

	// キャラクターの描画
	g_pPlayer->Draw();

	// エネミーの描画
	g_pEnemy->Draw();

	// チュートリアル固有UIの描画
	// 注目表示の描画
	Tutorial_DrawFocusTarget();
	// メッセージボックスの描画
	Tutorial_DrawMessageWindow();
	// スキップ操作説明の描画
	Tutorial_DrawControlGuide();
}

void Tutorial_End(){
	if (g_TutorialEnding) return;
	Tutorial_StopVoice();
	g_TutorialEnding = true;
	g_TutorialEndWaiting = true;
	g_TutorialFadeOutStarted = false;
	g_TutorialEndWaitTimer = TUTORIAL_END_WAIT_TIME;
}

static int Tutorial_GetRankFromScore(int game_score){
	if (game_score >= 100000) return Rank_S;
	if (game_score >= 50000) return Rank_A;
	if (game_score >= 20000) return Rank_B;
	if (game_score >= 10000) return Rank_C;

	return Rank_D;
}

void Tutorial_NextMessage(){
	g_CurrentMessageIndex++;

	g_NormalSuccess = false;
	g_LongSuccess = false;
	g_RapidSuccess = false;
	g_FeverActivated = false;
	g_EnemyKilled = false;

	g_AutoDemoActive = false;
	g_AutoDemoComplete = false;
	g_AutoDemoTriggered = false;

	if (g_CurrentMessageIndex >= (int)g_TutorialMessages.size()){
		Tutorial_End();
		return;
	}

	Tutorial_EnterCurrentMessage();
}

void Tutorial_ExecuteCommand(const std::string& command){

	if (command == "None") { 
		return;

	} else if (command == "FocusPlayer") {
		g_FocusTarget = TutorialFocusTarget::Player;

	} else if (command == "FocusEnemy") {
		g_FocusTarget = TutorialFocusTarget::Enemy;

	} else if (command == "FocusPlayerHP") {
		g_FocusTarget = TutorialFocusTarget::PlayerHP;

	} else if (command == "FocusEnemyHP") {
		g_FocusTarget = TutorialFocusTarget::EnemyHP;

	} else if (command == "FocusFeverGauge") {
		g_FocusTarget = TutorialFocusTarget::FeverGauge;

	} else if (command == "FocusScoreRank") {
		g_FocusTarget = TutorialFocusTarget::ScoreRank;

	} else if (command == "FocusCombo") {
		g_FocusTarget = TutorialFocusTarget::Combo;

	} else if (command == "FocusLane") {
		g_FocusTarget = TutorialFocusTarget::Lane;

	} else if (command == "FocusJudge") {
		g_FocusTarget = TutorialFocusTarget::Judge;

	} else if (command == "ClearFocus") {
		g_FocusTarget = TutorialFocusTarget::None;

	} else if (command == "SpawnNormalPractice") {
		Note n;
		n.Initialize(
			g_currentTime + 2.0f,
			0,
			NOTE_NORMAL,
			0.0f
		);
		g_notes.push_back(n);

	} else if (command == "SpawnLongPractice") {
		Note n;
		n.Initialize(
			g_currentTime + 2.0f,
			0,
			NOTE_LONG,
			2.0f
		);
		g_notes.push_back(n);

	} else if (command == "SpawnRapidPractice") {
		Note n;
		n.Initialize(
			g_currentTime + 2.0f,
			1,
			NOTE_RAPID,
			2.0f
		);
		g_notes.push_back(n);

	} else if (command == "SpawnNormalDemoAuto"){
		Note n;

		g_AutoDemoType = NOTE_NORMAL;
		g_AutoDemoLane = 0;
		g_AutoDemoNoteTime = g_currentTime + 2.0f;
		g_AutoDemoEndTime = g_AutoDemoNoteTime;

		g_AutoDemoTriggered = false;
		g_AutoDemoComplete = false;
		g_AutoDemoActive = true;

		n.Initialize(
			g_AutoDemoNoteTime,
			g_AutoDemoLane,
			NOTE_NORMAL,
			0.0f
		);

		g_notes.push_back(n);

	} else if (command == "SpawnLongDemoAuto"){
		Note n;

		g_AutoDemoType = NOTE_LONG;
		g_AutoDemoLane = 0;
		g_AutoDemoNoteTime = g_currentTime + 2.0f;
		g_AutoDemoEndTime = g_AutoDemoNoteTime + 2.0f;

		g_AutoDemoTriggered = false;
		g_AutoDemoComplete = false;
		g_AutoDemoActive = true;

		n.Initialize(
			g_AutoDemoNoteTime,
			g_AutoDemoLane,
			NOTE_LONG,
			2.0f
		);

		g_notes.push_back(n);
	}
	else if (command == "SpawnRapidDemoAuto") {
		Note n;

		g_AutoDemoType = NOTE_RAPID;
		g_AutoDemoLane = 1;
		g_AutoDemoNoteTime = g_currentTime + 2.0f;
		g_AutoDemoEndTime = g_AutoDemoNoteTime + 2.0f;

		g_AutoDemoTriggered = false;
		g_AutoDemoComplete = false;
		g_AutoDemoActive = true;
		g_AutoDemoNextRapidTime = g_AutoDemoNoteTime;

		n.Initialize(
			g_AutoDemoNoteTime,
			g_AutoDemoLane,
			NOTE_RAPID,
			2.0f
		);

		g_notes.push_back(n);

	} else if (command == "StartFeverPractice") {
		g_FeverGainEnabled = true; // ゲージの加算許可

		// 次の成功で最大になるようにする
		float maxGauge = g_pPlayer->GetFeverGaugeMAX();
		g_pPlayer->SetFeverGauge(maxGauge - 1.0f);

		// FEVER検出用の直前状態を更新
		g_PrevFeverState = g_pPlayer->IsFever();

		// 練習用通常ノーツを1つ生成
		Note n;
		n.Initialize(
			g_currentTime + 2.0f,
			0,
			NOTE_NORMAL,
			0.0f
		);

		g_notes.push_back(n);

		g_FeverActivated = g_PrevFeverState;

	} else if (command == "StartFinalPractice") {
		g_FinalPracticeFixedNotesEndTime = g_currentTime + FINAL_PRACTICE_END_TIME;
		g_FinalSupportNotesActive = false;
		g_FinalSupportNoteTimer = 0.0f;
		g_FinalSupportNextLane = 0;
		g_FocusTarget = TutorialFocusTarget::None;

		g_FinalPracticeActive = true;

		g_EnemyKilled = false;
		g_RetryRequested = false;

		// 通しプレイではゲージ蓄積を許可
		g_FeverGainEnabled = true;

		// ゲーム状況をリセット
		g_GameScore = 0;
		g_ComboCount_Now = 0;
		g_ComboCount_Max = 0;
		Combo_Reset();
		Combo_SetPerfectChain(true);
		g_pPlayer->SetFeverGauge(0.0f);
		g_PrevFeverState = g_pPlayer->IsFever();

		// 使用済みノーツを削除
		g_notes.clear();

		// 通常ノーツ
		{
			Note n;
			n.Initialize(
				g_currentTime + 2.0f,
				0,
				NOTE_NORMAL,
				0.0f
			);
			g_notes.push_back(n);
		}

		// 通常ノーツ
		{
			Note n;
			n.Initialize(
				g_currentTime + 3.0f,
				1,
				NOTE_NORMAL,
				0.0f
			);
			g_notes.push_back(n);
		}

		// ロングノーツ
		{
			Note n;
			n.Initialize(
				g_currentTime + 4.0f,
				0,
				NOTE_LONG,
				2.0f
			);
			g_notes.push_back(n);
		}

		// 連打ノーツ
		{
			Note n;
			n.Initialize(
				g_currentTime + 7.0f,
				1,
				NOTE_RAPID,
				2.0f
			);
			g_notes.push_back(n);
		}

	} else if (command == "EndFinalPractice") {
		g_FinalPracticeActive = false;
		g_FeverGainEnabled = false;
		g_notes.clear();

	} else if (command == "EndTutorial") {
		Tutorial_End();
	}
}

bool Tutorial_CheckCondition(const std::string& condition){
	if (condition == "Click")
		return g_Decide;

	if (condition == "NormalSuccess")
		return g_NormalSuccess;

	if (condition == "LongSuccess")
		return g_LongSuccess;

	if (condition == "RapidSuccess")
		return g_RapidSuccess;

	if (condition == "FeverActivated")
		return g_FeverActivated;

	if (condition == "EnemyKilled")
		return g_EnemyKilled;

	if (condition == "AutoDemoComplete")
		return g_AutoDemoComplete;

	return false;
}

static TutorialFocusRect Tutorial_GetFocusRect(TutorialFocusTarget target){
	switch (target)
	{
	case TutorialFocusTarget::Player:
		return { 0.015f, 0.25f, 0.20f, 0.45f };

	case TutorialFocusTarget::Enemy:
		return { 0.72f, 0.25f, 0.28f, 0.40f };

	case TutorialFocusTarget::PlayerHP:
		return { 0.00f, 0.10f, 0.30f, 0.05f };

	case TutorialFocusTarget::EnemyHP:
		return { 0.70f, 0.10f, 0.30f, 0.05f };

	case TutorialFocusTarget::FeverGauge:
		return { 0.00f, 0.15f, 0.30f, 0.05f };

	case TutorialFocusTarget::Combo:
		return { 0.40f, 0.05f, 0.30f, 0.18f };

	case TutorialFocusTarget::ScoreRank:
		return { 0.00f, 0.00f, 0.36f, 0.09f };

	case TutorialFocusTarget::Lane:
		return { 0.20f, 0.33f, 0.75f, 0.35f };

	case TutorialFocusTarget::Judge:
		return { 0.21f, 0.32f, 0.08f, 0.35f };

	case TutorialFocusTarget::None:
	default:
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}
}

void Tutorial_DrawFocusRect(float x, float y, float width, float height){
	float blink = (std::sinf(g_currentTime * 5.0f) + 1.0f) * 0.5f;

	float alpha = 0.55f + blink * 0.45f;

	int textureId = Resouce_Manager_GetTexId(Color_White);

	XMFLOAT4 red = { 1.0f,0.05f,0.05f,alpha };

	float thickness = (float)Direct3D_GetBackBufferHeight() * FOCUS_RECT_THICKNESS_RATE;

	// 上辺
	Sprite_Draw(textureId, x, y, width, thickness, red);
	// 下辺
	Sprite_Draw(textureId, x, y + height - thickness, width, thickness, red);
	// 左辺
	Sprite_Draw(textureId, x, y + thickness, thickness, height - thickness * 2, red);
	// 右辺
	Sprite_Draw(textureId, x + width - thickness, y + thickness, thickness, height - thickness * 2, red);
}

void Tutorial_DrawFocusTarget(){
	if (g_FocusTarget == TutorialFocusTarget::None)
		return;

	TutorialFocusRect rect = Tutorial_GetFocusRect(g_FocusTarget);

	if (rect.width <= 0.0f || rect.height <= 0.0f)
		return;

	float w = (float)Direct3D_GetBackBufferWidth();

	float h = (float)Direct3D_GetBackBufferHeight();

	Tutorial_DrawFocusRect(
		rect.x * w, rect.y * h,
		rect.width * w, rect.height * h
	);
}

void Tutorial_EnterCurrentMessage(){
	if (g_TutorialMessages.empty())
		return;

	if (g_CurrentMessageIndex < 0 || g_CurrentMessageIndex >= static_cast<int>(g_TutorialMessages.size()))
		return;

	const TutorialMessage& msg =g_TutorialMessages[g_CurrentMessageIndex];

	// 表情同期
	Tutorial_ApplyMessageFace(msg);

	// ボイス再生
	Tutorial_PlayVoice(msg.voice);

	// Command実行
	Tutorial_ExecuteCommand(msg.command);
}

void Tutorial_DrawMessageWindow()
{
	if (g_TutorialMessages.empty())
		return;

	if (g_CurrentMessageIndex >= (int)g_TutorialMessages.size())
		return;

	const TutorialMessage& msg = g_TutorialMessages[g_CurrentMessageIndex];

	float w = (float)Direct3D_GetBackBufferWidth();
	float h = (float)Direct3D_GetBackBufferHeight();

	float blink = (std::sinf(g_currentTime * 6.0f) + 1.0f) * 0.5f;

	float alpha = 0.35f + blink * 0.65f;

	Text_Begin();

	// 会話パート
	if (Tutorial_IsConversationStep(msg.step)){
		// 暗幕
		Sprite_Draw(
			Resouce_Manager_GetTexId(Color_Black),
			0.0f, 0.0f,
			w, h,
			{ 1.0f, 1.0f, 1.0f, 0.75f }
		);

		// 立ち絵の表示
		Tutorial_DrawConversationCharacters(msg);

		// 大きい会話用テキストボックス
		Sprite_Draw(
			Resouce_Manager_GetTexId(TextBox),
			w * 0.05f, h * 0.65f,
			w * 0.90f, h * 0.25f
		);

		// 話者名
		std::wstring speakerW(
			msg.speaker.begin(),
			msg.speaker.end()
		);

		Text_Draw(
			speakerW.c_str(),
			w * 0.08f, h * 0.68f,
			XMVECTORF32{ 1, 1, 0, 1 },
			1.2f
		);

		// 本文
		const float conversationMaxWidth = w * 0.84f;

		std::wstring wrappedText =
			Text_Wrap(
				msg.text,
				conversationMaxWidth,
				Tutorial_ShouldWrapByWord()
			);

		Text_Draw(
			wrappedText.c_str(),
			w * 0.08f, h * 0.75f,
			XMVECTORF32{ 1, 1, 1, 1 }
		);
	}
	// 操作説明・TIPSパート
	else if (Tutorial_IsTipStep(msg.step))
	{
		// 小さいTIPS用ボックス
		Sprite_Draw(
			Resouce_Manager_GetTexId(TextBox),
			w * 0.18f, h * 0.78f,
			w * 0.64f, h * 0.16f
		);


		if (Tutorial_IsCurrentAutoDemo()) {
			Text_Draw(
				L"AUTO PLAY",
				w * 0.68f, h * 0.80f,
				XMVECTORF32{ 0.5f, 1.0f, 1.0f, alpha },
				0.9f
			);
		}
		Text_Draw(
			L"TIPS",
			w * 0.20f, h * 0.80f,
			XMVECTORF32{ 1, 1, 0, 1 },
			1.0f
		);

		const float tipsMaxWidth = w * 0.58f;

		std::wstring wrappedText =
			Text_Wrap(
				msg.text,
				tipsMaxWidth,
				Tutorial_ShouldWrapByWord()
			);

		Text_Draw(
			wrappedText.c_str(),
			w * 0.20f, h * 0.85f,
			XMVECTORF32{ 1, 1, 1, 1 }
		);
	}
	// その他
	else
	{
		Sprite_Draw(
			Resouce_Manager_GetTexId(TextBox),
			w * 0.05f, h * 0.70f,
			w * 0.90f, h * 0.25f
		);

		Text_Draw(
			msg.text.c_str(),
			w * 0.08f, h * 0.78f,
			XMVECTORF32{ 1, 1, 1, 1 },
			1.0f
		);
	}

	Text_End();
}

bool Tutorial_ShouldWrapByWord(){
	return Configu_GetLanguage() == Language::English;
}

void Tutorial_DrawConversationCharacters(const TutorialMessage& msg){
	float w = (float)Direct3D_GetBackBufferWidth();

	float h = (float)Direct3D_GetBackBufferHeight();

	// 発言者判定
	bool riffSpeaking = msg.speaker == "RIFF";
	bool naviSpeaking = msg.speaker == "REZZ";

	// 発言者は明るく、非発言者は少し暗く
	XMFLOAT4 riffColor = riffSpeaking ? XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f }
									  : XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f };

	XMFLOAT4 naviColor = naviSpeaking ? XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f }
									  : XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f };

	// リフ：左
	Sprite_Draw(
		Tutorial_GetRiffFaceTexture(g_RiffFace),
		w * 0.02f, h * 0.10f,
		w * 0.40f, h * 1.00f,
		riffColor,
		true
	);

	// ナビ：右
	Sprite_Draw(
		Tutorial_GetNaviFaceTexture(g_RezzFace),
		w * 0.70f , h * 0.20f,
		w * 0.25f , h * 0.70f,
		naviColor
	);
}

void Tutorial_SetSuccessFlag(NoteJudgeResult judge){
	if (g_TutorialMessages.empty()) return;

	if (g_CurrentMessageIndex < 0 || g_CurrentMessageIndex >= (int)g_TutorialMessages.size()) return;

	const std::string& condition = g_TutorialMessages[g_CurrentMessageIndex].nextCondition;

	if (condition == "NormalSuccess") {
		if (judge == NoteJudgeResult::Note_Perfect || judge == NoteJudgeResult::Note_Great) {
			g_NormalSuccess = true;
		}
	} else if (condition == "LongSuccess") {
		if (judge == NoteJudgeResult::Note_LongComplete) {
			g_LongSuccess = true;
		}
	} else if (condition == "RapidSuccess") {
		if (judge == NoteJudgeResult::Note_RapidComplete) {
			g_RapidSuccess = true;
		}
	} else if (condition == "AutoDemoComplete") {
		if (g_AutoDemoType == NOTE_NORMAL) {
			if (judge == NoteJudgeResult::Note_Perfect || judge == NoteJudgeResult::Note_Great){
				g_AutoDemoComplete = true;
				g_AutoDemoActive = false;
			}
		} else if (g_AutoDemoType == NOTE_LONG) {
			if (judge == NoteJudgeResult::Note_LongComplete) {
				g_AutoDemoComplete = true;
				g_AutoDemoActive = false;
			}
		} else if (g_AutoDemoType == NOTE_RAPID) {
			if (judge == NoteJudgeResult::Note_RapidComplete){
				g_AutoDemoComplete = true;
				g_AutoDemoActive = false;
			}
		}
	}
}

bool Tutorial_IsConversationStep(const std::string& step){
	return step == "Story" || step == "Ending";
}

bool Tutorial_IsTipStep(const std::string& step){
	return step == "UI" ||
		step == "Normal" ||
		step == "Long" ||
		step == "Rapid" ||
		step == "Fever" ||
		step == "Practice";
}

void Tutorial_RetryCurrentCommand(){
	if (g_TutorialMessages.empty())
		return;

	if (g_CurrentMessageIndex < 0 || g_CurrentMessageIndex >= (int)g_TutorialMessages.size())
		return;

	const TutorialMessage& msg = g_TutorialMessages[g_CurrentMessageIndex];

	if (msg.command == "SpawnNormalPractice" ||
		msg.command == "SpawnLongPractice" ||
		msg.command == "SpawnRapidPractice" || 
		msg.command == "StartFeverPractice")
	{
		Tutorial_ExecuteCommand(msg.command);
	}
}

void Tutorial_UpdateAutoDemoInput(bool triggerLane[2], bool pressedLane[2]) {
	if (!g_AutoDemoActive)
		return;

	// お手本中は実入力を無効化して、自動入力だけにする
	triggerLane[0] = false;
	triggerLane[1] = false;
	pressedLane[0] = false;
	pressedLane[1] = false;

	switch (g_AutoDemoType)
	{
	case NOTE_NORMAL:
		// 判定時刻に1回だけ押す
		if (!g_AutoDemoTriggered &&
			g_currentTime >= g_AutoDemoNoteTime){
			triggerLane[g_AutoDemoLane] = true;
			pressedLane[g_AutoDemoLane] = true;

			g_AutoDemoTriggered = true;
		}
		break;

	case NOTE_LONG:
		// 最初に1回押して、その後終了時刻まで押しっぱなし
		if (!g_AutoDemoTriggered &&g_currentTime >= g_AutoDemoNoteTime){
			triggerLane[g_AutoDemoLane] = true;
			pressedLane[g_AutoDemoLane] = true;

			g_AutoDemoTriggered = true;
		}else if (g_AutoDemoTriggered &&g_currentTime < g_AutoDemoEndTime){
			pressedLane[g_AutoDemoLane] = true;
		}
		break;

	case NOTE_RAPID:
		// 区間中、一定間隔でトリガーを出す
		if (g_currentTime >= g_AutoDemoNoteTime &&
			g_currentTime <= g_AutoDemoEndTime){
			if (g_currentTime >= g_AutoDemoNextRapidTime){
				triggerLane[g_AutoDemoLane] = true;
				pressedLane[g_AutoDemoLane] = true;

				g_AutoDemoNextRapidTime += 0.20f;
			}
		}
		break;
	}
}

bool Tutorial_IsCurrentAutoDemo(){
	if (g_TutorialMessages.empty())
		return false;

	if (g_CurrentMessageIndex < 0 || g_CurrentMessageIndex >= (int)g_TutorialMessages.size())
		return false;

	const TutorialMessage& msg = g_TutorialMessages[g_CurrentMessageIndex];

	return msg.nextCondition == "AutoDemoComplete";
}

float Tutorial_GetBaseDamage(float judgeDamageRate){
	float damage = BASE_DAMAGE * judgeDamageRate;

	// 通しプレイ中だけダメージを増やす
	if (g_FinalPracticeActive)
		damage *= FINAL_PRACTICE_DAMAGE_MULTIPLIER;

	return damage;
}

void Tutorial_StartMessageAdvanceWait(float waitTime){
	if (waitTime <= 0.0f){
		Tutorial_NextMessage();
		return;
	}

	g_MessageAdvanceWaiting = true;
	g_MessageAdvanceWaitTimer = waitTime;
}

float Tutorial_GetAdvanceWaitTime(const std::string& condition){
	// 会話送りはすぐ進める
	if (condition == "Click")
		return 0.0f;

	// お手本終了後、少し見せる
	if (condition == "AutoDemoComplete")
		return TUTORIAL_SUCCESS_WAIT_TIME;

	// 実践成功後、少し余韻を残す
	if (condition == "NormalSuccess" ||
		condition == "LongSuccess" ||
		condition == "RapidSuccess")
		return TUTORIAL_SUCCESS_WAIT_TIME;

	if (condition == "FeverActivated" ||
		condition == "EnemyKilled")
		return 1.0f;

	return 0.0f;
}

TutorialSection Tutorial_GetCurrentSection(){
	if (g_TutorialMessages.empty())
		return TutorialSection::None;

	return g_TutorialMessages[g_CurrentMessageIndex].section;
}

int Tutorial_FindSectionStart(TutorialSection section){
	for (int i = 0; i < static_cast<int>(g_TutorialMessages.size()); i++){
		if (g_TutorialMessages[i].section == section){
			return i;
		}
	}

	return -1;
}

TutorialSection Tutorial_GetNextSection(TutorialSection currentSection){
	switch (currentSection)
	{
	case TutorialSection::Intro:
		return TutorialSection::UI;

	case TutorialSection::UI:
		return TutorialSection::Notes;

	case TutorialSection::Notes:
		return TutorialSection::Fever;

	case TutorialSection::Fever:
		return TutorialSection::FinalPractice;

	case TutorialSection::FinalPractice:
		return TutorialSection::Ending;

	case TutorialSection::Ending:
		return TutorialSection::None;

	case TutorialSection::None:
	default:
		return TutorialSection::None;
	}
}

void Tutorial_ResetRuntimeStateForSkip(){
	// CVの停止
	Tutorial_StopVoice();

	// 進行中のノーツを削除
	g_notes.clear();

	// 注目枠を消す
	g_FocusTarget = TutorialFocusTarget::None;

	// お手本プレイを停止
	g_AutoDemoActive = false;
	g_AutoDemoComplete = false;
	g_AutoDemoTriggered = false;

	g_AutoDemoNoteTime = 0.0f;
	g_AutoDemoEndTime = 0.0f;
	g_AutoDemoNextRapidTime = 0.0f;

	// 成功条件をリセット
	g_NormalSuccess = false;
	g_LongSuccess = false;
	g_RapidSuccess = false;
	g_FeverActivated = false;
	g_EnemyKilled = false;

	// 再試行要求を解除
	g_RetryRequested = false;

	// メッセージ待機を解除
	g_MessageAdvanceWaiting = false;
	g_MessageAdvanceWaitTimer = 0.0f;

	// 通しプレイ状態を解除
	g_FinalPracticeActive = false;

	// FEVER加算を一度停止
	g_FeverGainEnabled = false;

	// コンボ状態をリセット
	g_ComboCount_Now = 0;
	g_ComboCount_Max = 0;

	// 通しプレイ用変数のリセット
	g_FinalSupportNotesActive = false;
	g_FinalSupportNoteTimer = 0.0f;
	g_FinalSupportNextLane = 0;
	g_FinalPracticeFixedNotesEndTime = 0.0f;

	Combo_Reset();
	Combo_SetPerfectChain(true);
}

void Tutorial_SkipToSection(TutorialSection section){
	int targetIndex = Tutorial_FindSectionStart(section);

	if (targetIndex < 0)
		return;

	Tutorial_ResetRuntimeStateForSkip();

	g_CurrentMessageIndex = targetIndex;

	Tutorial_EnterCurrentMessage();
}

void Tutorial_SkipToNextSection(){
	TutorialSection currentSection = Tutorial_GetCurrentSection();

	TutorialSection nextSection = Tutorial_GetNextSection(currentSection);

	// Ending中にスキップした場合は終了
	if (nextSection == TutorialSection::None){
		Tutorial_ResetRuntimeStateForSkip();
		Tutorial_End();
		return;
	}

	Tutorial_SkipToSection(nextSection);
}

bool Tutorial_UpdateSkipInput(double elapsedTime, bool skipPressed){
	if (g_TutorialEnding){
		g_SkipHoldTimer = 0.0f;
		g_SkipExecuted = false;
		return false;
	}

	if (!skipPressed){ // キーを離したら再びスキップ可能
		g_SkipHoldTimer = 0.0f;
		g_SkipExecuted = false;
		return false;
	}

	// すでに実行済み
	if (g_SkipExecuted) return false;

	g_SkipHoldTimer += static_cast<float>(elapsedTime);

	if (g_SkipHoldTimer >= TUTORIAL_SKIP_HOLD_TIME){
		g_SkipExecuted = true;
		g_SkipHoldTimer = 0.0f;

		return true;
	}

	return false;
}

bool Tutorial_CanManualAdvance() {
	if (g_TutorialEnding)
		return false;

	if (g_MessageAdvanceWaiting)
		return false;

	if (g_TutorialMessages.empty())
		return false;

	if (g_CurrentMessageIndex < 0 || g_CurrentMessageIndex >= static_cast<int>(g_TutorialMessages.size()))
		return false;

	const TutorialMessage& msg = g_TutorialMessages[g_CurrentMessageIndex];

	return msg.nextCondition == "Click";
}

void Tutorial_DrawControlGuide(){
	if (g_TutorialEnding)
		return;

	const float w = (float)Direct3D_GetBackBufferWidth();
	const float h = (float)Direct3D_GetBackBufferHeight();

	const float blink = (std::sinf(g_currentTime * 4.0f) + 1.0f) * 0.5f;

	const float alpha = 0.45f + blink * 0.55f;

	float holdRate = g_SkipHoldTimer / TUTORIAL_SKIP_HOLD_TIME;

	if (holdRate < 0.0f) holdRate = 0.0f;

	if (holdRate > 1.0f) holdRate = 1.0f;

	const bool controllerConnected = PadLogger_IsConnected();

	const wchar_t* skipText = controllerConnected
		? L"HOLD [B] : SKIP"
		: L"HOLD [J] : SKIP";

	const wchar_t* nextText = controllerConnected
		? L"[A] : NEXT"
		: L"[ENTER] : NEXT";

	Text_Begin();

	// SECTIONスキップ
	Text_Draw(
		skipText,
		w * 0.02f, h * 0.93f,
		XMVECTORF32{
			1.0f,
			1.0f,
			1.0f - holdRate,
			alpha
		},
		0.75f
	);

	// 手動で進められる時のみ表示
	if (Tutorial_CanManualAdvance()){
		Text_Draw(
			nextText,
			w * 0.02f, h * 0.90f,
			XMVECTORF32{
				1.0f,
				1.0f,
				1.0f,
				alpha
			},
			0.75f
		);
	}

	Text_End();
}

TutorialFace Tutorial_ParseFace(const std::string& face){
	if (face == "Smile")
		return TutorialFace::Smile;

	if (face == "Serious")
		return TutorialFace::Serious;

	return TutorialFace::Smile;
}

void Tutorial_ApplyMessageFace(const TutorialMessage& msg){
	TutorialFace face = Tutorial_ParseFace(msg.face);

	if (msg.speaker == "RIFF") {
		g_RiffFace = face;
	} else if (msg.speaker == "REZZ") {
		g_RezzFace = face;
	}
}

int Tutorial_GetRiffFaceTexture(TutorialFace face){
	switch (face)
	{
	case TutorialFace::Smile:
		return Resouce_Manager_GetTexId(Riff_Smile);

	case TutorialFace::Serious:
		return Resouce_Manager_GetTexId(Riff_Serious);

	default:
		return -1;
	}
}

int Tutorial_GetNaviFaceTexture(TutorialFace face){
	switch (face)
	{
	case TutorialFace::Smile:
		return Resouce_Manager_GetTexId(Rezz_Smile);

	case TutorialFace::Serious:
		return Resouce_Manager_GetTexId(Rezz_Serious);

	default:
		return -1;
	}
}

Audio_ID Tutorial_GetVoiceAudioID(const std::string& voice){
	const int voiceEntryCount = static_cast<int>(sizeof(g_TutorialVoiceTable) / sizeof(g_TutorialVoiceTable[0]));

	for (int index = 0;index < voiceEntryCount;++index){
		const TutorialVoiceEntry& entry = g_TutorialVoiceTable[index];

		if (voice == entry.voiceName){
			return entry.audioId;
		}
	}

	return Audio_ID_MAX;
}

void Tutorial_StopVoice(){
	if (g_CurrentVoiceAudioId < 0)
		return;

	StopAudio(g_CurrentVoiceAudioId);

	g_CurrentVoiceAudioId = -1;
}

void Tutorial_PlayVoice(const std::string& voice){
	// 前のメッセージのボイスを停止
	Tutorial_StopVoice();

	// ボイスなし
	if (voice.empty() || voice == "None")
		return;

	Audio_ID voiceId = Tutorial_GetVoiceAudioID(voice);

	if (voiceId == Audio_ID_MAX) // 未定義の音声へのアクセスのため
		return;

	int audioId =Resouce_Manager_GetAudioId(voiceId);

	g_CurrentVoiceAudioId = audioId;

	// ボイスはループしない
	PlayAudio(g_CurrentVoiceAudioId, false);
}

std::vector<std::string> SplitCSVLine(const std::string& line){
	std::vector<std::string> result;
	std::string field;

	bool insideQuotes = false;

	for (std::size_t i = 0; i < line.size(); ++i){
		const char c = line[i];

		if (c == '"'){
			// "" はフィールド内のダブルクォーテーション
			if (insideQuotes && i + 1 < line.size() && line[i + 1] == '"'){
				field.push_back('"');
				++i;
			} else {
				// 引用符の開始または終了
				insideQuotes = !insideQuotes;
			}
		} else if (c == ',' && !insideQuotes) {
			// 引用符の外側にあるカンマだけ列区切りにする
			result.push_back(field);
			field.clear();
		} else {
			field.push_back(c);
		}
	}

	// 最後のフィールドを追加
	result.push_back(field);

	return result;
}

std::wstring Utf8ToWstring(const std::string& str){
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
	return conv.from_bytes(str);
}

void TrimLineEnd(std::string& value){
	while (!value.empty()){
		char last = value.back();

		if (last == '\r' || last == '\n' || last == ' ' || last == '\t'){
			value.pop_back();
		} else {
			break;
		}
	}
}

TutorialSection Tutorial_ParseSection(const std::string& section){
	if (section == "Intro")
		return TutorialSection::Intro;

	if (section == "UI")
		return TutorialSection::UI;

	if (section == "Notes")
		return TutorialSection::Notes;

	if (section == "Fever")
		return TutorialSection::Fever;

	if (section == "FinalPractice")
		return TutorialSection::FinalPractice;

	if (section == "Ending")
		return TutorialSection::Ending;

	return TutorialSection::None;
}

void Tutorial_LoadCSV()
{
	g_TutorialMessages.clear();

	const char* path = Resouce_Manager_GetDataFilePath(DataFileID::Tutorial_Text);

	if (path == nullptr) return;

	std::ifstream file(path, std::ios::binary);

	if (!file) return;

	std::string line;
	int lineNo = 0;

	std::string currentLang = Configu_GetLanguageCode();

	while (std::getline(file, line))
	{
		lineNo++;

		// 1行目: ヘッダー
		// 2行目: 日本語説明
		if (lineNo <= 2)
		{
			continue;
		}

		if (line.empty())
		{
			continue;
		}

		std::vector<std::string> cols = SplitCSVLine(line);

		for (auto& col : cols){
			TrimLineEnd(col);
		}

		// ID,SECTION,STEP,SPEAKER,FACE,VOICE,LANG,TEXT,NEXT_CONDITION,COMMAND = 10
		if (cols.size() < TUTORIAL_CSV_COLUMN_COUNT){
			continue;
		}

		std::string lang = cols[6]; // LANG

		if (lang != currentLang)
		{
			continue;
		}

		TutorialMessage msg;

		msg.id = std::stoi(cols[0]);
		msg.section = Tutorial_ParseSection(cols[1]);
		msg.step = cols[2];
		msg.speaker = cols[3];
		msg.face = cols[4];
		msg.voice = cols[5];
		msg.lang = cols[6];
		msg.text = Utf8ToWstring(cols[7]);
		msg.nextCondition = cols[8];
		msg.command = cols[9];

		g_TutorialMessages.push_back(msg);
	}
}