/*
	敵の制御：enemy.h

	2026/05/15	hibiki sakuma
*/

#ifndef ENEMY_H
#define ENEMY_H

#include <DirectXMath.h>

// モーションパターン
enum class EnemyMotion {
    Motion1, // モーション①（暴走・ランダム）
    Motion2, // モーション②（上下＋スケール）
    Motion3, // モーション③（高速回転→減速）
    ReturnToCenter // 中心に戻る
};


// 敵種類（曲ごとに対応）
enum class EnemyType {
    Tutorial,
    Song001,
    Song002,
    Song003,
    None
};


class Enemy {
private: // 変数

    DirectX::XMFLOAT2 m_position{};

    // 状態
    EnemyType m_type{};
    float m_hp{};

    // 描画
    int m_texId{};

    // 暴走演出
    float m_timer{};
    float m_angle{};
    float m_scale{ 1.0f };

    EnemyMotion m_motion{};
    float m_motionTimer{};
    float m_motionDuration{};
    bool m_isReturning{};

    // ダメージ色
    float m_colorTimer{};
    DirectX::XMFLOAT4 m_color{ 1,1,1,1 };


public: // 外部関数

    Enemy();
    ~Enemy();

    void Initialize(EnemyType type);
    void Update(double elapsedTime);
    void Draw();

    void Damage(float damage);
    bool IsDead() const{ return m_hp <= 0.0f; }
    float GetHp();
    float GetMaxHp();

    DirectX::XMFLOAT2 GetPosition() const { return m_position; }


private: // 内部関数
    void SelectMotion(); // モーション選択
    void ReturnCenter(); // 中央に戻る
    void UpdateReturn(double elapsedTime);
    void MotionPattern1(double elapsedTime);
    void MotionPattern2(double elapsedTime);
    void MotionPattern3(double elapsedTime);
};

Enemy* GetEnemy();

#endif // !ENEMYT_H

