/*
*
*	ゲーム本体：game.cpp
*
*	2026/04/27		hibiki sakuma
*
*/

#include "Game.h"
#include "bg.h"
#include "ui.h"
#include "combo.h"
#include "Player.h"
#include "direct3d.h"
#include "note.h"
#include "enemy.h"
#include "Audio.h"
#include "fade.h"
#include "scene.h"
#include "score_manager.h"
#include "Result.h"
#include "Music_select.h"
#include "Configu.h"
#include "Character_select.h" 
#include "texture.h"
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;
#include "Key_logger.h" //キー入力
#include "pad_logger.h" // パッド入力

// 定数宣言
static constexpr float NOTE_SPEED_DEFAULT = 500.0f;
static constexpr float OFFSET_END_TIME = 2.0f;
static constexpr float BASE_DAMAGE = 10.0f;
// スコア
static constexpr int GAMESCORE_PERFECT = 100;
static constexpr int GAMESCORE_GREAT = 50;
static constexpr int GAMESCORE_KILL = 1000;
// フェード時間
static constexpr float FADE_IN_TIME = 1.0f;
static constexpr float FADE_OUT_TIME = 0.5f;
// フェード終了後のゲーム開始待機時間
static constexpr float GAME_START_DELAY = 1.0f;
// 未定義ID
static constexpr int INVALID_AUDIO_ID = -1;

// 変数宣言
static float g_currentTime = 0.0f;
static float g_GameStartDelayTimer = 0.0f;
static bool g_GameStart = false;
static bool g_GameFinish = false;
static bool g_HasBgmStarted = false;
static bool g_IsResultTransitionStarted = false;
static float g_JudgeX = -1.0f; // 判定ライン位置 note.cppでも使用
static float g_LastNoteTime = 0.0f;
static int g_ComboCount_Miss = -1;
static int g_ComboCount_Great = -1;
static int g_ComboCount_Perfect = -1;
static int g_ComboCount_Now = -1;
static int g_ComboCount_Max = -1;
static int g_KillCount = 0;
static int g_GameScore = 0;
static bool g_IsFullCombo = false;
static bool g_IsAllPerfect = false;
static bool g_PrevFeverState = false;

static Player* g_pPlayer = nullptr; // プレーヤーの情報
static Enemy* g_pEnemy = nullptr; // エネミーの情報
static std::vector<Note> g_notes; // ノーツ情報
static ScoreData g_scoreData; // 譜面情報
static ResultType result = ResultType::FAIL;
static Rank_Type g_GameRank = Rank_D;
static int g_GameBgmAudioId =INVALID_AUDIO_ID;

void Game_Initialize() {	
	Bg_Initialize();
	UI_Initialize();
	g_pPlayer = new Player();
	g_pPlayer->Initialize(Character_Select_GetCharacter());
	g_pEnemy = new Enemy();
	g_pEnemy->Initialize(Music_Select_GetEnemyId());
	
	g_GameBgmAudioId = Music_Select_GetBgmAudioId();

	// 譜面読み込み
	g_scoreData = Resouce_Manager_GetScoreData((ScoreID)Music_Select_GetId()); // 選曲画面から番号を呼び出し

	// 譜面からNote生成
	g_notes.clear(); // 前の情報をリセット
	for (auto& s : g_scoreData.notes) {
		Note n;
		n.Initialize(s.time, s.lane, s.type, s.param);
		g_notes.push_back(n);
	}

	// 譜面の終了時間の取得
	if (!g_scoreData.notes.empty()){ // 空データ対策　＊念のため
		g_LastNoteTime = g_scoreData.notes.back().time;
	}else{
		g_LastNoteTime = 0.0f;
	}

	// 変数の初期化
	g_currentTime = 0.0f;
	g_GameStartDelayTimer = GAME_START_DELAY;
	result = ResultType::FAIL;
	g_GameFinish = false;
	g_GameStart = false;
	g_HasBgmStarted = false;
	g_IsResultTransitionStarted = false;
	g_JudgeX = Direct3D_GetBackBufferWidth() * 0.25f;

	g_ComboCount_Miss = 0;
	g_ComboCount_Great = 0;
	g_ComboCount_Perfect = 0;
	g_ComboCount_Now = 0;
	g_ComboCount_Max = 0;
	g_IsFullCombo = false;
	g_IsAllPerfect = false;

	g_KillCount = 0;
	g_GameScore = 0;

	g_GameRank = Rank_D;
	g_PrevFeverState = false;

	//画面遷移　フェードイン処理
	Fade_Start(FADE_IN_TIME, false); // タイトルからつながっている
}

void Game_Finalize(){
	if(g_GameBgmAudioId != INVALID_AUDIO_ID)
		StopAudio(Resouce_Manager_GetAudioId((Audio_ID)g_GameBgmAudioId));

	delete g_pEnemy;
	delete g_pPlayer;
	UI_Finalize();
	Bg_Finalize();
}

void Game_UpDate(double elapsed_time){
	const float elapsedTime = static_cast<float>(elapsed_time);

	// フェード終了後、一定時間待って開始
	if (!g_GameStart){
		if (Fade_GetState() == FADE_STATE_FINISHED_IN){
			g_GameStartDelayTimer -= elapsedTime;

			if (g_GameStartDelayTimer <= 0.0f){
				g_GameStartDelayTimer = 0.0f;

				// ノーツ時刻と楽曲を同時に開始
				g_currentTime = 0.0f;

				PlayAudio(g_GameBgmAudioId, false);
				g_HasBgmStarted = true;
				g_GameStart = true;
			}
		}
	}else{
		// XAudio2の再生位置をゲーム時刻にする
		if (IsPlaying(g_GameBgmAudioId)){
			g_currentTime = GetAudioPlaybackTime(g_GameBgmAudioId);
		}
	}

	if (g_pEnemy->IsDead()) { // 敵撃破時に再生成
		g_KillCount++;
		g_GameScore += GAMESCORE_KILL;

		g_pEnemy->Initialize(Music_Select_GetEnemyId());
	}

	// 入力管理
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

	bool triggerLane[2] = {
		triggerUp,
		triggerDown
	};

	bool pressedLane[2] = {
		pressedUp,
		pressedDown
	};

	if (g_GameStart && !g_GameFinish) {
		for (auto& n : g_notes) {
			NoteJudgeResult judge = n.UpdateJudge(g_currentTime, triggerLane, pressedLane);

			// Lunaのスキル中は判定を強化
			if (judge == NoteJudgeResult::Note_Great && g_pPlayer->IsGreatToPerfectActive()) {
				judge = NoteJudgeResult::Note_Perfect;
			}

			switch (judge) {
			case NoteJudgeResult::Note_Perfect:
				UI_SetJudge(Judge_Perfect);

				g_ComboCount_Perfect++;
				g_ComboCount_Now++;
				// スキルでスコアをアップする
				g_GameScore += g_pPlayer->GetSkillScore(GAMESCORE_PERFECT);

				g_pPlayer->OnNoteSuccess(true);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(BASE_DAMAGE));

				Combo_Add();
				break;

			case NoteJudgeResult::Note_Great:
				UI_SetJudge(Judge_Great);

				g_ComboCount_Great++;
				g_ComboCount_Now++;
				g_GameScore += g_pPlayer->GetSkillScore(GAMESCORE_GREAT);

				g_pPlayer->OnNoteSuccess(false);
				Combo_SetPerfectChain(false);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(BASE_DAMAGE * 0.5f));

				Combo_Add();
				break;

			case NoteJudgeResult::Note_LongTick:
				UI_SetJudge(Judge_Perfect);

				g_ComboCount_Perfect++;
				g_ComboCount_Now++;
				g_GameScore += g_pPlayer->GetSkillScore(GAMESCORE_PERFECT);

				g_pPlayer->OnNoteSuccess(true);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(BASE_DAMAGE * 0.5f));

				Combo_Add();
				break;

			case NoteJudgeResult::Note_RapidTick:
				UI_SetJudge(Judge_Perfect);

				g_ComboCount_Perfect++;
				g_ComboCount_Now++;
				g_GameScore += g_pPlayer->GetSkillScore(GAMESCORE_PERFECT);

				g_pPlayer->OnNoteSuccess(true);

				g_pEnemy->Damage(g_pPlayer->GetAttackDamage(BASE_DAMAGE * 0.5f));

				Combo_Add();
				break;

			case NoteJudgeResult::Note_Miss:
				UI_SetJudge(Judge_Miss);

				g_ComboCount_Miss++;

				if (g_ComboCount_Now > g_ComboCount_Max)
					g_ComboCount_Max = g_ComboCount_Now;

				g_ComboCount_Now = 0;

				g_pPlayer->OnNoteMiss();
				Combo_Reset();
				Combo_SetPerfectChain(false);
				break;
			}
		}
	}

	Bg_Update(elapsed_time);
	g_pPlayer->Update(elapsed_time);
	bool currentFever = g_pPlayer->IsFever();

	if (!g_PrevFeverState && currentFever){
		UI_ShowSkillPopup(g_pPlayer->GetSkillName());
	}
	g_PrevFeverState = currentFever;

	g_pEnemy->Update(elapsed_time);

	// Note更新
	if (g_GameStart && !g_GameFinish) {
		for (auto& n : g_notes) {
			n.SetNoteSpeed(NOTE_SPEED_DEFAULT * Configu_GetNoteSpeed());

			n.Update(g_currentTime, g_JudgeX);
		}
	}

	// UI情報のセット
	UI_SetPlayerHpRate(g_pPlayer->GetHp() / g_pPlayer->GetMaxHP());
	UI_SetEnemyHpRate(g_pEnemy->GetHp() / g_pEnemy->GetMaxHp());
	UI_SetPlayerFeverRate(g_pPlayer->GetFeverGauge() / g_pPlayer->GetFeverGaugeMAX());
	
	UI_SetKillCount(g_KillCount);
	UI_SetScore(g_GameScore);
	// ランクの計算と設定
	g_GameRank = Score_CalculateRank(g_GameScore, Music_Select_GetMaxScore());
	UI_SetRank(g_GameRank);
	// セット後に更新
	UI_Update(elapsed_time);

	// ゲーム終了処理
	const bool hasPassedChartEnd = g_currentTime > g_LastNoteTime + OFFSET_END_TIME;
	const bool hasAudioFinished = g_HasBgmStarted && !IsPlaying(g_GameBgmAudioId);

	// ゲームオーバー時の処理
	if (g_pPlayer->GetHp() <= 0.0f){
		result = ResultType::FAIL;

		g_IsAllPerfect = false;
		g_IsFullCombo = false;

		g_GameFinish = true;
	} // ゲームクリア時の処理
	else if (hasPassedChartEnd || hasAudioFinished){
		// 最後まで継続していたコンボを反映
		if (g_ComboCount_Now > g_ComboCount_Max){
			g_ComboCount_Max = g_ComboCount_Now;
		}

		const int successJudgeCount = g_ComboCount_Perfect + g_ComboCount_Great;
		const bool hasJudgedNote = successJudgeCount > 0 || g_ComboCount_Miss > 0;

		g_IsFullCombo = hasJudgedNote && g_ComboCount_Miss == 0;
		g_IsAllPerfect = hasJudgedNote && g_ComboCount_Miss == 0 && g_ComboCount_Great == 0 && g_ComboCount_Perfect > 0;

		result = g_KillCount > 0
			? ResultType::CLEAR
			: ResultType::FAIL;

		g_GameFinish = true;
	}

	if (g_GameFinish && !g_IsResultTransitionStarted) {
		Fade_Start(FADE_OUT_TIME, true, { 0.0f,0.0f,0.0f });
		g_IsResultTransitionStarted = true;
	}

	if (Fade_GetState() == FADE_STATE_FINISHED_OUT){
		GameResultData resultData;

		resultData.resultType = result;
		resultData.songId = Music_Select_GetSongId();
		resultData.difficulty = Music_Select_GetDifficulty();
		resultData.score = g_GameScore;
		resultData.rank = g_GameRank;
		resultData.maxCombo = g_ComboCount_Max;
		resultData.perfectCount = g_ComboCount_Perfect;
		resultData.greatCount = g_ComboCount_Great;
		resultData.missCount = g_ComboCount_Miss;
		resultData.isAllPerfect = g_IsAllPerfect;
		resultData.isFullCombo = g_IsFullCombo;
		resultData.character = g_pPlayer->GetCharacter();

		Result_SetData(resultData);

		Scene_Change(SCENE_RESULT);
	}

	if (g_GameStart && Fade_GetState() == FADE_STATE_FINISHED_OUT) {
		g_GameStart = false;
	}

}

void Game_Draw(){
	Bg_Draw();
	UI_Draw();
	// 判定位置の描画
	float w = (float)Direct3D_GetBackBufferWidth();
	float h = (float)Direct3D_GetBackBufferHeight();

	float judgeX = w * 0.25f;

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
	Sprite_Draw(
		Judge_Up_TexId,
		judgeX - (Judge_TexSize * 0.5f), h * 0.5f - h * 0.1f - (Judge_TexSize * 0.5f),
		Judge_TexSize, Judge_TexSize,
		{ 1.0f,1.0f,1.0f,0.5f }
	);
	Sprite_Draw(
		Judge_Down_TexId,
		judgeX - (Judge_TexSize * 0.5f), h * 0.5f + h * 0.1f - (Judge_TexSize * 0.5f),
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

}
