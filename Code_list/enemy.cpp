/*
	敵の制御：enemy.cpp

	2026/05/15	hibiki sakuma
*/


#include "enemy.h"
#include "sprite.h"
#include "direct3d.h"
#include "resource_manager.h"

#include <cmath>
#include <DirectXMath.h>
#include "Player.h"
using namespace DirectX;

// 定数宣言
static constexpr float ENEMY_IMG_SIZE = 384.0f; 
static constexpr float ENEMY_START_HP = 500.0f;

// 変数宣言
static Enemy * g_enemy = nullptr;
static float g_Enemy_X = -1;

Enemy::Enemy() {
	g_enemy = this;
}

Enemy::~Enemy() {
	if (g_enemy == this) g_enemy = nullptr;
}

Enemy* GetEnemy() {
	return g_enemy;
}


void Enemy::Initialize(EnemyType type){
	// 敵タイプを変数に設定
	m_type = type;

	float screenH = (float)Direct3D_GetBackBufferHeight();
	g_Enemy_X = (float)Direct3D_GetBackBufferWidth() * 0.85f;
	m_position = { g_Enemy_X, screenH * 0.5f};

	// 種類別テクスチャ
	switch (m_type) {
	case EnemyType::Tutorial: m_texId = Resouce_Manager_GetTexId(Enemy_Tutorial); break;
	case EnemyType::Song001 : m_texId = Resouce_Manager_GetTexId(Enemy_Song001); break;
	case EnemyType::Song002 : m_texId = Resouce_Manager_GetTexId(Enemy_Song002); break;
	case EnemyType::Song003 : m_texId = Resouce_Manager_GetTexId(Enemy_Song003); break;
	}

	// 変数の初期化
	m_hp = ENEMY_START_HP;
	m_timer = 0.0f;
	m_angle = 0.0f;
	m_scale = 1.0f;
	m_color = { 1.0f,1.0f,1.0f,1.0f };
	m_colorTimer = 0.0f;
	m_motion = EnemyMotion::ReturnToCenter;
	m_motionTimer = 0.0f;
	m_motionDuration = 0.0f;

}

void Enemy::Update(double elapsedTime) {
	// アニメーション演出
	m_timer += (float)elapsedTime;
	m_motionTimer += (float)elapsedTime;
	
	switch (m_motion)
	{
	case EnemyMotion::Motion1:
		MotionPattern1(elapsedTime);
		break;

	case EnemyMotion::Motion2:
		MotionPattern2(elapsedTime);
		break;

	case EnemyMotion::Motion3:
		MotionPattern3(elapsedTime);
		break;

	case EnemyMotion::ReturnToCenter:
		UpdateReturn(elapsedTime);
		break;
	}

	// モーション終了
	if (m_motion != EnemyMotion::ReturnToCenter && m_motionTimer >= m_motionDuration){
		ReturnCenter();
	}

	// ダメージ色戻す
	if (m_colorTimer > 0.0f) {
		m_colorTimer -= (float)elapsedTime;
		if (m_colorTimer <= 0.0f) {
			m_color = { 1.0f,1.0f,1.0f,1.0f };
		}
	}
}


void Enemy::Draw() {
	Sprite_DrawRotated(
		m_texId,
		m_position.x,m_position.y, // 表示場所
		ENEMY_IMG_SIZE * m_scale,ENEMY_IMG_SIZE * m_scale, // 表示サイズ
		m_angle,
		m_color
	); 

}

void Enemy::Damage(float damage) {
	m_hp -= damage;
	if (m_hp < 0) m_hp = 0;
	
	// ダメージ時、色の変更
	m_color = { 1.0f, 0.1f, 0.1f, 1.0f };
	m_colorTimer = 0.5f;
}

float Enemy::GetHp(){
	return m_hp;
}

float Enemy::GetMaxHp(){
	return ENEMY_START_HP;
}

void Enemy::SelectMotion(){
	int r = rand() % 3; // 3のみ不具合

	switch (r){
	case 0: m_motion = EnemyMotion::Motion1; break;
	case 1: m_motion = EnemyMotion::Motion2; break;
	case 2: m_motion = EnemyMotion::Motion3; break;
	}

	m_motionTimer = 0.0f;
	m_motionDuration = 1.5f + (rand() % 100 / 100.0f); // 1.5〜2.5秒

}

void Enemy::ReturnCenter(){
	m_motion = EnemyMotion::ReturnToCenter;
	m_motionTimer = 0.0f;
	m_motionDuration = 0.5f;

}

void Enemy::UpdateReturn(double elapsedTime){

	float centerY = Direct3D_GetBackBufferHeight() * 0.5f;
	float centerX = g_Enemy_X;
	float speed = 10.0f;

	// 線形補間
	m_position.x += (centerX - m_position.x) * speed * (float)elapsedTime;
	m_position.y += (centerY - m_position.y) * speed * (float)elapsedTime;

	m_angle *= 0.9f;
	if (fabs(m_scale - 1.0f) < 0.01f)
		m_scale = 1.0f;


	// 十分近づいたら次へ
	if (fabs(m_position.y - centerY) < 1.0f){
		SelectMotion();
	}

}

void Enemy::MotionPattern1(double elapsedTime){
	float range = 100.0f;

	float centerX = g_Enemy_X;
	float centerY = Direct3D_GetBackBufferHeight() * 0.5f;

	m_position.x = centerX + sinf(m_timer * 2.0f) * range;
	m_position.y = centerY + cosf(m_timer * 3.0f) * range;

	// 左右に揺れる
	m_angle = sinf(m_timer * 5.0f) * 0.2f;

}

void Enemy::MotionPattern2(double elapsedTime){

	float baseY = Direct3D_GetBackBufferHeight() * 0.5f;

	m_position.y = baseY + sinf(m_timer * 4.0f) * 120.0f;

	m_scale = 1.0f + sinf(m_timer * 6.0f) * 0.2f;
}

void Enemy::MotionPattern3(double elapsedTime){

	// 回転
	float speed = 10.0f * exp(-m_motionTimer * 2.0f);
	int dir = (rand() % 2) ? 1 : -1;
	m_angle += dir * speed * (float)elapsedTime;

	// 少し動く
	float baseX = g_Enemy_X;
	float baseY = Direct3D_GetBackBufferHeight() * 0.5f;

	m_position.x = baseX + sinf(m_timer * 3.0f) * 30.0f;
	m_position.y = baseY + cosf(m_timer * 2.0f) * 30.0f;


}