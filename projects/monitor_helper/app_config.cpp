#include "stdafx.h"
#include "app_config.h"
#include "util.h"

bool AppConfig::startupWithWindows = false;
bool AppConfig::disableMouseWakeUpDisplay = false;

std::wstring AppConfig::KEY_STARTUP_WITH_WINDOWS = L"startup_with_windows";
std::wstring AppConfig::KEY_DISABLE_MOUSE_WAKE_UP_DISPLAY = L"disable_mouse_wake_up_display";

AppConfig::AppConfig()
{
	AppConfig::LoadData();
}

void AppConfig::LoadData()
{
	AppConfig::startupWithWindows = IsStartupWithWindows();
	AppConfig::disableMouseWakeUpDisplay = IsDisableMouseWakeUpDisplay();
}

bool AppConfig::IsStartupWithWindows()
{
	return GetIntData(AppConfig::KEY_STARTUP_WITH_WINDOWS, false);
}

bool AppConfig::IsDisableMouseWakeUpDisplay()
{
	return GetIntData(AppConfig::KEY_DISABLE_MOUSE_WAKE_UP_DISPLAY, false);
}

AppConfig::~AppConfig()
{
}