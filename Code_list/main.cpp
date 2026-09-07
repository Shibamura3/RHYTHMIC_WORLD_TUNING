//ウィンドウの表示　[main.cpp]
//2025/06/03---2025/06/06
//
#ifndef WINVER
#define WINVER 0x0A00          // Windows 10 を指定
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00    // Windows 10 を指定
#endif

#include "game_window.h"
#include "direct3d.h"
#include "shader.h"
#include "sprite.h"
#include "texture.h"
#include "sprite_animu.h"
#include "fade.h"
#include "system_timer.h"
#include "Audio.h"
#include "key_logger.h" //キーボード入力
#include "pad_logger.h"
#include "mouse.h" //マウス入
#include "resource_manager.h"
#include "text_manager.h"
#include "ui_text_manager.h"
#include "scene.h"
#include "play_record_manager.h"

#include <cwchar>
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Xinput.h>
#pragma comment(lib,"xinput.lib")

#include <DirectXMath.h>
using namespace DirectX;

// デバッグ用FPS表示
static constexpr int FPS_TEXT_BUFFER_SIZE = 32;

static constexpr float FPS_TEXT_X = 10.0f;
static constexpr float FPS_TEXT_Y = 10.0f;
static constexpr float FPS_TEXT_SCALE = 0.65f;
static constexpr DirectX::XMVECTORF32 FPS_TEXT_COLOR{1.0f,0.2f,0.2f,1.0};

static void DrawDebugFps(double fps){
#if defined(DEBUG) || defined(_DEBUG)

	wchar_t fpsText[FPS_TEXT_BUFFER_SIZE]{};

	swprintf_s(
		fpsText,
		FPS_TEXT_BUFFER_SIZE,
		L"FPS: %.1f",
		fps
	);

	Text_Begin();

	Text_Draw(
		fpsText,
		FPS_TEXT_X,
		FPS_TEXT_Y,
		FPS_TEXT_COLOR,
		FPS_TEXT_SCALE
	);

	Text_End();

#else

	(void)fps;

#endif
}

//ウィンドウの表示メイン文
int APIENTRY WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLife, _In_ int nCmdShow) {
	//一箇所描いておけばok ないと音と画像など読み込むときに使えなくなる
	(void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	
	//DPIスケーリング
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	//分割したウィンドウ表示の呼び出し
	HWND hWnd = GameWindow_Create(hinstance);

	//Direct3dの初期化
	SystemTimer_Initialize();
	KeyLogger_Initialize();
	PadLogger_Initalize();
	InitAudio(); // Audio_Initialize

	Direct3D_Initialize(hWnd);
	Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
	Texture_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
	Sprite_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
	SpriteAnim_Initialize();
	Fade_Initialize();
	Resouce_ManagerInitialize();
	Text_Initialize();

	Scene_Initialize(); // ゲームのシーン全体の処理が入っている
	PlayRecord_Initialize();
	UI_Text_Load();

	//ウィンドウの表示
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	//fps.実行フレーム計測用
	double exec_last_time = SystemTimer_GetTime();
	double fps_last_time = exec_last_time;
	double current_time = 0.0;
	ULONG frame_count = 0;
	double fps = 0.0;

	//メッセージループ
	MSG msg;

	do {
		//peekメッセージを貯めてから処理
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { // ウィンドウメッセージが来ていたら 
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		else { // ゲームの処理 

			//fps計測
			current_time = SystemTimer_GetTime(); //システム時刻を取得
			double elapsed_time = current_time - fps_last_time; // fps計測用の経過時間こ計算

			if (elapsed_time >= 1.0) {// 1秒ごとに計測

				fps = frame_count / elapsed_time; // 
				fps_last_time = current_time; //  FPSを測定した時刻を取得
				frame_count = 0; // フレームカウントを初期化

			}

			//実行フレームの計測 1/60ごとに実行
			elapsed_time = current_time - exec_last_time;
			if (elapsed_time >= (1.0 / 60.0)) {
				//if (true) {
				exec_last_time = current_time; // 処理した時刻を保存

				//ゲームの更新
				KeyLogger_Update();//キーロガーのアップデート、一回だけしか宣言してはいけない
				PadLogger_Update();
				Scene_Update(elapsed_time);//ゲームのアップデート
				//経過時間を使用して速度を設定
				SpriteAnim_Update(elapsed_time);
				Fade_Update(elapsed_time);

				//ゲームの描画
				Direct3D_Clear();//画用紙を白（クリアー）する
				Sprite_Begin();//スプライト描き始め
				Scene_Draw();//ゲーム本体の描画
				Fade_Draw();

				DrawDebugFps(fps);

				Direct3D_Present();//画用紙にプレゼントする

				Scene_Refresh();

				frame_count++;
			}

		}
	} while (msg.message != WM_QUIT);

	//描画とは逆順に後片付け
	PlayRecord_Finalize();
	Scene_Finalize();

	Text_Finalize();
	Resouce_ManagerFinalize();
	Fade_Finalize();
	SpriteAnim_Finalize();
	Sprite_Finalize();
	Texture_Finalize();
	Shader_Finalize();
	Direct3D_Finalize();

	return (int)msg.wParam;
}