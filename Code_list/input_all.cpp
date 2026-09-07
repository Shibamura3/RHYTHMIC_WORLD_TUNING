/*
	何か押された場合の処理用：input_all.h

	2026/05/22	hibiki sakuma
*/

#include "input_all.h"

#include "Key_logger.h"
#include "pad_logger.h"

bool IsAnyButtonTriggered()
{
    // キーボード
    if (KeyLogger_IsTrigger(KK_ENTER)) return true;
    if (KeyLogger_IsTrigger(KK_SPACE)) return true;
    if (KeyLogger_IsTrigger(KK_Z)) return true;
    if (KeyLogger_IsTrigger(KK_X)) return true;
    if (KeyLogger_IsTrigger(KK_C)) return true;
    if (KeyLogger_IsTrigger(KK_A)) return true;
    if (KeyLogger_IsTrigger(KK_S)) return true;
    if (KeyLogger_IsTrigger(KK_W)) return true;
    if (KeyLogger_IsTrigger(KK_Q)) return true;

    // コントローラー
    if (PadLogger_IsConnected())
    {
        if (PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_A)) return true;
        if (PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_B)) return true;
        if (PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_X)) return true;
        if (PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_Y)) return true;
        if (PadLogger_IsTrigger(0, SDL_CONTROLLER_BUTTON_START)) return true;
    }
    return false;
}

