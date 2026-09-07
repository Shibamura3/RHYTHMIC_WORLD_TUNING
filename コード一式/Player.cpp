/*
	プレイヤーの制御：player.cpp

	2026/05/08	hibiki sakuma
*/

#include "player.h"
#include "Audio.h"
#include "sprite.h"
#include "direct3d.h"
#include <cmath>
using namespace DirectX;

// 定数宣言
static constexpr float PLAYER_X = 0.0f;
static constexpr float PLAYER_Y_RATE = 0.50f;
static constexpr float PLAYER_DAMAGE = 5.0f;
static constexpr float PLAYER_START_HP = 500.0f;
static constexpr float PLAYER_FEVER_MAX = 100.0f;
static constexpr float STATE_DISPLAY_TIME = 0.50f;
// スキル設定
static constexpr float DEFAULT_MULTIPLIER = 1.0f;
static constexpr float RIFF_ATTACK_MULTIPLIER = 2.0f;
static constexpr float MELODY_HEAL_AMOUNT = 25.0f;
static constexpr float MELODY_SCORE_MULTIPLIER = 2.0f;
static constexpr float SKILL_DURATION = 5.0f;
// 浮遊設定
static constexpr float FLOAT_AMPLITUDE = 10.0f; // 振れ幅
static constexpr float FLOAT_SPEED = 3.0f; // 速度

// 外部からは見えない、このファイルだけの静的変数
static Player* g_PlayerInstance = nullptr;

Player::Player() {
    g_PlayerInstance = this; // 生成されたときに自分を登録
}

Player::~Player() {
    if (g_PlayerInstance == this) {
        g_PlayerInstance = nullptr; // 破棄されたら解除
    }
}

// 実体の実装
Player* GetPlayer() {
    return  g_PlayerInstance;
}

void Player::Initialize(PlayerCharacter character) {
    float h = (float)Direct3D_GetBackBufferHeight();
    m_character = character;
    m_state = PlayerState::Normal;

    m_texSize = h * 0.4f;
    m_baseY = h * PLAYER_Y_RATE - m_texSize * 0.5f;

    m_position = { PLAYER_X, m_baseY};
    m_floatTimer = 0.0f;
    m_stateTimer = STATE_DISPLAY_TIME;

    // ステータスの初期化
    m_hp = PLAYER_START_HP;
    m_feverGauge = 0.0f;
    m_isFever = false;

    m_feverDuration = SKILL_DURATION;
    m_feverTimer = 0.0f;
    m_attackMultiplier = DEFAULT_MULTIPLIER;
    m_scoreMultiplier = DEFAULT_MULTIPLIER;
    m_greatToPerfect = false;

    // キャラごとのスキルの設定
    switch (m_character)
    {
    case PlayerCharacter::Riff:
        m_skillType = PlayerSkillType::AttackUp;
        break;

    case PlayerCharacter::Luna:
        m_skillType = PlayerSkillType::GreatToPerfect;
        break;

    case PlayerCharacter::Melody:
        m_skillType = PlayerSkillType::HealAndScoreUp;
        break;

    default:
        m_skillType = PlayerSkillType::AttackUp;
        break;
    }

    // 画像の読み込み
    LoadCharacterTextures();

}

void Player::Update(double elapsed_time) {
    UpdateFloating(elapsed_time);

    UpdateStateTimer(elapsed_time);

    // FEVERゲージの処理
    UpdateFever(elapsed_time);

}

void Player::Draw() {
    int texId = m_texNormal;

    switch (m_state) {
        case PlayerState::Success:       texId = m_texSuccess; break;
        case PlayerState::Miss:          texId = m_texMiss;    break;
        case PlayerState::Fever:         texId = m_texFeverNormal;   break;
        case PlayerState::Success_Fever: texId = m_texFeverSuccess; break;
        case PlayerState::Miss_Fever:    texId = m_texFeverMiss;    break;
        default: break;
    }

    Sprite_Draw(
        texId,
        m_position.x,m_position.y,
        m_texSize, m_texSize
    );

}

float Player::GetAttackDamage(float baseDamage) const{
    return baseDamage * m_attackMultiplier;
}

int Player::GetSkillScore(int baseScore) const{
    return static_cast<int>(static_cast<float>(baseScore) * m_scoreMultiplier);
}

float Player::GetHp() const{
    return m_hp;
}

float Player::GetMaxHP() const{
    return PLAYER_START_HP;
}

float Player::GetFeverGauge() const{
    return m_feverGauge;
}

float Player::GetFeverGaugeMAX() const{
    return PLAYER_FEVER_MAX;
}

const wchar_t* Player::GetSkillName() const
{
    switch (m_character)
    {
    case PlayerCharacter::Riff:
        return L"OVER DRIVE";

    case PlayerCharacter::Luna:
        return L"PERFECT SYNC";

    case PlayerCharacter::Melody:
        return L"HEALING BEAT";
    default:
        return L"";
    }
}

void Player::SetFeverGauge(float value){
    if (value < 0.0f) value = 0.0f;

    if (value > PLAYER_FEVER_MAX) value = PLAYER_FEVER_MAX;

    m_feverGauge = value;
}

// 浮遊
void Player::UpdateFloating(double elapsed_time) {
    m_floatTimer += static_cast<float>(elapsed_time) * FLOAT_SPEED;
    float offset = std::sinf(m_floatTimer) * FLOAT_AMPLITUDE;

    m_position.y = m_baseY + offset;
}

// 状態管理
void Player::UpdateStateTimer(double elapsedTime) {
    if (m_state == PlayerState::Normal || m_state == PlayerState::Fever)
        return;

    m_stateTimer -= static_cast<float>(elapsedTime);
    if (m_stateTimer <= 0.0f) {
        m_state = m_isFever ? PlayerState::Fever : PlayerState::Normal;
    }
}


// 判定通知
void Player::OnNoteSuccess(bool perfect, bool add_fevergauge) {
    if (!add_fevergauge) return; // チュートリアル中の一部では加算しない

    if (m_isFever) {
        m_state = PlayerState::Success_Fever;
    } else {
        m_state = PlayerState::Success;
        m_feverGauge += perfect ? 5.0f : 1.0f;

        if (m_feverGauge >= PLAYER_FEVER_MAX) {
            m_feverGauge = PLAYER_FEVER_MAX;
            OnFeverStart();
        }
    }
    PlayAudio(Resouce_Manager_GetAudioId(Note_Success_SE));
    m_stateTimer = STATE_DISPLAY_TIME;
}

void Player::OnNoteMiss() {

    if (m_isFever) {
        m_state = PlayerState::Miss_Fever;
        m_stateTimer = 0.50f;
    } else {
        m_state = PlayerState::Miss;
        m_stateTimer = 0.50f;
    }

    m_stateTimer = STATE_DISPLAY_TIME;

    //m_hp -= PLAYER_DAMAGE;
    if (m_hp < 0.0f) m_hp = 0.0f;
}

void Player::OnFeverStart() {
    if (m_isFever) return;

    m_isFever = true;
    m_state = PlayerState::Fever;

    m_feverGauge = PLAYER_FEVER_MAX;
    m_feverTimer = m_feverDuration;

    ActivateSkill();
}

void Player::OnFeverEnd(){
    m_isFever = false;

    m_feverGauge = 0.0f;
    m_feverTimer = 0.0f;

    // 全スキル効果を解除
    m_attackMultiplier = DEFAULT_MULTIPLIER;
    m_scoreMultiplier = DEFAULT_MULTIPLIER;
    m_greatToPerfect = false;

    m_state = PlayerState::Normal;
}

void Player::LoadCharacterTextures() {
    switch (m_character) {
    case PlayerCharacter::Riff:
        m_texNormal  = Resouce_Manager_GetTexId(SD_Riff_main);
        m_texSuccess = Resouce_Manager_GetTexId(SD_Riff_success);
        m_texMiss    = Resouce_Manager_GetTexId(SD_Riff_miss);
        m_texFeverNormal = Resouce_Manager_GetTexId(SD_Riff_main_FEVER);
        m_texFeverSuccess = Resouce_Manager_GetTexId(SD_Riff_success_FEVER);
        m_texFeverMiss = Resouce_Manager_GetTexId(SD_Riff_miss_FEVER);

        break;
    case PlayerCharacter::Luna:
        m_texNormal  = Resouce_Manager_GetTexId(SD_Luna_main);
        m_texSuccess = Resouce_Manager_GetTexId(SD_Luna_success);
        m_texMiss    = Resouce_Manager_GetTexId(SD_Luna_miss);
        m_texFeverNormal  = Resouce_Manager_GetTexId(SD_Luna_main_FEVER);
        m_texFeverSuccess = Resouce_Manager_GetTexId(SD_Luna_success_FEVER);
        m_texFeverMiss    = Resouce_Manager_GetTexId(SD_Luna_miss_FEVER);

        break;

    case PlayerCharacter::Melody:
        m_texNormal = Resouce_Manager_GetTexId(SD_Melody_main);
        m_texSuccess = Resouce_Manager_GetTexId(SD_Melody_success);
        m_texMiss = Resouce_Manager_GetTexId(SD_Melody_miss);
        m_texFeverNormal = Resouce_Manager_GetTexId(SD_Melody_main_FEVER);
        m_texFeverSuccess = Resouce_Manager_GetTexId(SD_Melody_success_FEVER);
        m_texFeverMiss = Resouce_Manager_GetTexId(SD_Melody_miss_FEVER);

        break;

    }
}

void Player::ActivateSkill()
{
    // スキル効果を初期状態へ戻す
    m_attackMultiplier = DEFAULT_MULTIPLIER;
    m_scoreMultiplier = DEFAULT_MULTIPLIER;
    m_greatToPerfect = false;

    switch (m_skillType)
    {
    case PlayerSkillType::AttackUp:
        // Riff：一定時間、攻撃力上昇
        m_attackMultiplier = RIFF_ATTACK_MULTIPLIER;
        break;

    case PlayerSkillType::GreatToPerfect:
        // Luna：一定時間、GreatをPerfectとして扱う
        m_greatToPerfect = true;
        break;

    case PlayerSkillType::HealAndScoreUp:
        // Melody：体力回復後、一定時間スコア上昇
        m_hp += MELODY_HEAL_AMOUNT;

        if (m_hp > PLAYER_START_HP){
            m_hp = PLAYER_START_HP;
        }

        m_scoreMultiplier = MELODY_SCORE_MULTIPLIER;
        break;

    default:
        break;
    }
}

void Player::UpdateFever(double elapsedTime){
    if (!m_isFever) return;

    m_feverTimer -= (float)elapsedTime;

    if (m_feverTimer < 0.0f) m_feverTimer = 0.0f;

    // スキル時間に合わせてゲージを減らす
    if (m_feverDuration > 0.0f) {
        m_feverGauge = PLAYER_FEVER_MAX * (m_feverTimer / m_feverDuration);
    }

    if (m_feverTimer <= 0.0f) {
        OnFeverEnd();
    }
}
