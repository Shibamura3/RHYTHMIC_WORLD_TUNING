/*
	リソースファイルの管理：resouce_manager.h

	2026/01/23	hibiki sakuma
*/
#ifndef RESOUCE_MANAGER_H
#define RESOUCE_MANAGER_H

#include "note.h"
#include <vector>

enum Tex_ID
{
	// UI素材
	TextBox,

	// カラー素材
	Color_White,
	Color_Black,

	// タイトル画面関連
	Title_Back,
	Title_Logo,
	Title_Start,

	// 選曲画面
	Select_Back,
	Configu_Icon,
	Edit_Icon,
	Song001_jacket,
	Song002_jacket,
	Song003_jacket,

	// リザルト画面
	Result_Back,

	// ナビキャラ
	Rezz_Smile,
	Rezz_Serious,
	Rezz_Fail,
	Rezz_Clear,
	Rezz_Fullcombo,
	Rezz_AllPerfect,

	// プレイヤー
	// リフ
	Riff_Smile,
	Riff_Serious,
	SD_Riff_main,
	SD_Riff_main_FEVER,
	SD_Riff_miss,
	SD_Riff_miss_FEVER,
	SD_Riff_success,
	SD_Riff_success_FEVER,
	// ルナ
	Luna_Smile,
	SD_Luna_main,
	SD_Luna_main_FEVER,
	SD_Luna_miss,
	SD_Luna_miss_FEVER,
	SD_Luna_success,
	SD_Luna_success_FEVER,
	// メロディ
	Melody_Smile,
	SD_Melody_main,
	SD_Melody_main_FEVER,
	SD_Melody_miss,
	SD_Melody_miss_FEVER,
	SD_Melody_success,
	SD_Melody_success_FEVER,

	// 敵
	Enemy_Tutorial,
	Enemy_Song001,
	Enemy_Song002,
	Enemy_Song003,

	// 各種ゲージ
	Player_HP_Case,
	Player_FEVER_Case,
	Player_HP_Point,
	Player_FEVER_Point,
	Enemy_HP_Case,
	Enemy_HP_Point,

	// ノーツ
	Lane,
	Judge_S,
	Judge_W,
	Judge_X,
	Judge_Y,
	Judge_Square,
	Judge_Triangle,
	Notes_Normal,
	Notes_tail,
	Notes_tail_head,
	Notes_tail_body,
	Notes_tail_tail,
	Anim_P1,
	Anim_P2,
	Anim_P3,

	// コンボエフェクト
	Combo_Perfect,
	Combo_Great,
	Combo_Miss,
	Combo_Text,
	Combo_Num,

	// ランク用画像
	Rank_IMG_D,
	Rank_IMG_C,
	Rank_IMG_B,
	Rank_IMG_A,
	Rank_IMG_S,

	// 背景画像
	BG_ground,
	BG_building,
	BG_sky,

	// 追加
	Tex_ID_MAX
};

enum Audio_ID
{
	// BGM
	Title_BGM,
	Select_BGM,
	Tutorial_BGM,
	Game_BGM,
	Result_BGM,
	// Song_BGM
	Game_Song001_BGM,
	Game_Song002_BGM,
	Game_Song003_BGM,
	Edit_Game_BGM,
	// SE
	Title_AnyPress_SE,
	MusicSelect_Decide_SE,
	CursorMove_SE,
	Cancel_SE,
	Note_Success_SE,
	Check_SE,
	// CV
	// コンフィグのテストボイス
	TestCV,
	// チュートリアルのボイスデータ
	Rezz_Tutorial_001,
	Rezz_Tutorial_002,
	Rezz_Tutorial_003,
	Rezz_Tutorial_004,
	Rezz_Tutorial_005,
	Rezz_Tutorial_006,
	Rezz_Tutorial_007,
	Rezz_Tutorial_008,
	Rezz_Tutorial_009,
	Rezz_Tutorial_010,
	Rezz_Tutorial_011,
	Rezz_Tutorial_012,
	Rezz_Tutorial_013,
	Rezz_Tutorial_014,
	Rezz_Tutorial_015,
	Rezz_Tutorial_016,
	Rezz_Tutorial_017,
	Rezz_Tutorial_018,

	Riff_Tutorial_001,
	Riff_Tutorial_002,
	Riff_Tutorial_003,

	// 追加
	Audio_ID_MAX
};


enum ScoreID
{
	SCORE_SONG001_EASY,
	SCORE_SONG001_NORMAL,
	SCORE_SONG001_HARD,
	SCORE_SONG002_EASY,
	SCORE_SONG002_NORMAL,
	SCORE_SONG002_HARD,
	SCORE_SONG003_EASY,
	SCORE_SONG003_NORMAL,
	SCORE_SONG003_HARD,

	SCORE_EDIT,
	// 追加
	SCORE_MAX
};

enum class DataFileID 
{
	Tutorial_Text,
	Music_Select_song_text,
	Music_Select_song_master,
	Music_Select_song_difficulty,
	UI_Text,
	Character_Select_text,
	Max
};

enum class FontID
{
	Japanese_Font,
	English_Font,
	Korean_Font,
	Chinese_Font,

	Max
};

void Resouce_ManagerInitialize();
void Resouce_ManagerFinalize();

int Resouce_Manager_GetTexId(Tex_ID id);
int Resouce_Manager_GetAudioId(Audio_ID id);
const char* Resouce_Manager_GetDataFilePath(DataFileID id);
const wchar_t* Resouce_Manager_GetFontFilePath(FontID id);

struct ScoreNote
{
	float time;
	int lane;
	int type;
	float param;
};

struct ScoreData
{
	std::vector<ScoreNote> notes;
};

const ScoreData& Resouce_Manager_GetScoreData(ScoreID id);
void Resouce_Manager_Reload_EditScoreData(); // エディットモードで編集時用

#endif // !RESOUCE_MANAGER_H
