/*
    ノーツの制御：note.h

    2026/05/22	hibiki sakuma
*/

#ifndef NOTE_H
#define NOTE_H

#include <DirectXMath.h>

enum NoteType
{
    NOTE_NORMAL, // 0:通常ノーツ
    NOTE_LONG  , // 1:ロングノーツ
    NOTE_RAPID , // 2:連打ノーツ

    Type_MAX
};

enum class NoteState
{
    Note_Waiting,     // まだ判定待ち
    Note_Holding,     // ロングノーツ長押し中
    Note_RapidActive, // 連打ノーツ受付中
    Note_Finished     // 終了
};

enum class NoteJudgeResult
{
    Note_None,
    Note_Perfect,
    Note_Great,
    Note_Miss,
    Note_LongTick,
    Note_RapidTick,

    Note_LongComplete, // ロングノーツを最後まで押し切った
    Note_RapidComplete // 連打ノーツ区間を成功して終えた
};

class Note {
private:
    float m_time{};       // 判定時間
    int   m_lane{};       // レーン（0:上,1:下）
    int   m_type{};
    float m_param{};
    bool  m_active{ true };

    // 特殊ノーツ管理用
    NoteState m_state = NoteState::Note_Waiting;
    float m_endTime = 0.0f; // ロング・連打の終了時刻
    float m_lastTickTime = 0.0f; // ロング加点用
    bool m_rapidSuccess = false; // 連打中に押したか

    // 描画用
    DirectX::XMFLOAT2 m_position{};

    // テクスチャ
    int m_texId[Type_MAX]{};

public:
    Note();
    ~Note();

    void Initialize(float time, int lane, int type, float param);
    void Update(float currentTime ,float judgeX);
    void Draw(float currentTime);

    // 判定
    //bool CheckHit(float currentTime, int playerLane); // 判定結果
    float GetTime() const; // 判定精度
    //bool IsMiss(float currentTime); 
    NoteJudgeResult UpdateJudge(
        float currentTime,
        const bool triggerLane[2],
        const bool pressedLane[2]
    );
    // デモ画面で使用
    void ForceFinish() { m_active = false; }

    // エフェクト用位置情報取得
    DirectX::XMFLOAT2 GetPosition() const { return m_position; }

    bool IsActive() const { return m_active; }

    void SetNoteSpeed(float speed);
};

#endif // !NOTE_H