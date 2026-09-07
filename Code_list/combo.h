/*
	ƒRƒ“ƒ{‚Ì§ŒäFcombo.h

	2026/06/19	hibiki sakuma
*/
#ifndef COMBO_H
#define COMBO_H

void Combo_Initialize(int digit);
void Combo_Update(double elapsed_time);
void Combo_Draw();

void Combo_Add();   // ¬Œ÷
void Combo_Reset(); // Miss
void Combo_SetPerfectChain(bool flag);

#endif
