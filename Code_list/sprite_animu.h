/************************************************
*スプライト表示　sprite_animu.h
* 
* 
*************************************************/
#ifndef SPRITE_ANIMU_H
#define SPRITE_ANIMU_H

#include <DirectXMath.h>

void SpriteAnim_Initialize();
void SpriteAnim_Finalize();

void SpriteAnim_Update(double elapsed_time);
void SpriteAnim_Draw(int playid,float dx,float dy,float dw,float dh);

int Spriteanim_RegisterPattern(int texId, int patternMax, int h_patternMax, double seconds_par_pattern, const DirectX::XMUINT2& pattern_size, const DirectX::XMUINT2& start_position, bool is_looped);

int SpriteAnim_CreatePlayer(int anim_pattern_id);

void SpriteAnim_DestroyPlayer(int index);

bool SpriteAnim_IsStopped(int index);

#endif // SPRITE_ANIMU_H