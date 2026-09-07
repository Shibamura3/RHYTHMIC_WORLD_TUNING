/*
    スコア全体の制御：score_manager.h

    2026/08/23	hibiki sakuma
*/

#ifndef SCORE_MANAGER_H
#define SCORE_MANAGER_H

#include "score_type.h"

Rank_Type Score_CalculateRank(int score, int maxScore);

float Score_CalculateRate(int score, int maxScore);

#endif // !SCORE_MANAGER_H