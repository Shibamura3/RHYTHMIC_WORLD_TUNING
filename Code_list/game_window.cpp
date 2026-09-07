//*****************************************************************
//
//
//
//
//******************************************************************

#include "game_window.h"
#include <algorithm>//真ん中にウィンドウをだすために必要
#include "keyboard.h"
#include "mouse.h"
#include "direct3d.h"

//ウィンドウの情報
static constexpr char WINDOW_CLASS[] = "GameWindows";//メインウィンドウクラス
static constexpr char TITLE[] = "RHYTHMIC WORLD : TUNING";//タイトルバーのテキスト

//メインウィンドウのサイズ計算　16:9
static constexpr int SCREEN_WIDTH = 16 * 100;
static constexpr int SCREEN_HIGHT = 9 * 100;

// ウィンドウ情報の保存
static HWND g_hWnd = nullptr;

//ウィンドウプロシージャのプロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND GameWindow_Create(HINSTANCE hinstance)
{
	//ウィンドウクラスの登録
	WNDCLASSEX wcex{};

	//初期化
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hinstance;
	wcex.hIcon = LoadIcon(hinstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	//wcex.lpszMenuName = nullptr;//メニュバーの表示、今回は使わない
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassEx(&wcex);

	RECT window_rect{ 0, 0, SCREEN_WIDTH, SCREEN_HIGHT };
	DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

	AdjustWindowRect(&window_rect, style, FALSE);

	const int WINDOW_WIDTH = window_rect.right - window_rect.left;
	const int WINDOW_HEIGHT = window_rect.bottom - window_rect.top;

	//デスクトップのサイズを取得
	//プライマリモニターの画面解像度取得
	int desktop_width = GetSystemMetrics(SM_CXSCREEN);
	int desktop_height = GetSystemMetrics(SM_CYSCREEN);

	//ウィンドウの表示位置を真ん中に調整する　 	
	const int WINDOW_X = std::max((desktop_width - WINDOW_WIDTH) / 2, 0);
	const int WINDOW_Y = std::max((desktop_height - WINDOW_HEIGHT) / 2, 0);

	g_hWnd = CreateWindow(
		WINDOW_CLASS,
		TITLE,
		style,
		//WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX),
		//WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		WINDOW_X,
		WINDOW_Y,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		nullptr, nullptr, hinstance, nullptr);

	return g_hWnd;
}

void GameWindow_SetClientSize(int clientWidth, int clientHeight){
	if (g_hWnd == nullptr){
		return;
	}

	DWORD style = static_cast<DWORD>(GetWindowLongPtr(g_hWnd, GWL_STYLE));

	RECT windowRect
	{
		0,
		0,
		clientWidth,
		clientHeight
	};

	AdjustWindowRect(&windowRect, style, FALSE);

	const int windowWidth = windowRect.right - windowRect.left;
	const int windowHeight = windowRect.bottom - windowRect.top;

	const int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	const int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

	const int windowX = std::max((desktopWidth - windowWidth) / 2, 0);
	const int windowY = std::max((desktopHeight - windowHeight) / 2, 0);

	SetWindowPos(
		g_hWnd,
		nullptr,
		windowX,
		windowY,
		windowWidth,
		windowHeight,
		SWP_NOZORDER |
		SWP_NOACTIVATE
	);
}

//ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_ACTIVATEAPP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {//エスケープキーの場合
			SendMessage(hWnd, WM_CLOSE, 0, 0); //sendmessageはオリジナルのメッセージを飛ばすこともできる
		}
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		break;
	case WM_CLOSE://ウィンドウを閉じようとしたとき、右上の×など
		if (MessageBox(hWnd, "終了してよろしいですか", "確認", MB_YESNO | MB_DEFBUTTON2) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY: //ウィンドウの破棄メッセージ
		PostQuitMessage(0);//WM_Quit を
		break;

	case WM_SIZE:
	{
		const unsigned int width = LOWORD(lParam);
		const unsigned int height = HIWORD(lParam);

		// 最小化中は0になるため更新しない
		if (wParam != SIZE_MINIMIZED && width > 0 && height > 0) {
			Direct3D_ResizeBackBuffer(width, height
			);
		}
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}