/*
    UIテキスト管理：ui_text_manager.h

    2026/08/25    hibiki sakuma
*/

#ifndef UI_TEXT_MANAGER_H
#define UI_TEXT_MANAGER_H

#include <string>

bool UI_Text_Load();

void UI_Text_Clear();

const std::wstring& UI_Text_Get(const std::string& textId);

#endif // !UI_TEXT_MANAGER_H