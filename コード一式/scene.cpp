/*
	画面遷移の制御：scene.h

	2026/05/22	hibiki sakuma
*/

#include "scene.h"
#include "Title.h"
#include "Music_select.h"
#include "Configu.h"
#include "Tutorial.h"
#include "Edit.h"
#include "Character_select.h"
#include "Game.h"
#include "Result.h"
static Scene g_Scene_now = SCENE_TITLE;
static Scene g_Scene_next = g_Scene_now;



void Scene_Initialize(){
	switch (g_Scene_now) {
	case SCENE_TITLE:
		Title_Initialize();
		break;
	case SCENE_MUSIC_SELEC:
		Music_Select_Initialize();
		break;
	case SCENE_CONFIGU:
		Configu_Initialize();
		break;
	case SCENE_TUTORIAL:
		Tutorial_Initialize();
		break;
	case SCENE_EDIT:
		Edit_Initialize();
		break;
	case SCENE_CHARACTER_SELECT:
		Character_Select_Initialize();
		break;
	case SCENE_GAME:
		Game_Initialize();
		break;
	case SCENE_RESULT:
		Result_Initialize();
		break;
	case SCENE_MAX:

		break;
	default:

		break;
	}
}

void Scene_Finalize(){
	switch (g_Scene_now) {
	case SCENE_TITLE:
		Title_Finalize();
		break;
	case SCENE_MUSIC_SELEC:
		Music_Select_Finalize();
		break;
	case SCENE_CONFIGU:
		Configu_Finalize();
		break;
	case SCENE_TUTORIAL:
		Tutorial_Finalize();
		break;
	case SCENE_EDIT:
		Edit_Finalize();
		break;
	case SCENE_CHARACTER_SELECT:
		Character_Select_Finalize();
		break;
	case SCENE_GAME:
		Game_Finalize();
		break;
	case SCENE_RESULT:
		Result_Finalize();
		break;
	case SCENE_MAX:

		break;
	default:

		break;
	}
}

void Scene_Update(double elapsed_time){
	switch (g_Scene_now) {
	case SCENE_TITLE:
		Title_Update(elapsed_time);
		break;
	case SCENE_MUSIC_SELEC:
		Music_Select_Update(elapsed_time);
		break;
	case SCENE_CONFIGU:
		Configu_Update(elapsed_time);
		break;
	case SCENE_TUTORIAL:
		Tutorial_Update(elapsed_time);
		break;
	case SCENE_EDIT:
		Edit_Update(elapsed_time);
		break;
	case SCENE_CHARACTER_SELECT:
		Character_Select_Update(elapsed_time);
		break;
	case SCENE_GAME:
		Game_UpDate(elapsed_time);
		break;
	case SCENE_RESULT:
		Result_Update(elapsed_time);
		break;
	case SCENE_MAX:
		
		break;
	default:
		
		break;
	}
}

void Scene_Draw(){
	switch (g_Scene_now) {
	case SCENE_TITLE:
		Title_Draw();
		break;
	case SCENE_MUSIC_SELEC:
		Music_Select_Draw();
		break;
	case SCENE_CONFIGU:
		Configu_Draw();
		break;
	case SCENE_TUTORIAL:
		Tutorial_Draw();
		break;
	case SCENE_EDIT:
		Edit_Draw();
		break;
	case SCENE_CHARACTER_SELECT:
		Character_Select_Draw();
		break;
	case SCENE_GAME:
		Game_Draw();
		break;
	case SCENE_RESULT:
		Result_Draw();
		break;
	case SCENE_MAX:
		
		break;
	default:
		
		break;
	}
}

void Scene_Refresh(){
	if (g_Scene_now != g_Scene_next) { // シーンチェンジをするタイミング
		// 現在のシーンのあと片付け
		Scene_Finalize();
		// 現在のシーンを次のシーンに移す
		g_Scene_now = g_Scene_next;
		// 次のシーンの初期化
		Scene_Initialize();
	}
}

void Scene_Change(Scene scene){
	g_Scene_next = scene;
}
