/*
	リソースファイルの管理：resouce_manager.h

	2026/01/23	hibiki sakuma
*/

#include "resource_manager.h"
#include "texture.h"
#include "Audio.h"

#include <fstream>
#include <sstream>
#include <direct.h>

// それぞれの素材情報を格納
static int g_Resouce_Tex[Tex_ID_MAX]{};
static int g_Resouce_Audio[Audio_ID_MAX]{};
static ScoreData g_Resouce_ScoreData[SCORE_MAX]{};
static const char* g_Resouce_DataFilePath[static_cast<int>(DataFileID::Max)]{};
static const wchar_t* g_Resouce_FontFilePath[static_cast<int>(FontID::Max)]{};

// ファイル読み込み用関数
static void LoadScoreData(ScoreID id, const char* fliename);

void Resouce_ManagerInitialize(){
	// UI素材
	g_Resouce_Tex[TextBox] = Texture_Load(L"resource/sprite/UI/textbox.png");

	// カラー素材
	g_Resouce_Tex[Color_White] = Texture_Load(L"resource/sprite/color/w.png");
	g_Resouce_Tex[Color_Black] = Texture_Load(L"resource/sprite/color/black.png");

	// 画像の読み込み
	// タイトル画面
	g_Resouce_Tex[Title_Back]  = Texture_Load(L"resource/sprite/background/title.png");
	g_Resouce_Tex[Title_Logo]  = Texture_Load(L"resource/sprite/UI/Title/Title_Logo.png");
	g_Resouce_Tex[Title_Start] = Texture_Load(L"resource/sprite/UI/Title/Start_Button.png");

	// 選曲画面
	g_Resouce_Tex[Select_Back] = Texture_Load(L"resource/sprite/background/select.png");
	g_Resouce_Tex[Configu_Icon]   = Texture_Load(L"resource/sprite/UI/music_select_icon/configu.png");
	g_Resouce_Tex[Edit_Icon]      = Texture_Load(L"resource/sprite/UI/music_select_icon/edit.png");
	g_Resouce_Tex[Song001_jacket] = Texture_Load(L"resource/sprite/UI/music_select_icon/song001_jacket.png");
	g_Resouce_Tex[Song002_jacket] = Texture_Load(L"resource/sprite/UI/music_select_icon/song002_jacket.png");
	g_Resouce_Tex[Song003_jacket] = Texture_Load(L"resource/sprite/UI/music_select_icon/song003_jacket.png");

	// リザルト画面
	g_Resouce_Tex[Result_Back] = Texture_Load(L"resource/sprite/background/result.png");
	g_Resouce_Tex[Rezz_Fail]       = Texture_Load(L"resource/sprite/Rezz/Rezz_Fail.png");
	g_Resouce_Tex[Rezz_Clear]      = Texture_Load(L"resource/sprite/Rezz/Rezz_Clear.png");
	g_Resouce_Tex[Rezz_Fullcombo]  = Texture_Load(L"resource/sprite/Rezz/Rezz_Fullcombo.png");
	g_Resouce_Tex[Rezz_AllPerfect] = Texture_Load(L"resource/sprite/Rezz/Rezz_Fullcombo.png");

	// ナビキャラクター立ち絵
	g_Resouce_Tex[Rezz_Smile]   = Texture_Load(L"resource/sprite/Rezz/Rezz_Smile.png");
	g_Resouce_Tex[Rezz_Serious] = Texture_Load(L"resource/sprite/Rezz/Rezz_Serious.png");

	// プレイヤー
	// リフ
	g_Resouce_Tex[Riff_Smile]            = Texture_Load(L"resource/sprite/player/Riff/Riff_Smile.png");
	g_Resouce_Tex[Riff_Serious]          = Texture_Load(L"resource/sprite/player/Riff/Riff_Serious.png");
	g_Resouce_Tex[SD_Riff_main]	         = Texture_Load(L"resource/sprite/player_SD/Riff/RiffSD_main.png");
	g_Resouce_Tex[SD_Riff_main_FEVER]    = Texture_Load(L"resource/sprite/player_SD/Riff/RiffSD_main_FEVER.png");
	g_Resouce_Tex[SD_Riff_miss]		     = Texture_Load(L"resource/sprite/player_SD/Riff/RiffSD_miss.png");
	g_Resouce_Tex[SD_Riff_miss_FEVER]    = Texture_Load(L"resource/sprite/player_SD/Riff/RiffSD_miss_FEVER.png");
	g_Resouce_Tex[SD_Riff_success]       = Texture_Load(L"resource/sprite/player_SD/Riff/RiffSD_success.png");
	g_Resouce_Tex[SD_Riff_success_FEVER] = Texture_Load(L"resource/sprite/player_SD/Riff/RiffSD_success_FEVER.png");
	// ルナ
	g_Resouce_Tex[Luna_Smile]            = Texture_Load(L"resource/sprite/player/Luna/Luna_Smile.png");
	g_Resouce_Tex[SD_Luna_main]          = Texture_Load(L"resource/sprite/player_SD/Luna/LunaSD_main.png");
	g_Resouce_Tex[SD_Luna_main_FEVER]    = Texture_Load(L"resource/sprite/player_SD/Luna/LunaSD_main_FEVER.png");
	g_Resouce_Tex[SD_Luna_miss]          = Texture_Load(L"resource/sprite/player_SD/Luna/LunaSD_miss.png");
	g_Resouce_Tex[SD_Luna_miss_FEVER]    = Texture_Load(L"resource/sprite/player_SD/Luna/LunaSD_miss_FEVER.png");
	g_Resouce_Tex[SD_Luna_success]       = Texture_Load(L"resource/sprite/player_SD/Luna/LunaSD_success.png");
	g_Resouce_Tex[SD_Luna_success_FEVER] = Texture_Load(L"resource/sprite/player_SD/Luna/LunaSD_success_FEVER.png");
	// メロディ
	g_Resouce_Tex[Melody_Smile]            = Texture_Load(L"resource/sprite/player/Melody/Melody_Smile.png");
	g_Resouce_Tex[SD_Melody_main]          = Texture_Load(L"resource/sprite/player_SD/Melody/MelodySD_main.png");
	g_Resouce_Tex[SD_Melody_main_FEVER]    = Texture_Load(L"resource/sprite/player_SD/Melody/MelodySD_main_FEVER.png");
	g_Resouce_Tex[SD_Melody_miss]          = Texture_Load(L"resource/sprite/player_SD/Melody/MelodySD_miss.png");
	g_Resouce_Tex[SD_Melody_miss_FEVER]    = Texture_Load(L"resource/sprite/player_SD/Melody/MelodySD_miss_FEVER.png");
	g_Resouce_Tex[SD_Melody_success]       = Texture_Load(L"resource/sprite/player_SD/Melody/MelodySD_success.png");
	g_Resouce_Tex[SD_Melody_success_FEVER] = Texture_Load(L"resource/sprite/player_SD/Melody/MelodySD_success_FEVER.png");


	// 敵
	g_Resouce_Tex[Enemy_Tutorial] = Texture_Load(L"resource/sprite/enemy/tutorial.png");
	g_Resouce_Tex[Enemy_Song001]  = Texture_Load(L"resource/sprite/enemy/Enemy_Song001.png");
	g_Resouce_Tex[Enemy_Song002]  = Texture_Load(L"resource/sprite/enemy/Enemy_Song002.png");
	g_Resouce_Tex[Enemy_Song003]  = Texture_Load(L"resource/sprite/enemy/Enemy_Song003.png");

	// 各種ゲージ
	g_Resouce_Tex[Player_HP_Case]     = Texture_Load(L"resource/sprite/UI/GameUI/Player_HP_Case.png");
	g_Resouce_Tex[Player_FEVER_Case]  = Texture_Load(L"resource/sprite/UI/GameUI/Player_FEVER_Case.png");
	g_Resouce_Tex[Player_HP_Point]    = Texture_Load(L"resource/sprite/UI/GameUI/Player_HP_Point.png");
	g_Resouce_Tex[Player_FEVER_Point] = Texture_Load(L"resource/sprite/UI/GameUI/Player_FEVER_Point.png");
	g_Resouce_Tex[Enemy_HP_Case]      = Texture_Load(L"resource/sprite/UI/GameUI/Enemy_HP_Case.png");
	g_Resouce_Tex[Enemy_HP_Point]     = Texture_Load(L"resource/sprite/UI/GameUI/Enemy_HP_Point.png");

	// ノーツ
	g_Resouce_Tex[Lane]            = Texture_Load(L"resource/sprite/UI/Notes/Lane.png");
	g_Resouce_Tex[Judge_W]         = Texture_Load(L"resource/sprite/UI/Notes/Judge_W.png");
	g_Resouce_Tex[Judge_S]         = Texture_Load(L"resource/sprite/UI/Notes/Judge_S.png");
	g_Resouce_Tex[Judge_Y]         = Texture_Load(L"resource/sprite/UI/Notes/Judge_Y.png");
	g_Resouce_Tex[Judge_X]         = Texture_Load(L"resource/sprite/UI/Notes/Judge_X.png");
	g_Resouce_Tex[Judge_Triangle]  = Texture_Load(L"resource/sprite/UI/Notes/Judge_Triangle.png");
	g_Resouce_Tex[Judge_Square]    = Texture_Load(L"resource/sprite/UI/Notes/Judge_Square.png");	
	g_Resouce_Tex[Notes_Normal]    = Texture_Load(L"resource/sprite/UI/Notes/notes_nolmal.png");
	g_Resouce_Tex[Notes_tail]      = Texture_Load(L"resource/sprite/UI/Notes/notes_tail.png");
	g_Resouce_Tex[Notes_tail_head] = Texture_Load(L"resource/sprite/UI/Notes/notes_tail_head.png");
	g_Resouce_Tex[Notes_tail_body] = Texture_Load(L"resource/sprite/UI/Notes/notes_tail_body.png");
	g_Resouce_Tex[Notes_tail_tail] = Texture_Load(L"resource/sprite/UI/Notes/notes_tail_tail.png");

	// コンボエフェクト
	g_Resouce_Tex[Combo_Perfect] = Texture_Load(L"resource/sprite/UI/Combo/Combo_Perfect.png");
	g_Resouce_Tex[Combo_Great]   = Texture_Load(L"resource/sprite/UI/Combo/Combo_Great.png");
	g_Resouce_Tex[Combo_Miss]    = Texture_Load(L"resource/sprite/UI/Combo/Combo_Miss.png");
	g_Resouce_Tex[Combo_Text]    = Texture_Load(L"resource/sprite/UI/Combo/Combo_Text.png");
	g_Resouce_Tex[Combo_Num]     = Texture_Load(L"resource/sprite/UI/Combo/Combo_num.png");

	// ランク用画像
	g_Resouce_Tex[Rank_IMG_D] = Texture_Load(L"resource/sprite/UI/Rank/rank_D.png");
	g_Resouce_Tex[Rank_IMG_C] = Texture_Load(L"resource/sprite/UI/Rank/rank_C.png");
	g_Resouce_Tex[Rank_IMG_B] = Texture_Load(L"resource/sprite/UI/Rank/rank_B.png");
	g_Resouce_Tex[Rank_IMG_A] = Texture_Load(L"resource/sprite/UI/Rank/rank_A.png");
	g_Resouce_Tex[Rank_IMG_S] = Texture_Load(L"resource/sprite/UI/Rank/rank_S.png");

	// アニメーション用画像
	g_Resouce_Tex[Anim_P1] = Texture_Load(L"resource/sprite/UI/Title/Anim_p1.png");
	g_Resouce_Tex[Anim_P2] = Texture_Load(L"resource/sprite/UI/Title/Anim_p2.png");
	g_Resouce_Tex[Anim_P3] = Texture_Load(L"resource/sprite/UI/Title/Anim_p3.png");

	// 背景画像
	g_Resouce_Tex[BG_ground] = Texture_Load(L"resource/sprite/background/ground_1.png");
	g_Resouce_Tex[BG_building] = Texture_Load(L"resource/sprite/background/building_1.png");
	g_Resouce_Tex[BG_sky]      = Texture_Load(L"resource/sprite/background/sky_1.png");


	// 音源の読み込み
	// BGM
	g_Resouce_Audio[Title_BGM]     = LoadAudio("resource/audio/BGM/title_bgm_1.wav", AUDIO_BGM);
	g_Resouce_Audio[Select_BGM]    = LoadAudio("resource/audio/BGM/select_bgm_1.wav", AUDIO_BGM);
	g_Resouce_Audio[Tutorial_BGM]  = LoadAudio("resource/audio/BGM/tutorial_bgm_1.wav", AUDIO_BGM);
	g_Resouce_Audio[Result_BGM]    = LoadAudio("resource/audio/BGM/result_bgm_1.wav", AUDIO_BGM);
	// Game_BGM
	g_Resouce_Audio[Game_BGM]         = LoadAudio("resource/audio/BGM/song/test_bgm.wav", AUDIO_BGM);
	g_Resouce_Audio[Game_Song001_BGM] = LoadAudio("resource/audio/BGM/song/Drumshadow_Motion_001.wav", AUDIO_BGM);
	g_Resouce_Audio[Game_Song002_BGM] = LoadAudio("resource/audio/BGM/song/Bass_Blitz_002.wav", AUDIO_BGM);
	g_Resouce_Audio[Game_Song003_BGM] = LoadAudio("resource/audio/BGM/song/Rhythm_Riff_Rampage_003.wav", AUDIO_BGM);
	g_Resouce_Audio[Edit_Game_BGM]    = LoadAudio("resource/audio/BGM/song/Echoes_of_Steady_Pulse_Edit.wav", AUDIO_BGM);
	// SE
	g_Resouce_Audio[Title_AnyPress_SE] = LoadAudio("resource/audio/SE/title_anypress_se.wav", AUDIO_SE);
	g_Resouce_Audio[MusicSelect_Decide_SE] = LoadAudio("resource/audio/SE/musicselect_decide_se.wav", AUDIO_SE);

	g_Resouce_Audio[CursorMove_SE] = LoadAudio("resource/audio/SE/cursormove_se.wav", AUDIO_SE);
	g_Resouce_Audio[Cancel_SE] = LoadAudio("resource/audio/SE/cancel_se.wav", AUDIO_SE);

	g_Resouce_Audio[Note_Success_SE] = LoadAudio("resource/audio/SE/success_se.wav", AUDIO_SE);

	g_Resouce_Audio[Check_SE] = LoadAudio("resource/audio/SE/SE_check.wav", AUDIO_SE);


	// CV
	g_Resouce_Audio[TestCV] = LoadAudio("resource/audio/CV/CV_test.wav", AUDIO_VOICE);

	g_Resouce_Audio[Rezz_Tutorial_001] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_001.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_002] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_002.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_003] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_003.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_004] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_004.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_005] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_005.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_006] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_006.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_007] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_007.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_008] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_008.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_009] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_009.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_010] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_010.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_011] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_011.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_012] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_012.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_013] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_013.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_014] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_014.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_015] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_015.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_016] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_016.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_017] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_017.wav", AUDIO_VOICE);
	g_Resouce_Audio[Rezz_Tutorial_018] = LoadAudio("resource/audio/CV/Rezz_Tutorial/rezz_tutorial_018.wav", AUDIO_VOICE);

	g_Resouce_Audio[Riff_Tutorial_001] = LoadAudio("resource/audio/CV/Riff_Tutorial/riff_tutorial_001.wav", AUDIO_VOICE);
	g_Resouce_Audio[Riff_Tutorial_002] = LoadAudio("resource/audio/CV/Riff_Tutorial/riff_tutorial_002.wav", AUDIO_VOICE);
	g_Resouce_Audio[Riff_Tutorial_003] = LoadAudio("resource/audio/CV/Riff_Tutorial/riff_tutorial_003.wav", AUDIO_VOICE);


	// CSVファイルの読み込み
	// テキストデータのファイルパス
	//チュートリアル画面
	g_Resouce_DataFilePath[static_cast<int>(DataFileID::Tutorial_Text)] = "resource/data/text_data/tutorial_text.csv";	
	// 選曲画面
	g_Resouce_DataFilePath[static_cast<int>(DataFileID::Music_Select_song_master)]     = "resource/data/text_data/music_select/song_master.csv";
	g_Resouce_DataFilePath[static_cast<int>(DataFileID::Music_Select_song_difficulty)] = "resource/data/text_data/music_select/song_difficulty.csv";
	g_Resouce_DataFilePath[static_cast<int>(DataFileID::Music_Select_song_text)]       = "resource/data/text_data/music_select/song_text.csv";
	// UIテキスト
	g_Resouce_DataFilePath[static_cast<int>(DataFileID::UI_Text)] = "resource/data/text_data/ui_text.csv";
	// キャラクター選択画面
	g_Resouce_DataFilePath[static_cast<int>(DataFileID::Character_Select_text)] = "resource/data/text_data/character_text.csv";

	// 楽譜データ
	LoadScoreData(SCORE_SONG001_EASY,   "resource/data/score_data/Drumshadow_Motion_001_Easy.csv");
	LoadScoreData(SCORE_SONG001_NORMAL, "resource/data/score_data/Drumshadow_Motion_001_Normal.csv");
	LoadScoreData(SCORE_SONG001_HARD,   "resource/data/score_data/Drumshadow_Motion_001_Hard.csv");
	LoadScoreData(SCORE_SONG002_EASY,   "resource/data/score_data/Bass_Blitz_002_Easy.csv");
	LoadScoreData(SCORE_SONG002_NORMAL, "resource/data/score_data/Bass_Blitz_002_Normal.csv");
	LoadScoreData(SCORE_SONG002_HARD,   "resource/data/score_data/Bass_Blitz_002_Hard.csv");
	LoadScoreData(SCORE_SONG003_EASY,   "resource/data/score_data/Rhythm_Riff_Rampage_003_Easy.csv");
	LoadScoreData(SCORE_SONG003_NORMAL, "resource/data/score_data/Rhythm_Riff_Rampage_003_Normal.csv");
	LoadScoreData(SCORE_SONG003_HARD,   "resource/data/score_data/Rhythm_Riff_Rampage_003_Hard.csv");

	LoadScoreData(SCORE_EDIT, "resource/data/score_data/score_edit.csv");

	// フォントデータの読み込み（ファイルパス）
	g_Resouce_FontFilePath[static_cast<int>(FontID::Japanese_Font)] = L"resource/data/font/font_jp.spritefont";
	g_Resouce_FontFilePath[static_cast<int>(FontID::English_Font)]  = L"resource/data/font/font_en.spritefont";
	g_Resouce_FontFilePath[static_cast<int>(FontID::Korean_Font)]   = L"resource/data/font/font_kr.spritefont";
	g_Resouce_FontFilePath[static_cast<int>(FontID::Chinese_Font)]  = L"resource/data/font/font_zh_cn.spritefont";

}

void Resouce_ManagerFinalize(){
	Texture_AllRelease();

	for (int i = 0; i < Audio_ID_MAX; i++) {
		UnloadAudio(g_Resouce_Audio[i]);
	}
}

int Resouce_Manager_GetTexId(Tex_ID id){
	return g_Resouce_Tex[id];
}

int Resouce_Manager_GetAudioId(Audio_ID id){
	return g_Resouce_Audio[id];
}

const char* Resouce_Manager_GetDataFilePath(DataFileID id){
	int index = static_cast<int>(id);

	if (index < 0 || index >= static_cast<int>(DataFileID::Max)) // エラーガード
		return nullptr;

	return g_Resouce_DataFilePath[index];
}

const wchar_t* Resouce_Manager_GetFontFilePath(FontID id)
{
	int index = static_cast<int>(id);

	if (index < 0 || index >= static_cast<int>(FontID::Max)) // エラーガード
		return nullptr;

	return g_Resouce_FontFilePath[index];
}

const ScoreData& Resouce_Manager_GetScoreData(ScoreID id){
	return g_Resouce_ScoreData[id];
}

void Resouce_Manager_Reload_EditScoreData(){
	LoadScoreData(SCORE_EDIT, "resource/data/score_data/score_edit.csv");
}

// 内部関数
static void LoadScoreData(ScoreID id, const char* filename){
	char currentDirectory[256]{};

	_getcwd(currentDirectory, sizeof(currentDirectory));

	printf("CURRENT DIR : %s\n", currentDirectory);

	printf("LOAD SCORE  : %s\n", filename);

	std::ifstream file(filename);

	if (!file.is_open()){
		printf("OPEN FAILED : %s\n", filename);

		std::string message = "Failed to open score file:\n";

		message += filename;

		MessageBoxA(
			nullptr,
			message.c_str(),
			"Score Load Error",
			MB_OK | MB_ICONERROR
		);

		return;
	}

	printf("OPEN SUCCESS: %s\n", filename);

	g_Resouce_ScoreData[id].notes.clear();

	std::string line;

	// 1行目スキップ
	std::getline(file, line);

	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		std::stringstream ss(line);

		std::string timeStr, laneStr, typeStr, paramStr;

		std::getline(ss, timeStr, ',');
		std::getline(ss, laneStr, ',');
		std::getline(ss, typeStr, ',');
		std::getline(ss, paramStr, ',');

		ScoreNote n{};
		n.time = std::stof(timeStr);
		n.lane = std::stoi(laneStr);
		n.type = std::stoi(typeStr);
		n.param = std::stof(paramStr);

		g_Resouce_ScoreData[id].notes.push_back(n);
	}

}
