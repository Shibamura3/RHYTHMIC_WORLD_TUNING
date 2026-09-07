/*
	コンフィグ画面用ノーツ速度デモ：config_demo.cpp

	2026/07/10    hibiki sakuma
*/

#include "config_demo.h"

#include "Configu.h"
#include "resource_manager.h"
#include "sprite.h"
#include "direct3d.h"
#include "pad_logger.h"
#include <vector>

#include <DirectXMath.h>

using namespace DirectX;

// 定数
// ノーツ移動
static constexpr float NOTE_SPEED_DEFAULT = 500.0f;
static constexpr float NOTE_SPAWN_INTERVAL = 1.0f;

// レーン
static constexpr float LANE_X_RATE = 0.20f;
static constexpr float LANE_Y_RATE = 0.70f;
static constexpr float LANE_WIDTH_RATE = 0.70f;
static constexpr float LANE_HEIGHT_RATE = 0.10f;

// ノーツ
static constexpr float NOTE_SPAWN_X_RATE = 0.8f;
static constexpr float NOTE_SIZE_RATE = 0.15f;

// 判定位置
static constexpr float JUDGE_X_RATE = 0.25f;
static constexpr float JUDGE_SIZE_RATE = 0.11f;
static constexpr float JUDGE_ALPHA = 0.50f;
// データ型
struct DemoNote
{
	float x = 0.0f;
};

// 変数
static float g_NoteSpawnTimer = 0.0f;
static float g_ScreenWidth = 0.0f;
static float g_ScreenHeight = 0.0f;
static std::vector<DemoNote> g_DemoNotes;

// 内部関数
static void SpawnDemoNote();
static void RemoveFinishedNotes();

void Configu_Demo_Initialize(){
	g_ScreenWidth = static_cast<float>(Direct3D_GetBackBufferWidth());
	g_ScreenHeight = static_cast<float>(Direct3D_GetBackBufferHeight());

	g_NoteSpawnTimer = 0.0f;
	g_DemoNotes.clear();

	// 起動直後からノーツを1個表示
	SpawnDemoNote();
}

void Configu_Demo_Finalize(){
    g_DemoNotes.clear();

    g_NoteSpawnTimer = 0.0f;
}

void Configu_Demo_Update(double elapsed_time){
	const float deltaTime = static_cast<float>(elapsed_time);

	g_NoteSpawnTimer += deltaTime;

	while (g_NoteSpawnTimer >= NOTE_SPAWN_INTERVAL){
		g_NoteSpawnTimer -= NOTE_SPAWN_INTERVAL;

		SpawnDemoNote();
	}

	const float noteSpeed = NOTE_SPEED_DEFAULT * Configu_GetNoteSpeed();

	// 右から左へ移動
	for (DemoNote& note : g_DemoNotes){
		note.x -= noteSpeed * deltaTime;
	}

	RemoveFinishedNotes();
}

void Configu_Demo_Draw(){
    // レーン
    const float laneX = g_ScreenWidth * LANE_X_RATE;
    const float laneY = g_ScreenHeight * LANE_Y_RATE;

    const float laneWidth = g_ScreenWidth * LANE_WIDTH_RATE;
    const float laneHeight = g_ScreenHeight * LANE_HEIGHT_RATE;

    Sprite_Draw(
        Resouce_Manager_GetTexId(Lane),
        laneX, laneY,
        laneWidth, laneHeight,
        {1.0f,1.0f,1.0f,0.5f}
    );

    // 判定位置
    int judgeTexId = -1;

    if (!PadLogger_IsConnected()){
        judgeTexId = Resouce_Manager_GetTexId(Judge_S);
    } else {
        switch (PadLogger_GetControllerDisplayType())
        {
            case ControllerDisplayType::PlayStation:
                judgeTexId = Resouce_Manager_GetTexId(Judge_Square);
                break;

            case ControllerDisplayType::Xbox:
            case ControllerDisplayType::Unknown:
            default:

                judgeTexId = Resouce_Manager_GetTexId(Judge_X);
                break;
        }
    }

    const float judgeX = g_ScreenWidth * JUDGE_X_RATE;

    const float judgeCenterY = laneY + laneHeight * 0.5f;

    const float judgeSize = g_ScreenHeight * JUDGE_SIZE_RATE;

    Sprite_Draw(
        judgeTexId,
        judgeX - judgeSize *  0.5f,
        judgeCenterY - judgeSize * 0.5f,
        judgeSize, judgeSize,
        XMFLOAT4
        { 1.0f, 1.0f, 1.0f, JUDGE_ALPHA}
    );

    // ノーツ
    const float noteSize = g_ScreenHeight * NOTE_SIZE_RATE;

    const float noteCenterY = laneY + laneHeight *  0.5f;
    const int noteTexId = Resouce_Manager_GetTexId(Notes_Normal);

    for (const DemoNote& note : g_DemoNotes){
        Sprite_Draw(
            noteTexId,
            note.x - noteSize * 0.5f, noteCenterY - noteSize * 0.5f,
            noteSize, noteSize
        );
    }
}

static void SpawnDemoNote(){
	DemoNote note;

	note.x = g_ScreenWidth * NOTE_SPAWN_X_RATE;

	g_DemoNotes.push_back(note);
}

static void RemoveFinishedNotes()
{
    const float judgeX =
        g_ScreenWidth *
        JUDGE_X_RATE;

    for (auto iterator =
        g_DemoNotes.begin();
        iterator !=
        g_DemoNotes.end();)
    {
        // ノーツ中心が判定位置を越えたら削除
        if (iterator->x <
            judgeX)
        {
            iterator =
                g_DemoNotes.erase(
                    iterator
                );
        }
        else
        {
            ++iterator;
        }
    }
}