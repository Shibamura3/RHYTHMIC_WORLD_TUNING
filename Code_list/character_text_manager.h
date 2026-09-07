/*
    キャラクター多言語テキスト管理：Character_text_manager.h

    2026/08/28 hibiki sakuma
*/

#ifndef CHARACTER_TEXT_MANAGER_H
#define CHARACTER_TEXT_MANAGER_H

#include <string>

struct CharacterLocalizedText
{
    std::wstring name;
    std::wstring description;

    std::wstring skillName;
    std::wstring skillDescription;
};

// CSVを読み込む
bool Character_Text_Initialize(const char* filePath);

// 読み込んだデータを破棄する
void Character_Text_Finalize();

// CharacterIDから現在の言語に対応したデータを取得する
const CharacterLocalizedText& Character_Text_Get(const std::string& characterId);

#endif