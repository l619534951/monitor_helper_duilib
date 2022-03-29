#pragma once
class AppConfig
{
public:
	static bool startupWithWindows;

	static bool disableMouseWakeUpDisplay;


	static std::wstring KEY_STARTUP_WITH_WINDOWS;

	static std::wstring KEY_DISABLE_MOUSE_WAKE_UP_DISPLAY;


	static void LoadData();

	AppConfig();

	~AppConfig();

private:
	static bool IsStartupWithWindows();

	static bool IsDisableMouseWakeUpDisplay();
};

