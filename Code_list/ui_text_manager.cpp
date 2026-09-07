/*
    UIテキスト管理：ui_text_manager.cpp

    2026/08/25    hibiki sakuma
*/

#include "ui_text_manager.h"

#include "Configu.h"
#include "resource_manager.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    static std::unordered_map<std::string, std::wstring> g_UITexts;


    static std::string MakeTextKey( const std::string& textId, const std::string& language){
        return textId +"|" +language;
    }


    static std::vector<std::string> SplitCSVLine( const std::string& line){
        std::vector<std::string> result;

        std::string field;

        bool insideQuotes = false;

        for (std::size_t i = 0; i < line.size(); ++i){
            const char character =line[i];

            if (character == '"'){
                if (insideQuotes && i + 1 < line.size() && line[i + 1] == '"'){
                    field.push_back('"');
                    ++i;
                }else{
                    insideQuotes = !insideQuotes;
                }
            }else if (character == ',' && !insideQuotes){
                result.push_back(field);
                field.clear();
            }else{
                field.push_back(character);
            }
        }

        result.push_back(field);

        return result;
    }


    static void TrimLineEnd( std::string& value){
        while (!value.empty()){
            const char last = value.back();

            if (last == '\r' ||
                last == '\n' ||
                last == ' ' ||
                last == '\t')
            {
                value.pop_back();
            }else{
                break;
            }
        }
    }


    static void RemoveUtf8Bom(std::string& value){
        if (value.size() >= 3 &&
            static_cast<unsigned char>(value[0]) == 0xEF &&
            static_cast<unsigned char>(value[1]) == 0xBB &&
            static_cast<unsigned char>(value[2]) == 0xBF)
        {
            value.erase(0, 3);
        }
    }


    static std::wstring Utf8ToWstring(const std::string& value){
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

        return converter.from_bytes(value);
    }
}


bool UI_Text_Load(){
    g_UITexts.clear();

    const char* path = Resouce_Manager_GetDataFilePath(DataFileID::UI_Text);

    if (path == nullptr){
        return false;
    }

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()){
        return false;
    }

    std::string line; int lineNumber = 0;

    while (std::getline(file, line)){
        ++lineNumber;

        if (lineNumber == 1){
            RemoveUtf8Bom(line);
            continue;
        }

        if (line.empty()){
            continue;
        }

        std::vector<std::string> columns = SplitCSVLine(line);

        for (std::string& column : columns){
            TrimLineEnd(column);
        }

        if (columns.size() != 3){
            continue;
        }

        const std::string& textId = columns[0];

        const std::string& language = columns[1];

        const std::wstring text = Utf8ToWstring(columns[2]);

        g_UITexts[MakeTextKey(textId, language)] = text;
    }

    return !g_UITexts.empty();
}


void UI_Text_Clear(){
    g_UITexts.clear();
}


const std::wstring& UI_Text_Get(const std::string& textId){
    static const std::wstring EMPTY_TEXT;

    const std::string language = Configu_GetLanguageCode();

    const std::string currentKey = MakeTextKey(textId, language);

    const auto currentText = g_UITexts.find(currentKey);

    if (currentText != g_UITexts.end()){
        return currentText->second;
    }

    // 英語へフォールバックする
    const std::string englishKey = MakeTextKey(textId, "EN");

    const auto englishText = g_UITexts.find(englishKey);

    if (englishText != g_UITexts.end()){
        return englishText->second;
    }

    return EMPTY_TEXT;
}