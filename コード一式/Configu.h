/*
    コンフィグ画面の制御：Configu.h

    2026/07/09    hibiki sakuma
*/

#ifndef CONFIGU_H
#define CONFIGU_H

// 多言語設定
enum class Language
{
    Japanese,
    English,
    Korean,
    Chinese,

    MAX
};

// スクリーンサイズ
enum class ScreenSize
{
    Size1280x720,
    Size1600x900,
    Size1920x1080,

    Max
};

// 設定データ
struct Config
{
    int MasterVolume;

    int BGMVolume;
    int SEVolume;
    int VoiceVolume;

    float NoteSpeed;

    Language Lang;
    
    ScreenSize Screen;

};

void Configu_Initialize();
void Configu_Finalize();

void Configu_Update(double elapsed_time);

void Configu_Draw();

// 保存・読込
void Config_Save();
void Config_Load();


// 外部参照
float Configu_GetNoteSpeed();

int Configu_GetVolume();

Language Configu_GetLanguage();
const char* Configu_GetLanguageCode();
const wchar_t* Configu_GetLanguageDisplayText();


#endif // CONFIGU_H