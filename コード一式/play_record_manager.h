/*
    ÉvÉåÉCèÓïÒÇÃä«óùÅFplay_record_manager.h

    2026/08/24	hibiki sakuma
*/

#ifndef PLAY_RECORD_MANAGER_H
#define PLAY_RECORD_MANAGER_H

#include "Music_select.h"
#include "score_type.h"

#include <string>

struct DifficultyRecord
{
    bool hasPlayed = false;
    bool hasCleared = false;

    int highScore = 0;
    Rank_Type highRank = Rank_D;
};

void PlayRecord_Initialize();
void PlayRecord_Finalize();

bool PlayRecord_Load();
bool PlayRecord_Save();

void PlayRecord_Update(const std::string& songId, GameDifficulty difficulty, int score, Rank_Type rank, bool hasCleared);

const DifficultyRecord* PlayRecord_Get(const std::string& songId, GameDifficulty difficulty);

#endif //!PLAY_RECORD_MANAGER_H