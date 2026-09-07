/*
    テキスト（文字）の制御：text_manager.h

    2026/07/03	hibiki sakuma
*/
#ifndef TEXT_MANAGER_H
#define TEXT_MANAGER_H

#include <DirectXMath.h>
#include <string>

void Text_Initialize();
void Text_Finalize();
// 別言語のフォントデータを再読み込み
void Text_ReloadFont();
// テキスト表示　基本
void Text_Draw( const wchar_t* text, float x, float y);
// テキスト表示　色を変更する
void Text_Draw( const wchar_t* text, float x, float y, DirectX::FXMVECTOR color);
// テキスト表示　色を変更する　サイズを変更する
void Text_Draw( const wchar_t* text, float x, float y, DirectX::FXMVECTOR color, float scale);
// テキストの改行
float Text_MeasureWidth(const std::wstring& text);
std::wstring Text_Wrap(const std::wstring& text, float maxWidth, bool wrapByWord);

void Text_Begin();
void Text_End();

#endif // !EXT_MANAGER_H