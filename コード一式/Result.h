/*
	ÉäÉUÉãÉgâÊñ ÇÃêßå‰ÅFResult.h

	2026/05/22	hibiki sakuma
*/
#ifndef RESULT_H
#define RESULT_H

#include "Music_select.h"
#include "score_type.h"
#include "player.h"
#include <string>

enum class ResultType {
    FAIL,
    CLEAR,
};

struct GameResultData
{
	ResultType resultType = ResultType::FAIL;
	
	std::string songId;

	GameDifficulty difficulty = GameDifficulty::Easy;

	int score = 0;
	Rank_Type rank = Rank_D;

	int maxCombo = 0;
	int perfectCount = 0;
	int greatCount = 0;
	int missCount = 0;

	bool isAllPerfect = false;
	bool isFullCombo = false;

	PlayerCharacter character;
};

void Result_Initialize();
void Result_Finalize();
void Result_Update(double elapsed_time);
void Result_Draw();

void Result_SetData(const GameResultData& data);

#endif // !RESULT_H
