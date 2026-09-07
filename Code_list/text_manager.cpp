/*
    テキスト（文字）の制御：text_manager.cpp

    2026/07/03	hibiki sakuma
*/

#include "text_manager.h"
#include "resource_manager.h"
#include "Configu.h"
#include "direct3d.h"
#include "shader.h"

#include <string>
#include <SpriteBatch.h>
#include <SpriteFont.h>
using namespace DirectX;

// 変数宣言
static std::unique_ptr<SpriteBatch> g_SpriteBatch;
static std::unique_ptr<SpriteFont> g_Font;
// 内部関数
static std::wstring Text_ReplaceUnsupportedCharacters(const wchar_t* text);
static FontID Text_GetFontIDFromLanguage(Language language);
static float GetLanguageScale();
static std::wstring Text_ReplaceUnsupportedCharacters(const wchar_t* text);
static FontID Text_GetFontIDFromLanguage(Language language);

static float GetLanguageScale();
void Text_Initialize(){
    g_SpriteBatch = std::make_unique<SpriteBatch>( Direct3D_GetContext());

    Text_ReloadFont();
}

void Text_Finalize() {
    g_Font.reset();
    g_SpriteBatch.reset();
}

void Text_ReloadFont(){
    FontID fontID =Text_GetFontIDFromLanguage(Configu_GetLanguage());

    const wchar_t* fontPath = Resouce_Manager_GetFontFilePath(fontID);

    // 指定言語のパスが未登録なら日本語へ戻す
    if (fontPath == nullptr){
        fontPath = Resouce_Manager_GetFontFilePath( FontID::Japanese_Font);
    }

    if (fontPath == nullptr)
        return;

    g_Font.reset();

    g_Font = std::make_unique<DirectX::SpriteFont>( Direct3D_GetDevice(),fontPath);
}

void Text_Draw(const wchar_t* text, float x, float y) {
    if (text == nullptr) return;

    std::wstring safeText = Text_ReplaceUnsupportedCharacters(text);

    g_Font->DrawString(
        g_SpriteBatch.get(),
        safeText.c_str(),
        XMFLOAT2(x, y),
        Colors::Black,
        0.0f,
        XMFLOAT2(0, 0),
        GetLanguageScale()
    );
}

void Text_Draw( const wchar_t* text, float x, float y, FXMVECTOR color) {
    if (text == nullptr) return;

    std::wstring safeText = Text_ReplaceUnsupportedCharacters(text);

    g_Font->DrawString(
        g_SpriteBatch.get(),
        safeText.c_str(),
        XMFLOAT2(x, y),
        color,
        0.0f,
        XMFLOAT2(0, 0),
        GetLanguageScale()
    );
}

void Text_Draw( const wchar_t* text, float x, float y, FXMVECTOR color, float scale) {
    if (text == nullptr) return;

    std::wstring safeText = Text_ReplaceUnsupportedCharacters(text);

    g_Font->DrawString(
        g_SpriteBatch.get(),
        safeText.c_str(),
        XMFLOAT2(x, y),
        color,
        0.0f,
        XMFLOAT2(0, 0),
        scale * GetLanguageScale()
    );
}

float Text_MeasureWidth(const std::wstring& text){
    if (g_Font == nullptr || text.empty())
        return 0.0f;

    const DirectX::XMVECTOR size = g_Font->MeasureString(text.c_str());

    const float originalWidth = DirectX::XMVectorGetX(size);

    return originalWidth * GetLanguageScale();
}

static std::wstring Text_WrapByCharacter(const std::wstring & text, float maxWidth){
    std::wstring result;
    std::wstring currentLine;

    for (wchar_t character : text){
        // CSVやコードで指定された改行を維持
        if (character == L'\n'){
            result += currentLine;
            result += L'\n';
            currentLine.clear();
            continue;
        }

        std::wstring candidate = currentLine + character;

        const float candidateWidth = Text_MeasureWidth(candidate);

        // 1文字目でなければ、幅を超える前に改行
        if (!currentLine.empty() && candidateWidth > maxWidth) {
            result += currentLine;
            result += L'\n';

            currentLine.clear();
            currentLine += character;
        } else {
            currentLine += character;
        }
    }

    result += currentLine;

    return result;
}

static std::wstring Text_WrapByWord(const std::wstring& text, float maxWidth){
    std::wstring result;
    std::wstring currentLine;
    std::wstring currentWord;

    auto appendWord = [&]() {
            if (currentWord.empty())
                return;

            std::wstring candidate;

            if (currentLine.empty()){
                candidate = currentWord;
            } else {
                candidate = currentLine + L" " + currentWord;
            }

            if (!currentLine.empty() && Text_MeasureWidth( candidate ) > maxWidth){
                result += currentLine;
                result += L'\n';

                currentLine = currentWord;
            } else {
                currentLine = candidate;
            }

            currentWord.clear();
        };

    for (wchar_t character : text) {
        if (character == L'\n') {
            appendWord();

            result += currentLine;
            result += L'\n';

            currentLine.clear();
        } else if (character == L' ' || character == L'\t') {
            appendWord();
        } else {
            currentWord += character;
        }
    }

    appendWord();

    result += currentLine;

    return result;
}

std::wstring Text_Wrap(const std::wstring& text, float maxWidth, bool wrapByWord) {
    if (text.empty() || maxWidth <= 0.0f)
        return text;

    if (wrapByWord)
        return Text_WrapByWord(text, maxWidth);

    return Text_WrapByCharacter(text, maxWidth);
}

void Text_Begin() {
    g_SpriteBatch->Begin();
}

void Text_End() {
    g_SpriteBatch->End();

    // テキスト用から画像用に切り替え
    Direct3D_SetDefaultState();
    Shader_Begin();
}

static std::wstring Text_ReplaceUnsupportedCharacters(const wchar_t* text)
{
    std::wstring result;

    if (text == nullptr) return result;

    for (const wchar_t* p = text; *p != L'\0'; ++p)
    {
        wchar_t ch = *p;

        // 改行やタブはそのまま通す
        if (ch == L'\n' || ch == L'\r' || ch == L'\t')
        {
            result += ch;
            continue;
        }

        // フォントに文字があるならそのまま
        if (g_Font->ContainsCharacter(ch))
        {
            result += ch;
        }
        //  ? に退避
        else if (g_Font->ContainsCharacter(L'?'))
        {
            result += L'?';
        }
        // 最後の退避
        else
        {
            result += L' ';
        }
    }

    return result;
}

static FontID Text_GetFontIDFromLanguage(Language language){
    switch (language)
    {
    case Language::Japanese:
        return FontID::Japanese_Font;

    case Language::English:
        return FontID::English_Font;

    case Language::Korean:
        return FontID::Korean_Font;

    case Language::Chinese:
        return FontID::Chinese_Font;

    default:
        return FontID::Japanese_Font;
    }
}

static float GetLanguageScale(){
    switch (Configu_GetLanguage())
    {
    case Language::English:
        return 0.70f;

    case Language::Korean:
        return 0.70f;

    case Language::Chinese:
        return 0.70f;

    case Language::Japanese:
    default:
        return 0.70f;
    }
}