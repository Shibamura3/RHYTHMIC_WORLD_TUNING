/*
	ゲームパッドの制御：pad_logger.h

	2025/12/03	hibiki sakuma
*/

#include "SDL.h"
#include "pad_logger.h"
#include "Key_logger.h"
#include <iostream>
using namespace DirectX;

// 定数宣言
static constexpr float DEADZONE = 10000.0f;
static constexpr float MAX_VAL = 32767.0f;

// SDLのコントローラーハンドル
static SDL_GameController* g_GameController = nullptr;
// 操作モードの検知
static InputMode g_CurrentInputMode = InputMode::KeyboardMouse;
// コントローラー判定
static ControllerDisplayType g_ControllerDisplayType = ControllerDisplayType::Unknown;
// ボタン状態管理用
static bool g_CurrentButtons[SDL_CONTROLLER_BUTTON_MAX]{};
static bool g_PrevButtons[SDL_CONTROLLER_BUTTON_MAX]{};

// 内部関数
static void PadLogger_UpdateControllerType();

void PadLogger_Initalize(){
	// SDLの初期化
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1"); // 背後でも入力を取る設定
	if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) return;

	// 起動時に接続済みのものを探す
	for (int i = 0; i < SDL_NumJoysticks(); ++i){
		if (!SDL_IsGameController(i))
			continue;

		g_GameController = SDL_GameControllerOpen(i);

		if (g_GameController != nullptr){
			PadLogger_UpdateControllerType();
			break;
		}
	}
}

void PadLogger_Update() {
	// OSのメッセージキューからSDLのイベントキューへ情報を強制的に移す
	SDL_PumpEvents();

	SDL_Event event;
	// 溜まっているイベント（接続・切断など）をすべて処理
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_CONTROLLERDEVICEADDED) {
			if (!g_GameController) {
				g_GameController = SDL_GameControllerOpen(event.cdevice.which);
			}

			if (g_GameController != nullptr) {
				PadLogger_UpdateControllerType();
			}
		}
		// 切断処理
		else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
			if (g_GameController) {
				SDL_GameControllerClose(g_GameController);
				g_GameController = nullptr;
			}

			g_ControllerDisplayType = ControllerDisplayType::Unknown;

			for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
				g_CurrentButtons[i] = false;
				g_PrevButtons[i] = false;
			}
		}
	}

	// もしすでにオープン済みなら、毎フレーム入力を取る
	if (g_GameController) {
		for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
			g_PrevButtons[i] = g_CurrentButtons[i];
			g_CurrentButtons[i] = SDL_GameControllerGetButton(g_GameController, (SDL_GameControllerButton)i);
		}
	}
	
}

// ボタンが押されているか
bool PadLogger_IsPressed(DWORD user_index,SDL_GameControllerButton button){
	if (g_GameController == nullptr)
		return false;

	const int index = static_cast<int>(button);

	if (index < 0 || index >= SDL_CONTROLLER_BUTTON_MAX)
		return false;

	return g_CurrentButtons[index];
}

// 押された瞬間
bool PadLogger_IsTrigger(DWORD user_index, SDL_GameControllerButton button){
	if (g_GameController == nullptr)
		return false;

	const int index = static_cast<int>(button);

	if (index < 0 || index >= SDL_CONTROLLER_BUTTON_MAX)
		return false;

	return g_CurrentButtons[index] && !g_PrevButtons[index];
}

bool PadLogger_IsAnyButtonTrigger(DWORD user_index){
	if (g_GameController == nullptr)
		return false;

	for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i){
		const auto button =static_cast<SDL_GameControllerButton>(i);

		if (PadLogger_IsTrigger(user_index,button)){
			return true;
		}
	}

	return false;
}

// スティック取得
DirectX::XMFLOAT2 PadLogger_GetLeftThumbStick(WORD user_index) {
	if (!g_GameController) return { 0.0f, 0.0f };

	short rawX = SDL_GameControllerGetAxis(g_GameController, SDL_CONTROLLER_AXIS_LEFTX);
	short rawY = SDL_GameControllerGetAxis(g_GameController, SDL_CONTROLLER_AXIS_LEFTY);

	float fx = 0.0f;
	float fy = 0.0f;

	// X軸の計算（デッドゾーンから最大値の間を 0.0 ~ 1.0 にスケーリング）
	if (abs(rawX) > DEADZONE) {
		float sign = (rawX > 0) ? 1.0f : -1.0f;
		fx = sign * (abs(rawX) - DEADZONE) / (MAX_VAL - DEADZONE);
	}

	// Y軸の計算
	if (abs(rawY) > DEADZONE) {
		float sign = (rawY > 0) ? 1.0f : -1.0f;
		fy = sign * (abs(rawY) - DEADZONE) / (MAX_VAL - DEADZONE);
	}

	// クランプ（1.0を超えないように）
	fx = std::max(-1.0f, std::min(1.0f, fx));
	fy = std::max(-1.0f, std::min(1.0f, fy));

	return { fx, -fy };
}

DirectX::XMFLOAT2 PadLogger_GetRightThumbStick(WORD user_index){
	if (!g_GameController) return { 0.0f, 0.0f };

	short rawX = SDL_GameControllerGetAxis(g_GameController, SDL_CONTROLLER_AXIS_RIGHTX);
	short rawY = SDL_GameControllerGetAxis(g_GameController, SDL_CONTROLLER_AXIS_RIGHTY);

	float fx = 0.0f;
	float fy = 0.0f;

	// X軸の計算（デッドゾーンから最大値の間を 0.0 ~ 1.0 にスケーリング）
	if (abs(rawX) > DEADZONE) {
		float sign = (rawX > 0) ? 1.0f : -1.0f;
		fx = sign * (abs(rawX) - DEADZONE) / (MAX_VAL - DEADZONE);
	}

	// Y軸の計算
	if (abs(rawY) > DEADZONE) {
		float sign = (rawY > 0) ? 1.0f : -1.0f;
		fy = sign * (abs(rawY) - DEADZONE) / (MAX_VAL - DEADZONE);
	}

	// クランプ（1.0を超えないように）
	fx = std::max(-1.0f, std::min(1.0f, fx));
	fy = std::max(-1.0f, std::min(1.0f, fy));

	return { fx, -fy };
}
float PadLogger_GetLeftTrigger(WORD user_index){
	if (!g_GameController) return 0.0f;
	// SDLの戻り値は 0 〜 32767
	short raw = SDL_GameControllerGetAxis(g_GameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
	return (float)raw / 32767.0f;
}

float PadLogger_GetRightTrigger(WORD user_index) {
	if (!g_GameController) return 0.0f;
	// SDLの戻り値は 0 〜 32767
	short raw = SDL_GameControllerGetAxis(g_GameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
	return (float)raw / 32767.0f;
}
InputMode PadLogger_GetInputMode(){
	return g_CurrentInputMode;
}
bool PadLogger_IsConnected(){
	return g_GameController != nullptr;
}

ControllerDisplayType PadLogger_GetControllerDisplayType()
{
	return g_ControllerDisplayType;
}

void PadLogger_UpdateControllerType(){
	if (g_GameController == nullptr){
		g_ControllerDisplayType = ControllerDisplayType::Unknown;

		return;
	}

	const SDL_GameControllerType type = SDL_GameControllerGetType(g_GameController);

	switch (type)
	{
	case SDL_CONTROLLER_TYPE_PS3:
	case SDL_CONTROLLER_TYPE_PS4:
	case SDL_CONTROLLER_TYPE_PS5:
		g_ControllerDisplayType = ControllerDisplayType::PlayStation;
		break;

	case SDL_CONTROLLER_TYPE_XBOX360:
	case SDL_CONTROLLER_TYPE_XBOXONE:
		g_ControllerDisplayType = ControllerDisplayType::Xbox;
		break;

	default:
		// 未知のコントローラーはXbox表示
		g_ControllerDisplayType = ControllerDisplayType::Xbox;
		break;
	}
}

