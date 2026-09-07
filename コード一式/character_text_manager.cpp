/*
    キャラクター多言語テキスト管理：Character_text_manager.cpp

    2026/08/28 hibiki sakuma
*/

#include "Character_text_manager.h"
#include "Configu.h"

#include <Windows.h>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// 読み込んだキャラクターテキスト
static std::unordered_map<std::string, CharacterLocalizedText>
g_CharacterLocalizedTexts;

// 該当するIDがなかった場合に返す空データ
static const CharacterLocalizedText EMPTY_CHARACTER_TEXT ={
    L"",
    L"",
    L"",
    L""
};

// UTF-8文字列からワイド文字列への変換
static std::wstring ConvertUtf8ToWide(const std::string& source){
    if (source.empty()){
        return L"";
    }

    const int requiredLength = MultiByteToWideChar(
        CP_UTF8,
        0,
        source.c_str(),
        static_cast<int>(source.size()),
        nullptr,
        0
    );

    if (requiredLength <= 0){
        return L"";
    }

    std::wstring result(static_cast<size_t>(requiredLength), L'\0');

    MultiByteToWideChar(
        CP_UTF8,
        0,
        source.c_str(),
        static_cast<int>(source.size()),
        &result[0],
        requiredLength
    );

    return result;
}

static std::wstring ReplaceNewLineTag(const std::wstring& source){
    std::wstring result = source;

    size_t position = 0;

    while ((position = result.find(L"\\n", position)) != std::wstring::npos){
        result.replace(
            position,
            2,
            L"\n"
        );

        position += 1;
    }
    return result;
}


// CSVの1行を列ごとに分割する
// ダブルクォーテーション内のカンマにも対応
static std::vector<std::string> ParseCsvLine(const std::string& line){
    std::vector<std::string> columns;

    std::string currentColumn;
    bool insideQuotes = false;

    for (size_t index = 0; index < line.size(); ++index){
        const char character = line[index];

        if (character == '"'){
            // CSV中の "" は文字としての "
            if (insideQuotes &&
                index + 1 < line.size() &&
                line[index + 1] == '"'){
                currentColumn += '"';
                ++index;
            }else{
                insideQuotes = !insideQuotes;
            }
        }else if (character == ',' && !insideQuotes){
            columns.push_back(currentColumn);
            currentColumn.clear();
        }else{
            currentColumn += character;
        }
    }

    columns.push_back(currentColumn);

    return columns;
}

// UTF-8 BOMを除去する
static void RemoveUtf8Bom(std::string& text){
    constexpr unsigned char UTF8_BOM_0 = 0xEF;
    constexpr unsigned char UTF8_BOM_1 = 0xBB;
    constexpr unsigned char UTF8_BOM_2 = 0xBF;

    if (text.size() < 3){
        return;
    }

    if (static_cast<unsigned char>(text[0]) == UTF8_BOM_0 &&
        static_cast<unsigned char>(text[1]) == UTF8_BOM_1 &&
        static_cast<unsigned char>(text[2]) == UTF8_BOM_2)
    {
        text.erase(0, 3);
    }
}

// CRLFのCRを除去する
static void RemoveCarriageReturn(std::string& text){
    if (!text.empty() && text.back() == '\r'){
        text.pop_back();
    }
}

// ヘッダー名から列番号を取得する
static int GetColumnIndex( const std::unordered_map<std::string, int>& headerIndices, const std::string& headerName){
    const auto iterator = headerIndices.find(headerName);

    if (iterator == headerIndices.end()){
        return -1;
    }

    return iterator->second;
}

// 指定列の文字列を安全に取得する
static std::string GetColumnValue( const std::vector<std::string>& columns, int columnIndex){
    if (columnIndex < 0){
        return "";
    }

    if (columnIndex >= static_cast<int>(columns.size())){
        return "";
    }

    return columns[static_cast<size_t>(columnIndex)];
}

// 現在言語のキャラクター説明ヘッダーを取得する
static const char* GetDescriptionHeaderName()
{
    switch (Configu_GetLanguage())
    {
    case Language::English:
        return "Description_EN";

    case Language::Korean:
        return "Description_KR";

    case Language::Chinese:
        return "Description_CN";

    case Language::Japanese:
    default:
        return "Description_JP";
    }
}

// 現在言語のスキル説明ヘッダーを取得する
static const char* GetSkillDescriptionHeaderName()
{
    switch (Configu_GetLanguage())
    {
    case Language::English:
        return "SkillDescription_EN";

    case Language::Korean:
        return "SkillDescription_KR";

    case Language::Chinese:
        return "SkillDescription_CN";

    case Language::Japanese:
    default:
        return "SkillDescription_JP";
    }
}

bool Character_Text_Initialize(const char* filePath){
    g_CharacterLocalizedTexts.clear();

    std::ifstream file(filePath);

    if (!file.is_open()){
        return false;
    }

    std::string headerLine;

    if (!std::getline(file, headerLine)){
        return false;
    }

    RemoveUtf8Bom(headerLine);
    RemoveCarriageReturn(headerLine);

    const std::vector<std::string> headers = ParseCsvLine(headerLine);

    std::unordered_map<std::string, int> headerIndices;

    for (size_t index = 0; index < headers.size(); ++index){
        headerIndices[headers[index]] = static_cast<int>(index);
    }

    const int characterIdIndex = GetColumnIndex(headerIndices, "CharacterID");
    const int nameIndex = GetColumnIndex(headerIndices, "Name");
    const int descriptionIndex = GetColumnIndex(headerIndices, GetDescriptionHeaderName());
    const int skillNameIndex = GetColumnIndex(headerIndices, "SkillName");
    const int skillDescriptionIndex = GetColumnIndex( headerIndices, GetSkillDescriptionHeaderName());

    if (characterIdIndex < 0 ||
        nameIndex < 0 ||
        descriptionIndex < 0 ||
        skillNameIndex < 0 ||
        skillDescriptionIndex < 0)
    {
        return false;
    }

    std::string line;

    while (std::getline(file, line)){
        RemoveCarriageReturn(line);

        if (line.empty()){
            continue;
        }

        const std::vector<std::string> columns = ParseCsvLine(line);
        const std::string characterId = GetColumnValue(columns, characterIdIndex);
        if (characterId.empty()){
            continue;
        }

        CharacterLocalizedText localizedText;
        localizedText.name = ConvertUtf8ToWide(GetColumnValue(columns, nameIndex));
        localizedText.description = ReplaceNewLineTag(ConvertUtf8ToWide(GetColumnValue(columns, descriptionIndex)));        
        localizedText.skillName = ConvertUtf8ToWide(GetColumnValue(columns, skillNameIndex));
        localizedText.skillDescription = ReplaceNewLineTag(ConvertUtf8ToWide(GetColumnValue(columns, skillDescriptionIndex)));

        g_CharacterLocalizedTexts[characterId] = std::move(localizedText);
    }

    return !g_CharacterLocalizedTexts.empty();
}

void Character_Text_Finalize(){
    g_CharacterLocalizedTexts.clear();
}

const CharacterLocalizedText& Character_Text_Get(const std::string& characterId){
    const auto iterator = g_CharacterLocalizedTexts.find(characterId);

    if (iterator == g_CharacterLocalizedTexts.end()){
        return EMPTY_CHARACTER_TEXT;
    }

    return iterator->second;
}