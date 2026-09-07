/*
    スコア全体の制御：score_manager.cpp

    2026/08/23	hibiki sakuma
*/

#include "score_manager.h"

// 定数宣言
static constexpr float RANK_RATE_S = 0.90f;
static constexpr float RANK_RATE_A = 0.80f;
static constexpr float RANK_RATE_B = 0.50f;
static constexpr float RANK_RATE_C = 0.30f;

float Score_CalculateRate(int score, int maxScore){
    if (maxScore <= 0) return 0.0f;

    float rate = static_cast<float>(score) / static_cast<float>(maxScore);

    if (rate >= 1.0f) rate = 1.0f;
    if (rate < 0.0f) rate = 0.0f;

    return rate;
}

Rank_Type Score_CalculateRank(int score, int maxScore){
    const float scoreRate = Score_CalculateRate(score, maxScore);

    if (scoreRate >= RANK_RATE_S) return Rank_S;

    if (scoreRate >= RANK_RATE_A) return Rank_A;

    if (scoreRate >= RANK_RATE_B) return Rank_B;

    if (scoreRate >= RANK_RATE_C) return Rank_C;

    return Rank_D;
}