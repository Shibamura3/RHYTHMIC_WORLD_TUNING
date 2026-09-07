/*
	背景の制御：bg.h

	2025/07/16	hibiki sakuma
*/

#ifndef BG_H
#define BG_H

#include <DirectXMath.h>

void Bg_Initialize();
void Bg_Finalize();
void Bg_Update(double elapsed_time);
void Bg_Draw();

#endif // !BG_H
