//テクスチャ管理
//texture.h

#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <D3d11.h>
#include "sprite.h"

void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Texture_Finalize(void);

//テクスチャ画像の読み込み
int Texture_Load(const wchar_t* pFilename);

//今回は全部の画像を一度にすべて解放する
void Texture_AllRelease();

void Texture_SetTexture(int texid);

//テクスチャの縦横を取得する
unsigned int Texture_Width(int texid);
unsigned int Texture_Height(int texid);

#endif // !TEXTURE_H
