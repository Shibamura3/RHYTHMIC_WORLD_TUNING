/*
    ノーツの制御：note.cpp

    2026/05/22	hibiki sakuma
*/

#include "note.h"
#include "sprite.h"
#include "resource_manager.h"
#include "direct3d.h"
#include <cmath>

// 定数宣言
static constexpr float SPAWN_OFFSET = 1.75f; // 判定何秒前に表示始めるか
static constexpr float NOTE_SPEEF_DEFAULT = 500.0f;
static constexpr float RAPID_VISUAL_INTERVAL = 0.1f;
// 判定幅
static constexpr float HIT_TIME = 0.08f;
static constexpr float MISS_TIME = 0.10f;
static constexpr float PERFECT_TIME = 0.03f;
static constexpr float LONG_TICK_INTERVAL = 0.25f;

// 変数宣言
static float g_NoteSpeed = NOTE_SPEEF_DEFAULT;
static float g_NoteSize = -1;
static float g_NoteSpawnX = -1;
Note::Note() {}
Note::~Note() {}

void Note::Initialize(float time, int lane, int type, float param)
{
    m_time = time;
    m_lane = lane;
    m_type = type;
    m_param = param;
    m_active = true;

    m_state = NoteState::Note_Waiting;

    // ロング/連打は param を秒数として扱う
    m_endTime = m_time + m_param;

    m_lastTickTime = 0.0f;
    m_rapidSuccess = false;

    m_texId[NOTE_NORMAL] = Resouce_Manager_GetTexId(Notes_Normal);
    m_texId[NOTE_LONG] = Resouce_Manager_GetTexId(Notes_tail);

    m_position = { 0.0f, 0.0f };
}

void Note::Update(float currentTime, float judgeX)
{
    float delta = m_time - currentTime;

    m_position.x = judgeX + delta * g_NoteSpeed;
    // ノーツの発生位置を画面依存
    float spawnDistance = (float)Direct3D_GetBackBufferWidth() * 0.8f;
    g_NoteSpawnX = spawnDistance - judgeX;
    // レーンY
    float h = (float)Direct3D_GetBackBufferHeight();
    float centerY = h * 0.5f;

    if (m_lane == 0) m_position.y = centerY - h * 0.1f;
    if (m_lane == 1) m_position.y = centerY + h * 0.1f;

}


void Note::Draw(float currentTime)
{
    if (!m_active) return;

    float delta = m_time - currentTime;
    float spawnTime = g_NoteSpawnX / g_NoteSpeed;
    float tailLength = 0;
    float capWidth = 0;
    float centerWidth = 0;
    g_NoteSize = (float)Direct3D_GetBackBufferHeight() * 0.12f;

    // 出現範囲チェック
    if (delta < spawnTime) {
        switch (m_type)
        {
        case NOTE_NORMAL:
            Sprite_Draw(m_texId[NOTE_NORMAL],
                m_position.x - g_NoteSize * 0.5f, 
                m_position.y - g_NoteSize * 0.5f,
                g_NoteSize, g_NoteSize);
            break;

        case NOTE_LONG:
            tailLength = g_NoteSpeed * m_param;

            capWidth = g_NoteSize;
            centerWidth = tailLength - capWidth * 2.0f;
            if (centerWidth < 0.0f) centerWidth = 0.0f;
            // ロングテール画像の描画
            // 左端
            Sprite_Draw(
                Resouce_Manager_GetTexId(Notes_tail_head),
                m_position.x, m_position.y - g_NoteSize * 0.5f,
                capWidth, g_NoteSize
            );
            // 中央
            Sprite_Draw(
                Resouce_Manager_GetTexId(Notes_tail_body),
                m_position.x + capWidth, m_position.y - g_NoteSize * 0.5f,
                centerWidth, g_NoteSize
            );
            // 右端
            Sprite_Draw(
                Resouce_Manager_GetTexId(Notes_tail_tail),
                m_position.x + capWidth + centerWidth, m_position.y - g_NoteSize * 0.5f,
                capWidth, g_NoteSize
            );

            Sprite_Draw(m_texId[NOTE_NORMAL], // 上に通常ノーツを描画
                m_position.x - g_NoteSize * 0.5f, 
                m_position.y - g_NoteSize * 0.5f,
                g_NoteSize, g_NoteSize);
            break;

        case NOTE_RAPID:
            tailLength = g_NoteSpeed * m_param;
            Sprite_Draw(m_texId[NOTE_LONG], // tailを下に描画
                m_position.x - g_NoteSize * 0.5f, 
                m_position.y - g_NoteSize * 0.5f,
                tailLength + g_NoteSize * 0.5f, g_NoteSize,
                {1.0f,1.0f,1.0f,0.5f} // 半透明
            );

            int count = (int)(m_param / RAPID_VISUAL_INTERVAL) + 1;
            float intervalPx = g_NoteSpeed * RAPID_VISUAL_INTERVAL;
            for (int i = 0; i < count; i++) {
                Sprite_Draw(
                    m_texId[NOTE_NORMAL],
                    m_position.x + intervalPx * i - g_NoteSize * 0.5f,
                    m_position.y - g_NoteSize * 0.5f,
                    g_NoteSize, g_NoteSize
                );
            }
            break;
        }

    }

}

float Note::GetTime() const
{
    return m_time;
}

NoteJudgeResult Note::UpdateJudge(float currentTime, const bool triggerLane[2], const bool pressedLane[2])
{
    if (!m_active)
        return NoteJudgeResult::Note_None;

    float delta = currentTime - m_time;

    bool trigger = triggerLane[m_lane];
    bool pressed = pressedLane[m_lane];

    switch (m_type) {
    case NOTE_NORMAL:
        if (trigger && fabs(delta) < HIT_TIME)
        {
            m_active = false;

            if (fabs(delta) < PERFECT_TIME)
                return NoteJudgeResult::Note_Perfect;

            return NoteJudgeResult::Note_Great;
        }

        if (delta > MISS_TIME)
        {
            m_active = false;
            return NoteJudgeResult::Note_Miss;
        }

        break;

    case NOTE_LONG:
        // 最初の通常ノーツ部分
        if (m_state == NoteState::Note_Waiting) {
            if (trigger && fabs(delta) < HIT_TIME) {
                m_state = NoteState::Note_Holding;
                m_lastTickTime = currentTime + LONG_TICK_INTERVAL;

                // 最初の判定は通常ノーツと同じ
                if (fabs(delta) < PERFECT_TIME)
                    return NoteJudgeResult::Note_Perfect;

                return NoteJudgeResult::Note_Great;
            }

            if (delta > MISS_TIME) {
                m_active = false;
                m_state = NoteState::Note_Finished;
                return NoteJudgeResult::Note_Miss;
            }
        }
        // 長押し中
        else if (m_state == NoteState::Note_Holding) {
            // 途中で離したら失敗
            if (!pressed && currentTime < m_endTime) {
                m_active = false;
                m_state = NoteState::Note_Finished;
                return NoteJudgeResult::Note_Miss;
            }

            // 押している間、一定間隔で加点
            if (pressed && currentTime < m_endTime) {
                if (currentTime - m_lastTickTime >= LONG_TICK_INTERVAL) {
                    m_lastTickTime += LONG_TICK_INTERVAL;
                    return NoteJudgeResult::Note_LongTick;
                }
            }

            // 終了時刻を超えたら成功終了
            if (currentTime >= m_endTime) {
                m_active = false;
                m_state = NoteState::Note_Finished;

                return NoteJudgeResult::Note_LongComplete;
            }
        }
        break;

    case NOTE_RAPID:
        float startTime = m_time - HIT_TIME;
        float endTime = m_endTime;

        // 区間中に押したら成功扱い
        if (currentTime >= startTime && currentTime <= endTime) {
            if (trigger) {
                m_rapidSuccess = true;
                return NoteJudgeResult::Note_RapidTick;
            }
        }

        // 区間終了
        if (currentTime > endTime) {
            m_active = false;
            m_state = NoteState::Note_Finished;

            if (!m_rapidSuccess) {
                return NoteJudgeResult::Note_Miss;
            }

            return NoteJudgeResult::Note_RapidComplete;
        }
        break;
    
    }

    return NoteJudgeResult::Note_None;
}

void Note::SetNoteSpeed(float speed){
    g_NoteSpeed = speed;
}