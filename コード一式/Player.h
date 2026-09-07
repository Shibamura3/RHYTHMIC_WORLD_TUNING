/* 
	プレイヤーの制御：player.h

	2026/05/08	hibiki sakuma
*/


#ifndef PLAYER_H
#define PLAYER_H

#include "resource_manager.h"

#include <DirectXMath.h>

// スキル種別
enum class PlayerSkillType{
    AttackUp,        // リフ：一定時間攻撃力アップ
    GreatToPerfect,  // ルナ：判定強化
    HealAndScoreUp,  // メロディ：回復とスコアアップ
};

// プレイヤーの状態
enum class PlayerState {
    Normal,
    Success,
    Miss,
    Fever,
    Success_Fever,
    Miss_Fever
};

// キャラクターID
enum class PlayerCharacter {
    Riff, // メインキャラクター:赤　リフ
    Luna, // サブキャラ①：青　ルナ
    Melody, // サブキャラ②：黄　メロディ

    Max
};

class Player {
private:
    // 座標
    DirectX::XMFLOAT2 m_position{};
    float m_baseY{};
    // プレイヤーの現在の状態
    PlayerCharacter m_character{ PlayerCharacter::Riff };
    PlayerState m_state = PlayerState::Normal;

    // 浮遊演出
    float m_floatTimer{};

    // 状態維持用タイマー
    float m_stateTimer{};

    // キャラクターの画像
    float m_texSize{};
    int m_texNormal{};
    int m_texSuccess{};
    int m_texMiss{};
    int m_texFeverNormal{};
    int m_texFeverSuccess{};
    int m_texFeverMiss{};

    // ステータス
    float m_hp{ 100.0f };
    float m_feverGauge{ 0.0f };
    bool  m_isFever{ false };

    // スキル関連
    PlayerSkillType m_skillType{ PlayerSkillType::AttackUp };

    float m_feverDuration{ 5.0f };     // スキル継続時間
    float m_feverTimer{ 0.0f };        // 残り時間
    float m_attackMultiplier{ 1.0f };  // 攻撃倍率
    float m_scoreMultiplier{ 1.0f };   // スコア倍率
    bool m_greatToPerfect{ false };    // 判定強化
public:

    Player();
    ~Player();

    void Initialize(PlayerCharacter character);
    void Update(double elapsedTime);
    void Draw();

    // 判定結果を外部から通知
    void OnNoteSuccess(bool perfect, bool add_fevergauge = true);
    void OnNoteMiss();
    void OnFeverStart();
    void OnFeverEnd();

    // 状態取得
    DirectX::XMFLOAT2 GetPosition() const { return m_position; }
    PlayerCharacter GetCharacter() const { return m_character; }

    // スキル情報用
    bool IsFever() const { return m_isFever; }
    float GetAttackDamage(float baseDamage) const;
    int GetSkillScore(int baseScore) const;
    bool IsGreatToPerfectActive() const { return m_greatToPerfect; }

    // ステータスの取得
    float GetHp() const;
    float GetMaxHP()const;
    float GetFeverGauge() const;
    float GetFeverGaugeMAX() const;
    const wchar_t* GetSkillName() const;

    // チュートリアル用
    void SetFeverGauge(float value);

private: // 内部関数
    void UpdateFloating(double elapsedTime);
    void UpdateStateTimer(double elapsedTime);
    void LoadCharacterTextures();
    void ActivateSkill();
    void UpdateFever(double elapsedTime);

};

// どこからでもプレイヤーの情報を確認できる
Player* GetPlayer();

#endif // !Player_H
