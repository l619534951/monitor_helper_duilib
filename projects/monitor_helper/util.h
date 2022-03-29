#pragma once
#include <Windows.h>


bool CreatShortCutToDesktop();

void SetStartupWithWindows();

void CancleStartupWithWindows();

bool isWin7();

bool IsGreaterThanOrEqualToWin8();

bool SaveIntData(std::wstring name, int value);

int GetIntData(std::wstring name, int defaultValue);
